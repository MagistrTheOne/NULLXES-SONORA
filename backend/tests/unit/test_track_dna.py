from __future__ import annotations

from pathlib import Path

import numpy as np
import soundfile as sf

from app.audio.dsp.features import extract_features
from app.audio.loader import load_audio


def write_arranged_wav(path: Path, sample_rate: int = 22050) -> Path:
    """Quiet / loud / quiet — enough contrast for arrangement, not a song."""
    seconds = 24.0
    frames = int(sample_rate * seconds)
    t = np.linspace(0.0, seconds, frames, endpoint=False)
    intro = (t < 8.0).astype(np.float32)
    drop = ((t >= 8.0) & (t < 16.0)).astype(np.float32)
    outro = (t >= 16.0).astype(np.float32)
    left = (
        0.08 * intro * np.sin(2 * np.pi * 70.0 * t)
        + 0.55 * drop * np.sin(2 * np.pi * 70.0 * t)
        + 0.35 * drop * np.sin(2 * np.pi * 140.0 * t)
        + 0.18 * drop * np.sin(2 * np.pi * 1000.0 * t)
        + 0.07 * outro * np.sin(2 * np.pi * 70.0 * t)
    )
    right = (
        0.08 * intro * np.sin(2 * np.pi * 70.0 * t + 0.2)
        + 0.45 * drop * np.sin(2 * np.pi * 70.0 * t + 0.4)
        + 0.28 * drop * np.sin(2 * np.pi * 180.0 * t)
        + 0.22 * drop * np.sin(2 * np.pi * 2200.0 * t)
        + 0.07 * outro * np.sin(2 * np.pi * 70.0 * t + 0.1)
    )
    sf.write(path, np.stack([left, right], axis=1).astype(np.float32), sample_rate)
    return path


def test_short_tone_still_builds_dna(stereo_wav) -> None:
    features = extract_features(load_audio(stereo_wav))
    assert features.dna is not None
    dna = features.dna
    assert dna.identity.tempo == features.bpm
    assert dna.structure.sections
    assert dna.structure.sections[0].name == "body"
    assert len(dna.energy.curve) >= 1
    assert {item.name for item in dna.translation.targets} == {"phone", "car", "club", "headphones"}
    assert dna.masking.roles == ["kick", "bass", "vocal", "lead"]
    assert len(dna.masking.matrix) == 4
    assert all(len(row) == 4 for row in dna.masking.matrix)
    assert all(row[index] == 0.0 for index, row in enumerate(dna.masking.matrix))
    assert any(item.type == "ARRANGEMENT_MAP" for item in dna.objects)


def test_arrangement_labels_energy_contrast(tmp_path: Path) -> None:
    path = write_arranged_wav(tmp_path / "arranged.wav")
    features = extract_features(load_audio(path))
    assert features.dna is not None
    sections = features.dna.structure.sections
    assert len(sections) >= 2
    names = [section.name for section in sections]
    assert sections[0].start == 0.0
    assert sections[-1].end == features.duration_sec
    assert max(section.energy for section in sections) > min(section.energy for section in sections)
    assert "drop" in names or names[0] in {"intro", "body"}
    energies = features.dna.energy.curve
    assert max(energies) > min(energies)
    assert features.dna.mix_character.low_end.risk in {"low", "medium", "high"}
    assert features.dna.mix_character.low_end.why
    phone = next(item for item in features.dna.translation.targets if item.name == "phone")
    assert 0.0 <= phone.score <= 1.0


def test_mix_character_exposes_method(clipped_wav) -> None:
    features = extract_features(load_audio(clipped_wav))
    assert features.dna is not None
    assert features.dna.mix_character.low_end.method
    types = {item.type for item in features.dna.objects}
    assert "ARRANGEMENT_MAP" in types
    assert "MASTER_CHAIN" in types
