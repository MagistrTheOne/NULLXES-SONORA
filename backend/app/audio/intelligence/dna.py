from __future__ import annotations

from app.audio.dsp.timeline import compute_timeline, downsample_curve, peak_envelope
from app.audio.intelligence.genre import infer_genre_profile
from app.audio.intelligence.masking import build_masking
from app.audio.intelligence.mix_character import build_mix_character
from app.audio.intelligence.objects import build_objects
from app.audio.intelligence.structure import detect_structure
from app.audio.intelligence.translation import build_translation
from app.audio.loader import LoadedAudio
from app.schemas.audio import AudioFeatures, Issue
from app.schemas.dna import EnergyMap, IdentityKey, TrackDNA, TrackIdentity


def build_track_dna(
    audio: LoadedAudio,
    features: AudioFeatures,
    issues: list[Issue],
) -> TrackDNA:
    timeline = compute_timeline(audio.samples, audio.sample_rate)
    structure = detect_structure(timeline, features.duration_sec, features.bpm)
    mix = build_mix_character(audio.samples, audio.sample_rate, features)
    translation = build_translation(features.frequency_distribution, mix)
    masking = build_masking(audio.samples, audio.sample_rate)
    curve = downsample_curve(timeline.rms)
    transient_mean = float(timeline.transient.mean()) if timeline.transient.size else 0.0
    key = features.key_estimation

    return TrackDNA(
        identity=TrackIdentity(
            tempo=features.bpm,
            key=IdentityKey(name=key.key, confidence=key.confidence),
            genre_profile=infer_genre_profile(
                bpm=features.bpm,
                duration_sec=features.duration_sec,
                dist=features.frequency_distribution,
                transient_mean=transient_mean,
            ),
        ),
        energy=EnergyMap(
            curve=curve,
            peaks=peak_envelope(audio.samples),
            hop_sec=round(timeline.hop_sec, 4),
            mean=round(float(sum(curve) / max(len(curve), 1)), 4),
            peak=round(float(max(curve) if curve else 0.0), 4),
            method="rms_envelope + peak_hold",
        ),
        structure=structure,
        mix_character=mix,
        translation=translation,
        masking=masking,
        objects=build_objects(issues=issues, structure=structure, mix=mix),
    )
