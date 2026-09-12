from __future__ import annotations

import numpy as np


def stereo_width(samples: np.ndarray) -> float:
    """Side/(mid+side) energy. Mono is 0.0. Uncorrelated stereo ~0.5."""
    if samples.ndim == 1 or samples.shape[0] < 2:
        return 0.0
    left = samples[0]
    right = samples[1]
    mid = 0.5 * (left + right)
    side = 0.5 * (left - right)
    energy_mid = float(np.mean(mid**2))
    energy_side = float(np.mean(side**2))
    return float(energy_side / (energy_mid + energy_side + 1e-12))
