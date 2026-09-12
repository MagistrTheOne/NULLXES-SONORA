from app.audio.dsp.features import extract_features
from app.audio.intelligence.issues import detect_issues
from app.audio.loader import load_audio


def test_clipped_signal_flags_clipping(clipped_wav) -> None:
    features = extract_features(load_audio(clipped_wav))
    types = {issue.type for issue in detect_issues(features)}
    assert "clipping" in types


def test_mono_flags_narrow_stereo(mono_wav) -> None:
    features = extract_features(load_audio(mono_wav))
    types = {issue.type for issue in detect_issues(features)}
    assert "narrow_stereo" in types
    issue = next(item for item in detect_issues(features) if item.type == "narrow_stereo")
    assert 0 < issue.severity <= 1
