"""
Melody/lead track composer.

Reads the harmony timeline, the drum track and (optionally) the rhythm
guitar to compose a line that:
  - lands chord tones on strong beats, scale tones in between
  - backs off where the drums are busy (fills, crashes, snare+kick stacks)
  - repeats as a motif (profile.motif) instead of re-rolling every bar
  - respects register, velocity range and section mute/density

Styles (instrument `style`, overridable per section via `melody_style`):
  pentatonic_sparse / melodic_line — sparse 8th-note line (default)
  fast_arpeggio / arpeggio         — 16th-note chord arpeggios
  stab                             — top-note stabs
  sustained_hook                   — long chord-tone notes (sung-chorus style)
  harmonized_thirds                — melodic line + diatonic 3rd above (twin leads)
  riff_octave                      — the rhythm guitar's non-pedal notes, an
                                     octave up (metalcore lead doubling)
  tremolo_lead                     — 16th tremolo on a slow melody
"""

from typing import Dict, List, Optional
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile
from .theory import chord_pitches, parse_key, midi_in_register, midi_to_pitch_str, SCALES
from .profile import get_instrument, velocity_range, style_for
from .harmony import ChordSpan, MotifPlan, span_at
from .structure import section_density, leaf_sections, is_muted, is_stop
from .chords import beats_to_duration


def generate_melody(
    profile: GenreProfile,
    sections: List[Section],
    timeline: List[ChordSpan],
    drum_track: Optional[Track] = None,
    key: Optional[str] = None,
    role: str = "melody",
    rng: Optional[random.Random] = None,
    riff_track: Optional[Track] = None,
) -> Track:
    rng = rng or random.Random()
    inst = get_instrument(profile, role)
    if inst is None:
        # Fall back to "lead" if "melody" isn't defined, and vice versa
        inst = get_instrument(profile, "lead" if role == "melody" else "melody")
    if inst is None or not timeline:
        return Track(notes=[], name=role.title(), role=role)

    tonic_pc, _ = parse_key(key or profile.default_key)
    register = inst.register
    vmin, vmax = velocity_range(profile, role)
    busy = _drum_busyness(drum_track)
    motif = MotifPlan(profile, role, rng)
    notes: List[Note] = []

    for leaf in leaf_sections(sections):
        density = section_density(profile, leaf)
        # Pull the melody out of low-density sections, stops and muted sections
        if density < 0.5 or is_muted(leaf, role) or is_stop(leaf):
            continue
        style = (style_for(leaf, role, inst.style or "") or "").lower()
        if "pentatonic" in style and "minor_pentatonic" in profile.scales:
            line_scale = "minor_pentatonic"
        elif "pentatonic" in style and "major_pentatonic" in profile.scales:
            line_scale = "major_pentatonic"
        else:
            line_scale = None

        last = (register[0] + register[1]) // 2
        for bar in range(leaf.start_bar, leaf.end_bar + 1):
            bp = motif.bar(leaf, bar - leaf.start_bar)
            start = (bar - 1) * 4
            span = span_at(timeline, start)
            if span is None:
                continue
            tones = chord_pitches(span.roman, tonic_pc, span.scale, base_octave=4)
            scale = line_scale or span.scale
            r = bp.rng
            before = len(notes)
            if "riff_octave" in style and riff_track is not None:
                _riff_octave(riff_track, start, register, vmin, vmax, r, notes)
            elif "arpeggio" in style:
                _emit_arpeggio(tones, start, 4, register, vmin, vmax, r, notes)
            elif "stab" in style:
                _emit_stab_lead(tones, start, 4, register, vmin, vmax, r, notes)
            elif "sustained_hook" in style:
                last = _emit_hook(tones, start, register, vmin, vmax, r, notes, last)
            elif "tremolo" in style:
                last = _emit_tremolo(tones, start, register, vmin, vmax, r, notes, last)
            else:
                last = _emit_melodic_line(tones, tonic_pc, scale, start, 4, register,
                                          vmin, vmax, density, r, notes, last, busy)
                if "harmonized_thirds" in style:
                    _add_thirds(notes[before:], tonic_pc, span.scale, register, notes)
            if bp.octave_drop:
                for n in notes[before:]:
                    p = _p(n.pitch) - 12
                    if p >= register[0]:
                        n.pitch = midi_to_pitch_str(p)

    return Track(name=role.title(), instrument=inst.gm or "lead-1-square", notes=notes, role=role)


def _p(pitch: str) -> int:
    from skills.generate_midi import parse_pitch
    return parse_pitch(pitch)


def _drum_busyness(drum_track: Optional[Track]) -> Dict[float, int]:
    """Count of drum hits per 8th-note slot (crashes/toms/snares weigh double)."""
    busy: Dict[float, int] = {}
    if not drum_track:
        return busy
    for n in drum_track.notes:
        if n.start is None:
            continue
        slot = round(n.start * 2) / 2
        weight = 2 if (n.drum or "") in ("crash", "china", "snare", "tom_high", "tom_mid",
                                         "tom_low", "floor_tom") else 1
        busy[slot] = busy.get(slot, 0) + weight
    return busy


def _emit_arpeggio(chord_tones, start_beat, dur_beats, register, vmin, vmax, rng, notes):
    pattern = [midi_in_register(p, register) for p in chord_tones]
    if len(pattern) < 4:
        pattern = pattern + [midi_in_register(p + 12, register) for p in pattern]
    step = 0.25  # 16ths
    for s in range(int(dur_beats / step)):
        notes.append(Note(
            pitch=midi_to_pitch_str(pattern[s % len(pattern)]), duration="16",
            velocity=round(rng.uniform(vmin, vmax), 3), start=round(start_beat + s * step, 4),
        ))


