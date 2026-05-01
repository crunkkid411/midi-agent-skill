"""
Genre profile loading, blending, and querying.
"""

from typing import List, Tuple, Optional, Dict, Any
from pathlib import Path
import random

from midi_types.genre_profile import (
    GenreProfile, ChordProgression, DrumPattern, SongSection, MicroSection,
    InstrumentRole, find_profile,
)


def load_profile(slug: str, genres_dir: Optional[Path] = None) -> GenreProfile:
    """Load a profile by slug or alias. Raises FileNotFoundError if missing."""
    path = find_profile(slug, genres_dir)
    if path is None:
        raise FileNotFoundError(
            f"No genre profile for '{slug}'. "
            f"Run the research workflow (workflows/music-theory-research.md) "
            f"and add genres/{slug}.json."
        )
    return GenreProfile.load(path)


def pick_weighted(items: List[Any], get_weight=lambda x: getattr(x, "weight", 1.0),
                  rng: Optional[random.Random] = None) -> Any:
    """Weighted random choice from a list of objects with `.weight`."""
    rng = rng or random
    weights = [max(0.0, get_weight(i)) for i in items]
    total = sum(weights)
    if total <= 0:
        return rng.choice(items)
    r = rng.random() * total
    acc = 0.0
    for item, w in zip(items, weights):
        acc += w
        if r <= acc:
            return item
    return items[-1]


def blend_profiles(profiles_with_weights: List[Tuple[GenreProfile, float]]) -> GenreProfile:
    """Blend multiple genre profiles into one synthetic profile.

    Strategy:
      - tempo: weighted average, clamped to overlap of ranges (or full union if disjoint)
      - key/scale: pick the highest-weighted profile's default
      - chord_progressions: union, with each entry's weight scaled by its source profile's weight
      - drum_patterns: same union strategy
      - song_sections: take the highest-weighted profile's structure (more coherent than mixing)
      - instruments: take from highest-weighted profile, supplemented with distinctive roles from others
      - principles: concatenate (deduped), prefixed with origin genre name
    """
    if not profiles_with_weights:
        raise ValueError("blend_profiles needs at least one profile")
    if len(profiles_with_weights) == 1:
        return profiles_with_weights[0][0]

    # Normalize weights
    total_w = sum(w for _, w in profiles_with_weights) or 1.0
    pw = [(p, w / total_w) for p, w in profiles_with_weights]
    primary = max(pw, key=lambda x: x[1])[0]

    # Tempo
    overlap_lo = max(p.tempo_range[0] for p, _ in pw)
    overlap_hi = min(p.tempo_range[1] for p, _ in pw)
    if overlap_lo <= overlap_hi:
        tempo_range = (overlap_lo, overlap_hi)
    else:
        # Disjoint — weighted average of midpoints
        avg = sum(((p.tempo_range[0] + p.tempo_range[1]) / 2) * w for p, w in pw)
        tempo_range = (int(avg) - 5, int(avg) + 5)

    # Chord progressions: union, scale weights
    blended_progs = []
    for p, w in pw:
        for cp in p.chord_progressions:
            blended_progs.append(ChordProgression(
                roman=cp.roman, weight=cp.weight * w,
                source=f"{cp.source or ''} [{p.name}@{w:.2f}]".strip(),
                bars_per_chord=cp.bars_per_chord,
            ))

    # Drum patterns: union, scaled weights
    blended_drums = []
    for p, w in pw:
        for d in p.drum_patterns:
            blended_drums.append(DrumPattern(
                name=f"{d.name}__{p.name}", kick=d.kick, snare=d.snare,
                hh=d.hh, oh=d.oh, clap=d.clap, perc=d.perc,
                swing=d.swing, ghost_density=d.ghost_density,
                weight=d.weight * w,
            ))

    # Instruments: primary's roles, plus any distinctive role from others
    inst_by_role: Dict[str, InstrumentRole] = {i.role: i for i in primary.instruments}
    for p, _ in pw:
        if p is primary:
            continue
        for i in p.instruments:
            if i.role not in inst_by_role:
                inst_by_role[i.role] = i

    # Principles: prefix with genre name
    principles = []
    for p, w in pw:
        prefix = f"[{p.name} @ {w:.2f}] "
        for principle in p.principles:
            principles.append(prefix + principle)

    blended = GenreProfile(
        name="+".join(p.name for p, _ in pw),
        aliases=[],
        tempo_range=tempo_range,
        default_key=primary.default_key,
        common_keys=primary.common_keys,
        scales=list(dict.fromkeys(s for p, _ in pw for s in p.scales)),
        chord_progressions=blended_progs,
        voicing_style=primary.voicing_style,
        drum_patterns=blended_drums,
        song_sections=primary.song_sections,  # use primary's structure for coherence
        instruments=list(inst_by_role.values()),
        velocity_ranges=dict(primary.velocity_ranges),
        principles=principles,
        references=[r for p, _ in pw for r in p.references],
        research_sources=[s for p, _ in pw for s in p.research_sources],
        sound_design=primary.sound_design,
        notes=f"Blended: {', '.join(f'{p.name}@{w:.2f}' for p, w in pw)}",
    )
    return blended


def get_instrument(profile: GenreProfile, role: str) -> Optional[InstrumentRole]:
    for i in profile.instruments:
        if i.role == role:
            return i
    return None


def velocity_range(profile: GenreProfile, role: str) -> Tuple[float, float]:
    if role in profile.velocity_ranges:
        return profile.velocity_ranges[role]
    inst = get_instrument(profile, role)
    if inst:
        return inst.velocity_range
    return (0.5, 0.8)
