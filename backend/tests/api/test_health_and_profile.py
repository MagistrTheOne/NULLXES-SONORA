import pytest

from tests.conftest import requires_postgres


@requires_postgres
@pytest.mark.asyncio
async def test_health_and_owner_profile(api_client) -> None:
    health = await api_client.get("/health")
    assert health.status_code == 200
    body = health.json()
    assert body["service"] == "sonora"
    assert body["database"] == "ok"

    profile = await api_client.get("/api/v1/profile")
    assert profile.status_code == 200
    data = profile.json()
    assert data["id"] == 1
    assert "bass" in data["preferences"]

    updated = await api_client.put(
        "/api/v1/profile",
        json={
            "style": ["slap house", "deep house"],
            "preferences": {"bass": "strong", "vocals": "clean", "mix": "wide"},
            "favorite_genres": ["house"],
            "preferred_sound_profile": {"low_end": "weighty"},
        },
    )
    assert updated.status_code == 200
    assert updated.json()["style"] == ["slap house", "deep house"]
    assert updated.json()["preferences"]["bass"] == "strong"