def _emit_stab_lead(chord_tones, start_beat, dur_beats, register, vmin, vmax, rng, notes):
    """Emit short stabs on downbeats and one syncopation."""
    top = midi_in_register(chord_tones[-1], register)
    for r in [0, 1.5, 2, 3.5]:
        if r >= dur_beats:
            break
        notes.append(Note(
            pitch=midi_to_pitch_str(top), duration="16",
            velocity=round(rng.uniform(vmin, vmax), 3), start=round(start_beat + r, 4),
        ))


def _emit_hook(chord_tones, start, register, vmin, vmax, rng, notes, last):
    choices = sorted({midi_in_register(p, register) for p in chord_tones})
    rhythm = rng.choice([[(0, 2), (2, 2)], [(0, 3), (3, 1)], [(0, 1.5), (1.5, 2.5)], [(0, 4)]])
    for off, dur in rhythm:
        pitch = min(choices, key=lambda p: abs(p - last) + rng.random() * 3)
        notes.append(Note(pitch=midi_to_pitch_str(pitch), duration=beats_to_duration(dur),
                          velocity=round(rng.uniform(vmin, vmax), 3), start=round(start + off, 4)))
        last = pitch
    return last


def _emit_tremolo(chord_tones, start, register, vmin, vmax, rng, notes, last):
    choices = sorted({midi_in_register(p, register) for p in chord_tones})
    for half in range(2):
        pitch = min(choices, key=lambda p: abs(p - last) + rng.random() * 4)
        for s in range(8):
            notes.append(Note(pitch=midi_to_pitch_str(pitch), duration="16",
                              velocity=round(rng.uniform(vmin, vmax) * (1.0 if s % 4 == 0 else 0.85), 3),
                              start=round(start + half * 2 + s * 0.25, 4), articulation="open"))
        last = pitch
    return last


def _emit_melodic_line(chord_tones, tonic_pc, scale_name, start_beat, dur_beats,
                       register, vmin, vmax, density, rng, notes, last_pitch, busy):
    """Chord tones on strong beats, scale-step passing tones on weak beats,
    rests where density is moderate or the drums are busy."""
    intervals = SCALES.get(scale_name, SCALES["minor_pentatonic"])
    base = (4 + 1) * 12 + tonic_pc
    scaled = sorted({midi_in_register(base + i + o, register) for i in intervals for o in (-12, 0, 12)})

    for slot in range(int(dur_beats * 2)):
        beat_pos = slot / 2.0
        t = start_beat + beat_pos
        is_downbeat = (slot % 8 == 0)
        is_strong = (slot % 4 == 0)
        play_prob = 0.85 if is_downbeat else 0.55 if is_strong else 0.35 * density
        if busy.get(round(t * 2) / 2, 0) >= 4:
            play_prob *= 0.35
        if rng.random() > play_prob:
            continue

        if is_strong and chord_tones:
            choices = [midi_in_register(p, register) for p in chord_tones]
            pitch = min(choices, key=lambda p: abs(p - last_pitch))
        else:
            direction = rng.choice([-1, 1])
            idx = min(range(len(scaled)), key=lambda i: abs(scaled[i] - last_pitch))
            pitch = scaled[max(0, min(len(scaled) - 1, idx + direction))]

        dur = "8" if rng.random() > 0.25 else "4"
        notes.append(Note(
            pitch=midi_to_pitch_str(pitch), duration=dur,
            velocity=round(rng.uniform(vmin, vmax), 3), start=round(t, 4),
        ))
        last_pitch = pitch
    return last_pitch


def _add_thirds(line: List[Note], tonic_pc: int, scale_name: str, register, out: List[Note]):
    """Twin-lead harmony: a diatonic 3rd above every note of the line."""
    intervals = SCALES.get(scale_name, SCALES["minor"])
    pcs = [(tonic_pc + i) % 12 for i in intervals]
    for n in list(line):
        p = _p(n.pitch)
        if p % 12 not in pcs:
            continue
        idx = pcs.index(p % 12)
        third_pc = pcs[(idx + 2) % len(pcs)]
        up = (third_pc - p) % 12 or 12
        if p + up <= register[1] + 4:
            out.append(Note(pitch=midi_to_pitch_str(p + up), duration=n.duration,
                            velocity=round((n.velocity or 0.7) * 0.92, 3), start=n.start))


def _riff_octave(riff: Track, bar_start: float, register, vmin, vmax, rng, out):
    """Lead doubles the riff's melodic (non-palm-muted) notes an octave or two up.
    Only the top note of each riff onset is taken, so power chords become a line."""
    tops: Dict[float, Note] = {}
    for n in riff.notes:
        if n.start is None or not (bar_start <= n.start < bar_start + 4):
            continue
        if n.articulation == "palm_mute":
            continue
        if n.start not in tops or _p(n.pitch) > _p(tops[n.start].pitch):
            tops[n.start] = n
    for t in sorted(tops):
        n = tops[t]
        p = midi_in_register(_p(n.pitch) + 12, register)
        out.append(Note(pitch=midi_to_pitch_str(p), duration=n.duration,
                        velocity=round(rng.uniform(vmin, vmax), 3), start=n.start))
