"""
Variation: how far a new take strays from the choices a song already made.

Every section carries bindings (its drum groove, chord progression, riff
style, ...). The seed alone only re-rolls the small stuff (fills, humanize
timing), so a regenerated section with the same bindings sounds the same.
`vary_sections` swaps some of those bound choices for others the genre(s)
know, so "regenerate" gives a genuinely different idea:

    amount 0.0  keep every bound choice (only timing/fills change)
    amount 0.5  about half the bound choices are re-picked
    amount 1.0  every bound choice is re-picked

The pool for each key is every value the blended profile offers: its drum
patterns and progressions (already weighted per genre by blend_profiles),
plus every style the source genres' sections use, weighted by genre weight.
This is also what makes a genre blend audible: the second genre's grooves and
progressions are in the pool, so they replace some of the primary's.
"""

import random
from typing import Any, Dict, List, Optional, Sequence, Tuple

from midi_types.genre_profile import GenreProfile
from midi_types.music import Section

REROLL_KEYS = ("drum_pattern", "electronic_pattern", "progression", "riff_style",
               "bass_style", "melody_style", "chords_style", "voicing", "double_track")

Pools = Dict[str, List[Tuple[Any, float]]]


def binding_pools(blended: GenreProfile,
                  sources: Sequence[Tuple[GenreProfile, float]]) -> Pools:
    pools: Pools = {k: [] for k in REROLL_KEYS}
    for d in blended.drum_patterns:
        key = "electronic_pattern" if d.layer == "electronic" else "drum_pattern"
        pools[key].append((d.name, d.weight))
    for cp in blended.chord_progressions:
        if cp.name:
            pools["progression"].append((cp.name, cp.weight))
    total = sum(w for _, w in sources) or 1.0
    for p, w in sources:
        share = w / total
        for ss in p.song_sections:
            for holder in [ss] + list(ss.micro):
                for k in REROLL_KEYS:
                    if k in ("drum_pattern", "electronic_pattern", "progression"):
                        continue  # those come from the blended lists above
                    v = getattr(holder, k, None)
                    for item in (v if isinstance(v, list) else [v]):
                        if isinstance(item, str) and item:
                            pools[k].append((item, share))
    return pools


def blend_amount(weights: Sequence[float]) -> float:
    """How much of a fresh blend comes from re-picked choices: 0 for one genre,
    1 for an even two-way blend (the sections then draw evenly from both)."""
    if len(weights) < 2:
        return 0.0
    total = sum(weights) or 1.0
    return min(1.0, 2.0 * (1.0 - max(weights) / total))


def _pick(pool: List[Tuple[Any, float]], avoid: set, rng: random.Random) -> Optional[Any]:
    choices = [(v, w) for v, w in pool if v not in avoid and w > 0]
    if not choices:
        return None
    total = sum(w for _, w in choices)
    r = rng.random() * total
    for v, w in choices:
        r -= w
        if r <= 0:
            return v
    return choices[-1][0]


def vary_sections(sections: List[Section], pools: Pools, amount: float,
                  rng: random.Random) -> List[Section]:
    """Re-pick bound choices in place (micro sections follow their parent's new
    value unless they bound their own). Returns the same list."""
    if amount <= 0 or not pools:
        return sections
    original = {(s.name, k): s.bindings.get(k) for s in sections if not s.is_micro for k in REROLL_KEYS}
    parent_new: Dict[Tuple[str, str], Any] = {}
    for s in sorted(sections, key=lambda x: x.is_micro):
        b = s.bindings
        for k in REROLL_KEYS:
            if k not in b or not pools.get(k):
                continue
            parent = s.name.partition(".")[0]
            if s.is_micro and (parent, k) in parent_new and b.get(k) == original.get((parent, k)):
                b[k] = parent_new[(parent, k)]
                continue
            if rng.random() >= amount:
                continue
            cur = b[k]
            new = _pick(pools[k], set(cur) if isinstance(cur, list) else {cur}, rng)
            if new is not None:
                b[k] = new
                if not s.is_micro:
                    parent_new[(s.name, k)] = new
    return sections
