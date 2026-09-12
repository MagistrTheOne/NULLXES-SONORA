from __future__ import annotations

from app.schemas.audio import AudioFeatures, Issue


def detect_issues(features: AudioFeatures) -> list[Issue]:
    """Engineering labels derived from DSP features. Not LLM output."""
    issues: list[Issue] = []
    dist = features.frequency_distribution

    if features.peak >= 0.99:
        severity = min(1.0, 0.7 + (features.peak - 0.99) * 10)
        issues.append(
            Issue(
                type="clipping",
                severity=round(severity, 3),
                area="full_band",
                detail="Peak amplitude is at or above 0.99 FS",
            )
        )

    low_end = dist.sub + dist.low
    if low_end >= 0.50:
        issues.append(
            Issue(
                type="muddy_low_end",
                severity=round(min(1.0, (low_end - 0.35) / 0.45), 3),
                area="low_end",
                detail=f"Sub+low energy share is {low_end:.2f}",
            )
        )

    if dist.low >= 0.26 and dist.mid >= 0.26:
        conflict = min(dist.low, dist.mid)
        issues.append(
            Issue(
                type="frequency_conflict",
                severity=round(min(1.0, conflict / 0.40), 3),
                area="low_end",
                detail="Low and mid bands both carry high energy share",
            )
        )

    if features.channels == 1 or features.stereo_width < 0.08:
        severity = 0.85 if features.channels == 1 else round(
            min(1.0, (0.08 - features.stereo_width) / 0.08), 3
        )
        issues.append(
            Issue(
                type="narrow_stereo",
                severity=severity,
                area="stereo_image",
                detail=(
                    "Source is mono"
                    if features.channels == 1
                    else f"Stereo width is {features.stereo_width:.3f}"
                ),
            )
        )

    if features.dynamic_range_db < 6.0:
        issues.append(
            Issue(
                type="low_dynamic_range",
                severity=round(min(1.0, (6.0 - features.dynamic_range_db) / 6.0), 3),
                area="dynamics",
                detail=f"Crest factor is {features.dynamic_range_db:.2f} dB",
            )
        )

    return issues
