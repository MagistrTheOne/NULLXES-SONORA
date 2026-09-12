from uuid import uuid4

from app.core.exceptions import UnsupportedMediaError
from app.services.storage import AudioStorage


def test_asset_layout(tmp_path) -> None:
    storage = AudioStorage(root=tmp_path)
    audio_id = uuid4()
    stored = storage.write_original(audio_id, "kick.wav", b"RIFF")
    assert stored == "original.wav"
    root = tmp_path / "audio" / str(audio_id)
    assert (root / "original.wav").exists()
    assert (root / "analysis").is_dir()
    assert (root / "waveform").is_dir()
    assert (root / "cache").is_dir()

    storage.write_latest_analysis(audio_id, {"ok": True})
    assert (root / "analysis" / "latest.json").read_text(encoding="utf-8")


def test_rejects_unknown_extension(tmp_path) -> None:
    storage = AudioStorage(root=tmp_path)
    try:
        storage.write_original(uuid4(), "notes.txt", b"nope")
    except UnsupportedMediaError:
        return
    raise AssertionError("expected UnsupportedMediaError")
