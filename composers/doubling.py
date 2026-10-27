"""
Multi-take recording — double / quad tracked parts.

Metal and post-hardcore rhythm guitars are almost always recorded at least
twice, panned hard left and right. Sometimes both takes play exactly the
same part (tight breakdowns), sometimes the second take varies (a
different high note, a pinch harmonic somewhere else), sometimes one side
plays the part higher (octave chords in a chorus) or harmonizes it.

make_takes() turns one composed part into 2 or 4 takes. The relationship is
the instrument's DoubleTrack.mode, overridden per section by the
`double_track` binding. In every mode the palm-muted chugs and the pedal
notes stay identical — that is what keeps a double-tracked riff tight —
and each take gets its own timing/velocity humanization (done by the
pipeline afterwards).

Quad tracking (takes=4) adds an inner pair (L2/R2 at ±inner_pan) that plays
the main part identically, as producers stack "two per side".
"""

from typing import List, Optional
import random

from midi_types.music import Note, Section, Track
from midi_types.genre_profile import DoubleTrack
from .theory import SCALES, parse_key, midi_to_pitch_str
from .harmony import ChordSpan, span_at


def make_takes(track: Track, cfg: DoubleTrack, sections: List[Section],
               timeline: List[ChordSpan], key: str, register, rng: random.Random) -> List[Track]:
    from .structure import leaf_sections
    leaves = leaf_sections(sections)
    tonic_pc, _ = parse_key(key)
    left = _clone(track, f"{track.name} L", -cfg.pan, track.notes)

    right_notes: List[Note] = []
    for n in track.notes:
        sec = _section_at(leaves, n.start)
        mode = (sec.bindings.get("double_track") if sec else None) or cfg.mode
        right_notes.extend(_vary(n, mode, cfg, timeline, tonic_pc, register, rng))
    right = _clone(track, f"{track.name} R", cfg.pan, right_notes)

    takes = [left, right]
    if cfg.takes >= 4:
        takes.append(_clone(track, f"{track.name} L2", -cfg.inner_pan, track.notes))
        takes.append(_clone(track, f"{track.name} R2", cfg.inner_pan, right_notes))
    return takes


def _clone(track: Track, name: str, pan: float, notes: List[Note]) -> Track:
    return Track(
        notes=[Note(n.pitch, n.duration, n.velocity, n.start, n.articulation, n.drum) for n in notes],
        instrument=track.instrument, is_drum=track.is_drum, name=name,
        role=track.role, vst=track.vst, pan=pan,
        automation=[dict(e) for e in track.automation],
    )


def _section_at(leaves: List[Section], beat: Optional[float]) -> Optional[Section]:
    if beat is None:
        return None
    for s in leaves:
        if s.start_beat() - 1e-6 <= beat < s.end_beat() - 1e-6:
            return s
    return None


def _vary(n: Note, mode: str, cfg: DoubleTrack, timeline, tonic_pc, register, rng) -> List[Note]:
    from skills.generate_midi import parse_pitch
    same = Note(n.pitch, n.duration, n.velocity, n.start, n.articulation, n.drum)
    if mode == "identical" or n.articulation == "palm_mute" or n.start is None:
        return [same]
    p = parse_pitch(n.pitch)
    span = span_at(timeline, n.start)
    scale = SCALES.get(span.scale if span else "minor", SCALES["minor"])
    pcs = sorted({(tonic_pc + i) % 12 for i in scale})

    if mode == "octave":
        up = p + 12
        return [Note(midi_to_pitch_str(up), n.duration, n.velocity, n.start, n.articulation)] \
            if up <= register[1] + 12 else [same]
    if mode == "harmony":
        if p % 12 in pcs:
            idx = pcs.index(p % 12)
            up = p + ((pcs[(idx + 2) % len(pcs)] - p) % 12 or 12)
            return [Note(midi_to_pitch_str(up), n.duration, n.velocity, n.start, n.articulation)]
        return [same]
    if mode == "variation" and rng.random() < cfg.variation_rate:
        roll = rng.random()
        if roll < 0.5 and p % 12 in pcs:
            # neighbouring scale tone instead of the same high note
            idx = pcs.index(p % 12) + rng.choice([-1, 1])
            target_pc = pcs[idx % len(pcs)]
            delta = ((target_pc - p) % 12)
            if delta > 6:
                delta -= 12
            return [Note(midi_to_pitch_str(p + delta), n.duration, n.velocity, n.start, n.articulation)]
        if roll < 0.8:
            # pinch harmonic lands on the other take's note (or comes off it)
            art = "open" if n.articulation == "pinch_harmonic" else "pinch_harmonic"
            return [Note(n.pitch, n.duration, n.velocity, n.start, art)]
        # this take picks the note a little softer
        return [Note(n.pitch, n.duration, (n.velocity or 0.8) * 0.92, n.start, n.articulation)]
    return [same]
