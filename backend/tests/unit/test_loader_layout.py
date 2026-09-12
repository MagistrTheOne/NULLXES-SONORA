import numpy as np

from app.audio.loader import from_librosa_layout, from_soundfile_layout, load_audio


def test_librosa_short_multichannel_keeps_channel_first() -> None:
    raw = np.arange(30, dtype=np.float32).reshape(6, 5)
    out = from_librosa_layout(raw)
    assert out.shape == (6, 5)
    assert np.array_equal(out, raw)


def test_soundfile_frames_channels_transposes_to_channel_first() -> None:
    raw = np.arange(30, dtype=np.float32).reshape(5, 6)
    out = from_soundfile_layout(raw)
    assert out.shape == (6, 5)
    assert np.array_equal(out, raw.T)


def test_stereo_file_reports_two_channels(stereo_wav) -> None:
    loaded = load_audio(stereo_wav)
    assert loaded.channels == 2
    assert loaded.samples.ndim == 2
    assert loaded.samples.shape[0] == 2
    assert loaded.samples.shape[1] > 2
    assert loaded.duration_sec > 1.0
