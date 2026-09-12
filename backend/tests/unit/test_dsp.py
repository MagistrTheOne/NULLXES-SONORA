import pytest

from app.audio.dsp.features import extract_features
from app.audio.loader import load_audio


def test_stereo_feature_contract(stereo_wav) -> None:
    loaded = load_audio(stereo_wav)
    features = extract_features(loaded)

    assert features.duration_sec == pytest.approx(loaded.duration_sec, abs=0.05)
    assert features.sample_rate == loaded.sample_rate
    assert features.channels == 2
    assert features.rms > 0
    assert 0 < features.peak <= 1
    assert features.stereo_width > 0.05
    assert "loudness_lufs_approx" in features.model_dump()
    assert features.dynamic_range_label in {"low", "medium", "high"}

    dist = features.frequency_distribution
    total = dist.sub + dist.low + dist.mid + dist.high + dist.air
    assert abs(total - 1.0) < 1e-6
    assert dist.low > 0
    assert dist.high > 0 or dist.mid > 0

    key = features.key_estimation
    assert 0.0 <= key.confidence <= 1.0
    assert key.method == "chroma_cqt"
    assert key.key is None or isinstance(key.key, str)
    assert features.analyzer_version.startswith("SONORA_DSP_")


def test_mono_has_zero_width(mono_wav) -> None:
    features = extract_features(load_audio(mono_wav))
    assert features.channels == 1
    assert features.stereo_width == 0.0
