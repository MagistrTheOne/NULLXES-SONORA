import pytest

from tests.conftest import requires_postgres


@requires_postgres
@pytest.mark.asyncio
async def test_upload_analyze_memory_events_and_recommend(
    api_client, stereo_wav
) -> None:
    upload = await api_client.post(
        "/api/v1/audio/upload",
        files={"file": ("track.wav", stereo_wav.read_bytes(), "audio/wav")},
    )
    assert upload.status_code == 201
    audio_id = upload.json()["id"]

    analyze = await api_client.post(
        "/api/v1/audio/analyze",
        json={"audio_id": audio_id},
    )
    assert analyze.status_code == 202
    analysis_id = analyze.json()["analysis_id"]

    detail = await api_client.get(f"/api/v1/audio/{audio_id}")
    assert detail.status_code == 200
    analysis = detail.json()["analysis"]
    assert analysis["id"] == analysis_id
    assert analysis["status"] == "completed"
    assert analysis["analyzer_version"].startswith("SONORA_DSP_")
    features = analysis["features"]
    assert "loudness_lufs_approx" in features
    assert "confidence" in features["key_estimation"]
    assert "method" in features["key_estimation"]

    events = await api_client.get("/api/v1/events")
    types = {item["type"] for item in events.json()["items"]}
    assert "audio_uploaded" in types
    assert "analysis_completed" in types

    rec = await api_client.post(
        "/api/v1/recommendation/generate",
        json={"analysis_id": analysis_id},
    )
    assert rec.status_code == 200
    payload = rec.json()
    assert payload["items"]
    assert payload["recommendations"] == [item["action"] for item in payload["items"]]

    decision = await api_client.post(
        f"/api/v1/recommendation/{payload['id']}/decision",
        json={"decision": "accepted", "item_index": 0},
    )
    assert decision.status_code == 201

    memory = await api_client.get("/api/v1/memory")
    assert memory.status_code == 200
    assert memory.json()["analyses"]
    assert memory.json()["decisions"]
