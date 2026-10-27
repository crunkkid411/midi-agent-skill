"""
Drum track composer.

Renders per leaf section (micro-sections where they exist), so verse.B can
play a different groove than verse.A. Pattern choice per leaf:
  1. section binding `drum_pattern` — a name, or a list played one per
     phrase (pattern_sequence: e.g. ["double-kick", "double-kick", "blast"])
  2. density < 0.4 → the lowest-energy pattern (legacy behaviour)
  3. weighted pick, shared by all micro-sections of one top-level section

Any lane in the pattern grid is rendered (kick, snare, hh, oh, clap, perc
plus crash, china, ride, ride_bell, splash, tom_*, floor_tom,
snare_rimshot, cowbell, ...). Notes carry `drum=<lane>`; the renderer maps
lanes onto the target kit (GM, Superior Drummer 3, ...).

Musical behaviour layered on top of the grid:
  - kick_follow: the kick doubles the rhythm guitar's low attacks
  - crash (+ kick) on every top-level section downbeat (binding crash=false
    turns it off) and when the groove changes inside a section
  - phrase-end fills from profile.fills (snare_roll, tom_run, triplet_toms,
    kick_snare_burst, snare_build, open_hat, drop_out, stop_hit)
  - stop sections: one kick+crash hit, then silence
  - energy (0-1) scales velocity; density gates hats/ghosts
"""

from typing import Dict, List, Optional
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile, DrumPattern
from .theory import GM_DRUM
from .profile import pick_weighted, velocity_range, find_drum_pattern
from .structure import (
    section_density, section_energy, leaf_sections, top_sections, is_muted, is_stop,
)


# Below this density a lane is thinned to every other hit.
LANE_DENSITY_THRESHOLD = {
    "hh": 0.35, "oh": 0.5, "perc": 0.5, "clap": 0.4, "ride": 0.35, "ride_bell": 0.45,
    "china": 0.3, "cowbell": 0.5, "shaker": 0.45, "tambourine": 0.45,
}
CYMBAL_LANES = {"hh", "oh", "ride", "ride_bell", "china", "crash", "splash", "hh_tight"}


def generate_drums(
    profile: GenreProfile,
    sections: List[Section],
    rng: Optional[random.Random] = None,
    riff_attacks: Optional[list] = None,
) -> Track:
    rng = rng or random.Random()
    if not profile.drum_patterns:
        return Track(notes=[], name="Drums", is_drum=True, role="drums")

    vmin, vmax = velocity_range(profile, "drums")
    notes: List[Note] = []
    tops = {s.name: s for s in top_sections(sections)}
    top_choice: Dict[str, DrumPattern] = {}
    fills = profile.fills
    phrase = fills.phrase_bars if fills else 4

    for leaf in leaf_sections(sections):
        if is_muted(leaf, "drums"):
            continue
        top = tops.get(leaf.name.partition(".")[0], leaf)
        density = section_density(profile, leaf)
        energy = section_energy(profile, leaf)
        lo, hi = _vel_window(vmin, vmax, energy)

        if is_stop(leaf):
            t = leaf.start_beat()
            for lane in ("kick", "crash", "china"):
                notes.append(_note(lane, t, hi, "8"))
            continue

        patterns = _patterns_for(profile, leaf, top, density, rng, top_choice)
        follow = leaf.bindings.get("kick_follow")
        prev = None
        for bar in range(leaf.start_bar, leaf.end_bar + 1):
            idx = (bar - leaf.start_bar) // phrase
            pattern = patterns[idx % len(patterns)]
            kick_follow = follow if follow is not None else pattern.kick_follow
            _emit_bar(pattern, bar, density, lo, hi, rng, notes,
                      riff_attacks if kick_follow else None)
            downbeat = (bar - 1) * 4
            crash_flag = leaf.bindings.get("crash")
            leaf_downbeat = bar == leaf.start_bar and crash_flag is not False and \
                (bar == top.start_bar or crash_flag is True)
            crash_here = leaf_downbeat or (prev is not None and pattern is not prev)
            if crash_here and energy >= 0.3:
                _crash(notes, downbeat, hi)
            prev = pattern

        if fills:
            _place_fills(profile, leaf, top, phrase, density, energy, lo, hi, rng, notes)

    notes.sort(key=lambda n: (n.start, n.drum or ""))
    return Track(name="Drums", instrument=None, is_drum=True, notes=notes, role="drums")


