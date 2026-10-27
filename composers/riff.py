"""
Rhythm-guitar riff composer (role: rhythm_guitar).

Genres built on riffs (metalcore, djent, thrash, punk, hard rock) don't
work as "chords + parts on top": the riff IS the rhythm section, and the
kick and bass lock to it. This composer writes the riff from the harmony
timeline and returns its attack times so drums (kick_follow) and bass
(style 'doubled') can lock to it.

Every note carries an articulation the renderer maps onto the target
instrument (Odin III velocity bands / keyswitches, GM ignores it):
palm_mute, open, pinch_harmonic, dead, slide.

Styles (instrument `style`, overridable per section via `riff_style`):
  pedal_tone   — palm-muted open-string pedal on 8ths, scale notes above it
                 (never more than 2 high notes in a row)
  gallop       — 8th + two 16ths ("da-da-dum") on the pedal, power-chord accents
  chug         — breakdown: syncopated 16th/8th/quarter/half chugs, rests grow
                 with repetition, no lone 16ths, accents on power chords
  open_chords  — ringing power chords on chord changes, palm-muted 8ths between
  tremolo      — 16th-note tremolo picking over chord tones
  power_strum  — 8th-note power-chord strums, upbeats palm-muted (punk/pop-punk)
  clean_arp    — clean 8th-note arpeggio of the chord (intros, ambient breaks)

The pedal note is the chord root on the lowest string of profile.tuning
(drop C → C2), so the riff always sits on the open string the genre
expects.
"""

from dataclasses import dataclass
from typing import List, Optional, Tuple
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile
from .theory import (
    SCALES, chord_pitches, parse_key, midi_to_pitch_str, lowest_string, pedal_note,
    voice_chord,
)
from .profile import get_instrument, velocity_range, style_for
from .harmony import ChordSpan, MotifPlan, span_at
from .structure import leaf_sections, section_energy, section_density, is_muted, is_stop
from .chords import beats_to_duration


@dataclass
class Attack:
    start: float
    duration: float
    low: bool          # pedal / power-chord root — what the kick and bass lock to
    accent: bool


def generate_riff(
    profile: GenreProfile,
    sections: List[Section],
    timeline: List[ChordSpan],
    key: Optional[str] = None,
    rng: Optional[random.Random] = None,
    role: str = "rhythm_guitar",
) -> Tuple[Track, List[Attack]]:
    rng = rng or random.Random()
    inst = get_instrument(profile, role)
    if inst is None or not timeline:
        return Track(notes=[], name="Rhythm Guitar", role=role), []

    tonic_pc, _ = parse_key(key or profile.default_key)
    register = inst.register
    vmin, vmax = velocity_range(profile, role)
    low_string = lowest_string(profile.tuning) or register[0]
    pinch_rate = float(inst.articulations.get("pinch_harmonic_rate", 0.04))
    motif = MotifPlan(profile, role, rng)

    w = _Writer(register, vmin, vmax)
    for leaf in leaf_sections(sections):
        if is_muted(leaf, role):
            continue
        style = style_for(leaf, role, inst.style or "pedal_tone")
        energy = section_energy(profile, leaf)
        density = section_density(profile, leaf)
        if is_stop(leaf):
            span = span_at(timeline, leaf.start_beat())
            if span:
                root = pedal_note(_root_pc(span, tonic_pc), low_string)
                w.chord(_power(root), leaf.start_beat(), 1.5, 1.0, "open", accent=True)
            continue

        for bar in range(leaf.start_bar, leaf.end_bar + 1):
            bp = motif.bar(leaf, bar - leaf.start_bar)
            bar_start = (bar - 1) * 4
            span = span_at(timeline, bar_start)
            if span is None:
                continue
            root_pc = _root_pc(span, tonic_pc)
            if bp.last_root:
                degrees = SCALES.get(span.scale, SCALES["minor"])
                root_pc = (tonic_pc + bp.rng.choice(degrees[3:6])) % 12
            pedal = pedal_note(root_pc, low_string)
            scale_notes = _scale_above(pedal, tonic_pc, span.scale, register)
            ctx = _BarCtx(bar_start, pedal, scale_notes, span, tonic_pc, energy, density,
                          bp.rng, bp.octave_drop, pinch_rate, low_string)
            {
                "pedal_tone": _pedal_tone, "gallop": _gallop, "chug": _chug,
                "open_chords": _open_chords, "tremolo": _tremolo,
                "power_strum": _power_strum, "clean_arp": _clean_arp,
            }.get(style, _pedal_tone)(w, ctx)

    track = Track(name="Rhythm Guitar", instrument=inst.gm or "distortion-guitar",
                  notes=w.notes, role=role)
    return track, sorted(w.attacks, key=lambda a: a.start)


