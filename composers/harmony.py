"""
Harmony timeline + motif repetition — shared by every pitched composer.

The timeline is the song's chord plan flattened to beat spans:
    [ChordSpan(start=0, end=4, roman='i', scale='phrygian', section=verse.A), ...]
Progressions are chosen per top-level section (or per micro-section when
the micro binds its own `progression`), so every layer reads the same
chord at the same beat no matter how sections are subdivided.

MotifPlan makes ideas repeat. Instead of re-rolling every bar, a composer
asks for a per-bar RNG: bars at the same position inside the motif get the
same seed, so the rhythm and intervallic shape repeat while the pitches
follow the current chord root. Variations (last_bar, last_half,
octave_drop, last_root) are applied on later repetitions. The motif key is
the section's base name, so verse2 replays verse's riff.
"""

from dataclasses import dataclass
from typing import List, Optional, Tuple
import random

from midi_types.music import Section
from midi_types.genre_profile import GenreProfile, ChordProgression
from .theory import parse_key
from .profile import progression_for, scale_for
from .structure import top_sections, leaf_sections


@dataclass
class ChordSpan:
    start: float                  # beats
    end: float                    # beats (exclusive)
    roman: str
    scale: str                    # scale in force (section override or key scale)
    section: Section              # leaf section the span starts in
    progression: ChordProgression


def build_timeline(profile: GenreProfile, sections: List[Section], key: Optional[str],
                   rng: random.Random) -> Tuple[List[ChordSpan], List[Tuple[Section, ChordProgression]]]:
    """Returns (timeline, plan). plan = [(top_section, progression)] for callers
    that still want the coarse view."""
    _, key_scale = parse_key(key or profile.default_key)
    timeline: List[ChordSpan] = []
    plan: List[Tuple[Section, ChordProgression]] = []
    if not profile.chord_progressions:
        return timeline, plan

    leaves = leaf_sections(sections)
    for top in top_sections(sections):
        top_prog = progression_for(profile, top, rng)
        plan.append((top, top_prog))
        kids = [l for l in leaves if l.name == top.name or l.name.startswith(top.name + ".")]
        for leaf in kids:
            own = leaf.bindings.get("progression")
            if leaf is not top and own is not None and own != top.bindings.get("progression"):
                prog, origin = progression_for(profile, leaf, rng), leaf.start_bar
            else:
                prog, origin = top_prog, top.start_bar
            scale = scale_for(leaf, key_scale)
            bpc = max(0.25, prog.bars_per_chord)
            beat = leaf.start_beat()
            end = leaf.end_beat()
            while beat < end - 1e-6:
                idx = int((beat - (origin - 1) * 4 + 1e-6) // (bpc * 4))
                chord_end = min(end, (origin - 1) * 4 + (idx + 1) * bpc * 4)
                if chord_end <= beat:
                    chord_end = min(end, beat + bpc * 4)
                timeline.append(ChordSpan(beat, chord_end, prog.roman[idx % len(prog.roman)],
                                          scale, leaf, prog))
                beat = chord_end
    return timeline, plan


def span_at(timeline: List[ChordSpan], beat: float) -> Optional[ChordSpan]:
    for s in timeline:
        if s.start <= beat + 1e-6 < s.end:
            return s
    return timeline[-1] if timeline and beat >= timeline[-1].end else None


def spans_in(timeline: List[ChordSpan], start: float, end: float) -> List[ChordSpan]:
    return [s for s in timeline if s.start < end and s.end > start]


# -------- Motif repetition --------

@dataclass
class BarPlan:
    rng: random.Random
    repetition: int               # which repetition of the motif (0 = first statement)
    position: int                 # bar index inside the motif
    octave_drop: bool = False
    last_root: bool = False
    fresh: bool = False           # this bar was regenerated rather than repeated


class MotifPlan:
    def __init__(self, profile: GenreProfile, role: str, rng: random.Random):
        cfg = profile.motif
        self.enabled = cfg is not None and role in cfg.roles
        self.bars = max(1, cfg.bars) if cfg else 1
        self.variations = (cfg.variations or ["exact"]) if cfg else ["exact"]
        self.seed = rng.randrange(1 << 30)
        self.role = role
        self._fallback = random.Random(self.seed)

    def bar(self, section: Section, bar_in_section: int) -> BarPlan:
        if not self.enabled:
            return BarPlan(random.Random(self._fallback.random()), 0, bar_in_section, fresh=True)
        key = section.base + ("." + section.name.partition(".")[2] if section.is_micro else "")
        rep, pos = divmod(bar_in_section, self.bars)
        variation = self.variations[rep % len(self.variations)] if rep > 0 else "exact"
        fresh = ((variation == "last_bar" and pos == self.bars - 1)
                 or (variation == "last_half" and pos >= self.bars / 2))
        seed = f"{self.seed}:{self.role}:{key}:{pos}"
        if fresh:
            seed += f":fresh:{rep}"
        return BarPlan(
            rng=random.Random(seed), repetition=rep, position=pos,
            octave_drop=(variation == "octave_drop"),
            last_root=(variation == "last_root" and pos == self.bars - 1),
            fresh=fresh,
        )
