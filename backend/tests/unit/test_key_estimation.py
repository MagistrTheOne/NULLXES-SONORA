from app.audio.dsp.features import extract_features
from app.audio.loader import load_audio


def test_key_estimation_is_uncertain_on_sines(stereo_wav) -> None:
    features = extract_features(load_audio(stereo_wav))
    estimation = features.key_estimation.model_dump()
    assert set(estimation) == {"key", "confidence", "method"}
    assert estimation["method"] == "chroma_cqt"
    assert 0.0 <= estimation["confidence"] <= 1.0