# -------- internals --------

@dataclass
class _BarCtx:
    start: float
    pedal: int
    scale: List[int]          # scale pitches above the pedal, inside the register
    span: ChordSpan
    tonic_pc: int
    energy: float
    density: float
    rng: random.Random
    octave_drop: bool
    pinch_rate: float
    low_string: int


class _Writer:
    def __init__(self, register, vmin, vmax):
        self.register, self.vmin, self.vmax = register, vmin, vmax
        self.notes: List[Note] = []
        self.attacks: List[Attack] = []

    def vel(self, rel: float, energy: float) -> float:
        v = self.vmin + (self.vmax - self.vmin) * rel
        return round(min(1.0, v * (0.88 + 0.12 * energy)), 3)

    def note(self, pitch: int, start: float, beats: float, rel: float, energy: float,
             articulation: str, low: bool = False, accent: bool = False) -> None:
        self.notes.append(Note(
            pitch=midi_to_pitch_str(pitch), duration=beats_to_duration(beats),
            velocity=self.vel(rel, energy), start=round(start, 4), articulation=articulation,
        ))
        if low or accent:
            self.attacks.append(Attack(round(start, 4), beats, low, accent))

    def chord(self, pitches: List[int], start: float, beats: float, energy: float,
              articulation: str, accent: bool = False, rel: float = 0.9) -> None:
        for i, p in enumerate(pitches):
            self.notes.append(Note(
                pitch=midi_to_pitch_str(p), duration=beats_to_duration(beats),
                velocity=self.vel(rel, energy), start=round(start, 4), articulation=articulation,
            ))
        self.attacks.append(Attack(round(start, 4), beats, True, accent))


def _root_pc(span: ChordSpan, tonic_pc: int) -> int:
    return chord_pitches(span.roman, tonic_pc, span.scale, base_octave=3)[0] % 12


def _power(root: int) -> List[int]:
    return [root, root + 7, root + 12]