# -------- pattern selection --------

def _patterns_for(profile, leaf, top, density, rng, cache) -> List[DrumPattern]:
    want = leaf.bindings.get("drum_pattern")
    if want:
        names = want if isinstance(want, list) else [want]
        found = [p for p in (find_drum_pattern(profile, n) for n in names) if p]
        if found:
            return found
    if density < 0.4 and len(profile.drum_patterns) > 1:
        return [min(profile.drum_patterns, key=_energy)]
    if top.name not in cache:
        cache[top.name] = pick_weighted(profile.drum_patterns, rng=rng)
    return [cache[top.name]]


def _energy(p: DrumPattern) -> float:
    """Rough energy estimate for selecting low-energy patterns."""
    def hits(s: str) -> int:
        return sum(1 for c in s if c not in (".", " "))
    lanes = p.all_lanes()
    return sum(hits(v) * (0.5 if k in CYMBAL_LANES else 1.0) for k, v in lanes.items())


# -------- rendering --------

def _vel_window(vmin: float, vmax: float, energy: float):
    scale = 0.8 + 0.2 * max(0.0, min(1.0, energy))
    return vmin * scale, min(1.0, vmax * scale)


def _note(lane: str, start: float, vel: float, dur: str = "16") -> Note:
    return Note(pitch=str(GM_DRUM.get(lane, 36)), duration=dur, velocity=round(min(1.0, vel), 3),
                start=round(start, 4), drum=lane)


def _crash(notes: List[Note], t: float, vel: float) -> None:
    notes[:] = [n for n in notes if not (abs(n.start - t) < 1e-6 and n.drum in ("hh", "ride", "china"))]
    notes.append(_note("crash", t, vel))
    if not any(abs(n.start - t) < 1e-6 and n.drum == "kick" for n in notes):
        notes.append(_note("kick", t, vel))


def _emit_bar(pattern: DrumPattern, bar: int, density: float,
              vmin: float, vmax: float, rng: random.Random,
              notes_out: List[Note], riff_attacks: Optional[list]) -> None:
    """Expand a single bar of the pattern's grids."""
    bar_start_beat = (bar - 1) * 4

    for lane, grid in pattern.all_lanes().items():
        if lane == "kick" and riff_attacks is not None:
            _follow_kick(riff_attacks, bar_start_beat, vmin, vmax, rng, notes_out)
            continue
        steps = len(grid)
        beats_per_step = 4.0 / steps
        half_skip = density < LANE_DENSITY_THRESHOLD.get(lane, 0.0)

        for i, ch in enumerate(grid):
            if ch in (".", " "):
                continue
            if half_skip and i % 2 == 1:
                continue

            offset = i * beats_per_step
            if pattern.swing > 0.5 and i % 2 == 1 and steps in (8, 16, 32):
                offset += beats_per_step * (pattern.swing - 0.5) * 2 * 0.5

            if ch == "g":
                vel = rng.uniform(vmin, vmin + (vmax - vmin) * 0.3)
            else:
                vel = rng.uniform(vmin + (vmax - vmin) * 0.5, vmax)
                if ch == "a" or i == 0:
                    vel = min(1.0, vel * 1.08)
            notes_out.append(_note(lane, bar_start_beat + offset, vel))

        # Ghost-note injection
        if pattern.ghost_density > 0 and lane in ("snare", "hh") and density >= 0.5:
            for i in range(steps):
                if grid[i] in (".", " ") and rng.random() < pattern.ghost_density / 4:
                    notes_out.append(_note(lane, bar_start_beat + i * beats_per_step,
                                           rng.uniform(vmin, vmin + (vmax - vmin) * 0.25)))


def _follow_kick(attacks, bar_start, vmin, vmax, rng, out) -> None:
    seen = set()
    for a in attacks:
        if bar_start <= a.start < bar_start + 4 and a.low and a.start not in seen:
            seen.add(a.start)
            vel = rng.uniform(vmin + (vmax - vmin) * 0.6, vmax) * (1.05 if a.accent else 1.0)
            out.append(_note("kick", a.start, vel))


