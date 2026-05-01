"""
Drum track composer.

Picks a drum pattern from the profile, expands the 16-step grid to MIDI
notes on channel 9 (GM drums). Honors per-section density (low density →
drop hi-hat, halve kick) and ghost-note injection.
"""

from typing import List, Optional
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile, DrumPattern
from .theory import drum_pitch
from .profile import pick_weighted, velocity_range
from .structure import section_density


DRUM_LANES = ["kick", "snare", "clap", "hh", "oh", "perc"]


def generate_drums(
    profile: GenreProfile,
    sections: List[Section],
    rng: Optional[random.Random] = None,
) -> Track:
    rng = rng or random.Random()
    if not profile.drum_patterns:
        return Track(notes=[], name="Drums", is_drum=True)

    vmin, vmax = velocity_range(profile, "drums")
    notes: List[Note] = []

    # Top-level sections dictate which pattern variant plays
    top_sections = [s for s in sections if "." not in s.name]

    for section in top_sections:
        density = section_density(profile, section)
        # Pick a pattern; for low-density sections prefer the lowest-energy one
        if density < 0.4 and len(profile.drum_patterns) > 1:
            pattern = min(profile.drum_patterns, key=lambda d: _energy(d))
        else:
            pattern = pick_weighted(profile.drum_patterns, rng=rng)

        # Expand for each bar in this section
        for bar in range(section.start_bar, section.end_bar + 1):
            _emit_bar(pattern, bar, density, vmin, vmax, rng, notes)

    return Track(
        name="Drums",
        instrument=None,
        is_drum=True,
        notes=notes,
    )


def _emit_bar(pattern: DrumPattern, bar: int, density: float,
              vmin: float, vmax: float, rng: random.Random,
              notes_out: List[Note]) -> None:
    """Expand a single bar of the pattern's 16-step grid."""
    bar_start_beat = (bar - 1) * 4

    for lane in DRUM_LANES:
        grid = getattr(pattern, lane, "")
        if not grid:
            continue
        steps = len(grid)
        if steps == 0:
            continue
        beats_per_step = 4.0 / steps  # 16 steps over 4 beats = 0.25

        # Density gating: drop hi-hats and ghosts more aggressively when low density
        lane_density_threshold = {
            "hh": 0.35, "oh": 0.5, "perc": 0.5, "clap": 0.4,
            "kick": 0.0, "snare": 0.1,
        }.get(lane, 0.0)
        if density < lane_density_threshold:
            # Halve the lane: drop every other hit
            half_skip = True
        else:
            half_skip = False

        pitch = drum_pitch(lane)
        # Swing: shift offbeat 16ths slightly later
        for i, ch in enumerate(grid):
            if ch in (".", " "):
                continue
            if half_skip and i % 2 == 1:
                continue

            offset = i * beats_per_step
            # Apply swing on offbeat 16ths
            if pattern.swing > 0.5 and i % 2 == 1:
                offset += beats_per_step * (pattern.swing - 0.5) * 2 * 0.5

            if ch == "g":
                vel = rng.uniform(vmin, vmin + (vmax - vmin) * 0.3)
            else:
                vel = rng.uniform(vmin + (vmax - vmin) * 0.5, vmax)
                # Velocity emphasis: downbeats louder
                if i == 0:
                    vel = min(1.0, vel * 1.05)

            notes_out.append(Note(
                pitch=str(pitch),
                duration="16",
                velocity=round(vel, 3),
                start=round(bar_start_beat + offset, 4),
            ))

        # Ghost-note injection
        if pattern.ghost_density > 0 and lane in ("snare", "hh") and density >= 0.5:
            for i in range(steps):
                if grid[i] in (".", " ") and rng.random() < pattern.ghost_density / 4:
                    offset = i * beats_per_step
                    notes_out.append(Note(
                        pitch=str(pitch), duration="16",
                        velocity=round(rng.uniform(vmin, vmin + (vmax - vmin) * 0.25), 3),
                        start=round(bar_start_beat + offset, 4),
                    ))


def _energy(p: DrumPattern) -> float:
    """Rough energy estimate for selecting low-energy patterns."""
    def hits(s: str) -> int:
        return sum(1 for c in s if c not in (".", " "))
    return hits(p.kick) + hits(p.snare) + hits(p.hh) * 0.5 + hits(p.oh) + hits(p.clap)