def _scale_above(pedal: int, tonic_pc: int, scale: str, register) -> List[int]:
    intervals = SCALES.get(scale, SCALES["minor"])
    out = []
    for octave in range(0, 3):
        for i in intervals:
            p = (pedal // 12 + octave) * 12 + (tonic_pc + i) % 12
            while p <= pedal:
                p += 12
            if p <= register[1] and p not in out:
                out.append(p)
    return sorted(out)


def _high(ctx: _BarCtx) -> int:
    pool = [p for p in ctx.scale if p >= ctx.pedal + 7] or ctx.scale or [ctx.pedal + 12]
    p = ctx.rng.choice(pool[:8])
    if ctx.octave_drop and p - 12 > ctx.pedal:
        p -= 12
    return p


def _pinch(ctx: _BarCtx) -> bool:
    return ctx.rng.random() < ctx.pinch_rate * (0.5 + ctx.energy)


def _pedal_tone(w: _Writer, ctx: _BarCtx) -> None:
    highs_in_row = 0
    for slot in range(8):
        t = ctx.start + slot * 0.5
        if slot == 0:
            w.chord(_power(ctx.pedal), t, 0.5, ctx.energy, "open", accent=True)
            highs_in_row = 0
            continue
        if highs_in_row < 2 and ctx.rng.random() < 0.35:
            art = "pinch_harmonic" if _pinch(ctx) else "open"
            w.note(_high(ctx), t, 0.5, 0.85, ctx.energy, art)
            highs_in_row += 1
        else:
            w.note(ctx.pedal, t, 0.5, 0.75, ctx.energy, "palm_mute", low=True)
            highs_in_row = 0


def _gallop(w: _Writer, ctx: _BarCtx) -> None:
    reverse = ctx.rng.random() < 0.3
    for beat in range(4):
        t = ctx.start + beat
        if beat in (0, 2) and ctx.rng.random() < 0.6:
            w.chord(_power(ctx.pedal if beat == 0 else _high_root(ctx)), t, 0.5,
                    ctx.energy, "open", accent=True)
            offsets = [0.5, 0.75]
        else:
            offsets = [0.0, 0.25, 0.5] if reverse else [0.0, 0.5, 0.75]
        for off in offsets:
            w.note(ctx.pedal, t + off, 0.25 if off % 0.5 else 0.5, 0.75, ctx.energy,
                   "palm_mute", low=True)


def _high_root(ctx: _BarCtx) -> int:
    """A power-chord root a 4th/5th/b6 above the pedal, kept on the low strings."""
    candidates = [p for p in ctx.scale if ctx.pedal + 3 <= p <= ctx.pedal + 8]
    return ctx.rng.choice(candidates) if candidates else ctx.pedal


def _chug(w: _Writer, ctx: _BarCtx) -> None:
    pos, consecutive_roots, pending_16th = 0.0, 0, False
    weights = {0.25: 1, 0.5: 3, 1.0: 3 if ctx.density < 0.6 else 1.5, 2.0: 1 if ctx.density < 0.6 else 0.3}
    while pos < 4 - 1e-6:
        t = ctx.start + pos
        # Rests: likelier the longer the root has repeated (metalerator's rule)
        if not pending_16th and pos > 0 and consecutive_roots and \
                ctx.rng.random() < 0.06 * consecutive_roots * (1.2 - ctx.density):
            rest = 0.5 if ctx.rng.random() < 0.7 else 1.0
            pos += min(rest, 4 - pos)
            consecutive_roots = 0
            continue
        allowed = {d: wt for d, wt in weights.items() if d <= 4 - pos + 1e-6}
        dur = 0.25 if pending_16th else ctx.rng.choices(list(allowed), weights=list(allowed.values()))[0]
        pending_16th = (dur == 0.25 and not pending_16th)   # 16ths come in pairs
        if dur >= 1.0 and ctx.rng.random() < 0.5:
            if ctx.rng.random() < 0.25:
                # dissonant high accent (b2 / tritone over the pedal)
                w.note(ctx.pedal + ctx.rng.choice([13, 18, 19]), t, dur, 0.95, ctx.energy,
                       "pinch_harmonic" if _pinch(ctx) else "open", accent=True)
                w.note(ctx.pedal, t, dur, 0.9, ctx.energy, "open", low=True)
            else:
                w.chord(_power(ctx.pedal), t, dur, ctx.energy, "open", accent=True, rel=1.0)
            consecutive_roots = 0
        else:
            w.note(ctx.pedal, t, dur, 0.85, ctx.energy, "palm_mute", low=True, accent=(pos == 0))
            consecutive_roots += 1
        pos += dur


def _open_chords(w: _Writer, ctx: _BarCtx) -> None:
    span = ctx.span
    root = ctx.pedal if ctx.pedal >= ctx.low_string else ctx.pedal + 12
    chord = voice_chord(span.roman, ctx.tonic_pc, span.scale, (ctx.low_string, ctx.low_string + 28),
                        "power_chords", low_root=ctx.low_string)
    if ctx.density >= 0.85 and ctx.rng.random() < 0.5:
        # Driving chorus: quarter-note power chords
        for beat in range(4):
            w.chord(chord, ctx.start + beat, 1.0, ctx.energy, "open", accent=beat == 0)
    else:
        w.chord(chord, ctx.start, 2.0, ctx.energy, "open", accent=True)
        for off in (2.0, 2.5, 3.0, 3.5):
            if ctx.rng.random() < 0.75:
                w.note(root, ctx.start + off, 0.5, 0.7, ctx.energy, "palm_mute", low=True)


def _tremolo(w: _Writer, ctx: _BarCtx) -> None:
    tones = [p for p in voice_chord(ctx.span.roman, ctx.tonic_pc, ctx.span.scale,
                                    (ctx.pedal + 12, ctx.pedal + 31), "triads")]
    note = ctx.rng.choice(tones)
    for step in range(16):
        if step % 4 == 0 and ctx.rng.random() < 0.5:
            note = ctx.rng.choice(tones + [p for p in ctx.scale if p > ctx.pedal + 12][:4])
        w.note(note, ctx.start + step * 0.25, 0.25, 0.7 + 0.2 * (step % 4 == 0), ctx.energy,
               "open", low=(step % 4 == 0))


def _power_strum(w: _Writer, ctx: _BarCtx) -> None:
    chord = voice_chord(ctx.span.roman, ctx.tonic_pc, ctx.span.scale,
                        (ctx.low_string, ctx.low_string + 28), "power_chords",
                        low_root=ctx.low_string)
    for slot in range(8):
        down = slot % 2 == 0
        w.chord(chord, ctx.start + slot * 0.5, 0.5, ctx.energy,
                "open" if down else "palm_mute", accent=slot in (0, 4), rel=0.9 if down else 0.75)


def _clean_arp(w: _Writer, ctx: _BarCtx) -> None:
    tones = voice_chord(ctx.span.roman, ctx.tonic_pc, ctx.span.scale,
                        (ctx.pedal + 12, ctx.pedal + 36), "open_voicings")
    order = tones + tones[-2:0:-1]
    for slot in range(8):
        w.note(order[slot % len(order)], ctx.start + slot * 0.5, 1.0, 0.45, ctx.energy,
               "open", low=(slot == 0))