# -------- fills --------

def _place_fills(profile, leaf, top, phrase, density, energy, lo, hi, rng, notes) -> None:
    cfg = profile.fills
    allow = leaf.bindings.get("fill")
    if allow is False or density < 0.3:
        return
    leaf_is_last = leaf.end_bar == top.end_bar
    for bar in range(leaf.start_bar, leaf.end_bar + 1):
        phrase_end = (bar - top.start_bar + 1) % phrase == 0
        section_end = leaf_is_last and bar == leaf.end_bar
        if not (phrase_end or section_end):
            continue
        prob = cfg.section_end_probability if section_end else cfg.probability
        if allow is True and section_end:
            prob = 1.0
        if rng.random() >= prob:
            continue
        beats = rng.choice(cfg.beats)
        if section_end and len(cfg.beats) > 1:
            beats = max(cfg.beats)
        kind = rng.choice(cfg.types)
        end = bar * 4
        start = end - beats
        notes[:] = [n for n in notes if not (start - 1e-6 <= n.start < end and n.drum != "crash")]
        FILLS.get(kind, _fill_snare_roll)(start, end, lo, hi, rng, notes, energy)


def _ramp(i: int, n: int, lo: float, hi: float) -> float:
    return lo + (hi - lo) * (0.55 + 0.45 * i / max(1, n - 1))


def _fill_snare_roll(start, end, lo, hi, rng, notes, energy):
    n = int(round((end - start) / 0.25))
    for i in range(n):
        notes.append(_note("snare", start + i * 0.25, _ramp(i, n, lo, hi)))


def _fill_tom_run(start, end, lo, hi, rng, notes, energy):
    toms = ["snare", "tom_high", "tom_mid", "tom_low", "floor_tom"]
    n = int(round((end - start) / 0.25))
    group = 2 if n <= 8 else 4
    for i in range(n):
        lane = toms[min(len(toms) - 1, i * len(toms) // n)]
        notes.append(_note(lane, start + i * 0.25, _ramp(i, n, lo, hi)))
        if i % group == group - 1 and energy >= 0.6:
            notes.append(_note("kick", start + i * 0.25, _ramp(i, n, lo, hi)))


def _fill_triplet_toms(start, end, lo, hi, rng, notes, energy):
    toms = ["tom_high", "tom_mid", "floor_tom"]
    n = int(round((end - start) * 3))
    for i in range(n):
        lane = "kick" if i % 3 == 2 else toms[(i // 3) % len(toms)]
        notes.append(_note(lane, start + i / 3, _ramp(i, n, lo, hi)))


def _fill_kick_snare_burst(start, end, lo, hi, rng, notes, energy):
    n = int(round((end - start) / 0.25))
    for i in range(n):
        lane = "kick" if i % 2 else rng.choice(["snare", "tom_high", "tom_mid", "floor_tom"])
        notes.append(_note(lane, start + i * 0.25, _ramp(i, n, lo, hi)))


def _fill_snare_build(start, end, lo, hi, rng, notes, energy):
    t, step = start, 0.5
    while t < end - 1e-6:
        if t >= start + (end - start) / 2:
            step = 0.25
        notes.append(_note("snare", t, _ramp(int((t - start) * 4), int((end - start) * 4), lo, hi)))
        t += step


def _fill_open_hat(start, end, lo, hi, rng, notes, energy):
    notes.append(_note("kick", start, hi * 0.9))
    notes.append(_note("snare", end - 1, hi))
    notes.append(_note("oh", end - 0.5, hi * 0.9, "8"))


def _fill_drop_out(start, end, lo, hi, rng, notes, energy):
    notes.append(_note("snare", start, lo, "16"))


def _fill_stop_hit(start, end, lo, hi, rng, notes, energy):
    for lane in ("kick", "snare", "crash"):
        notes.append(_note(lane, start, hi, "8"))


FILLS = {
    "snare_roll": _fill_snare_roll,
    "tom_run": _fill_tom_run,
    "triplet_toms": _fill_triplet_toms,
    "kick_snare_burst": _fill_kick_snare_burst,
    "snare_build": _fill_snare_build,
    "open_hat": _fill_open_hat,
    "drop_out": _fill_drop_out,
    "stop_hit": _fill_stop_hit,
}
