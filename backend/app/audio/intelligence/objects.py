from __future__ import annotations

from app.schemas.audio import Issue
from app.schemas.dna import MixCharacter, SonoraObject, TrackStructure


def build_objects(
    *,
    issues: list[Issue],
    structure: TrackStructure,
    mix: MixCharacter,
) -> list[SonoraObject]:
    objects = [
        SonoraObject(
            type="ARRANGEMENT_MAP",
            input="structure detection",
            parameters={
                "sections": [
                    {
                        "name": section.name,
                        "start": section.start,
                        "end": section.end,
                        "energy": section.energy,
                    }
                    for section in structure.sections
                ]
            },
            status="generated",
        )
    ]

    types = {issue.type for issue in issues}
    low_risk = mix.low_end.risk == "high" or "muddy_low_end" in types or "frequency_conflict" in types
    if low_risk:
        frequency = 120.0 if "frequency_conflict" not in types else 250.0
        if mix.low_end.why.startswith("Sub"):
            frequency = 55.0
        objects.append(
            SonoraObject(
                type="EQ_PROFILE",
                input=mix.low_end.why,
                parameters={"frequency": frequency, "gain": -3.0, "q": 1.2},
                status="generated",
            )
        )

    if "clipping" in types or "low_dynamic_range" in types:
        objects.append(
            SonoraObject(
                type="MASTER_CHAIN",
                input="peak / crest",
                parameters={
                    "input_gain_db": -3.0 if "clipping" in types else 0.0,
                    "limiter": "clipping" in types,
                    "restore_dynamics": "low_dynamic_range" in types,
                },
                status="generated",
            )
        )

    return objects
