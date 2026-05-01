"""
Melody/lead track composer.

Reads chord plan + drum track to compose a lead line that:
  - Lands chord tones on strong beats (downbeat + beat 3)
  - Uses scale tones for passing notes
  - Rests over busy drum moments (high-density sections still get space)
  - Respects the lead instrument's register and velocity range

Style-aware: 'fast_arpeggio' produces 16th-note runs, 'pentatonic_sparse'
produces wide-spaced melodic lines.
"""

from typing import List, Optional, Tuple
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile, ChordProgression
from .theory import (
    chord_pitches, parse_key, midi_in_register, midi_to_pitch_str,
    SCALES,
)
from .profile import get_instrument, velocity_range
from .structure import section_density


def generate_melody(
    profile: GenreProfile,
    sections: List[Section],
    chord_plan: List[Tuple[Section, ChordProgression]],
    drum_track: Optional[Track] = None,
    key: Optional[str] = None,
    role: str = "melody",
    rng: Optional[random.Random] = None,
) -> Track:
    rng = rng or random.Random()
    inst = get_instrument(profile, role)
    if inst is None:
        # Fall back to "lead" if "melody" isn't defined, and vice versa
        inst = get_instrument(profile, "lead" if role == "melody" else "melody")
    if inst is None:
        return Track(notes=[], name=role.title())

    tonic_pc, scale_name = parse_key(key or profile.default_key)
    register = inst.register
    style = (inst.style or "").lower()
    vmin, vmax = velocity_range(profile, role)

    # Use minor pentatonic if available for sparse melodies, else the song scale
    if "pentatonic" in style and "minor_pentatonic" in profile.scales:
        melodic_scale = "minor_pentatonic"
    elif "pentatonic" in style and "major_pentatonic" in profile.scales:
        melodic_scale = "major_pentatonic"
    else:
        melodic_scale = scale_name

    notes: List[Note] = []

    for section, prog in chord_plan:
        density = section_density(profile, section)
        # Pull the section out of melody during very low density sections
        if density < 0.5:
            continue

        bars_per_chord = max(0.25, prog.bars_per_chord)
        chord_idx = 0
        bar_cursor = section.start_bar
        while bar_cursor <= section.end_bar:
            roman = prog.roman[chord_idx % len(prog.roman)]
            chord_idx += 1
            chord_bars = min(bars_per_chord, section.end_bar - bar_cursor + 1)
            chord_start_beat = (bar_cursor - 1) * 4
            chord_tones = chord_pitches(roman, tonic_pc, scale_name, base_octave=4)

            if "fast_arpeggio" in style or "arpeggio" in style:
                _emit_arpeggio(chord_tones, chord_start_beat, chord_bars * 4,
                               register, vmin, vmax, rng, notes)
            elif "stab" in style:
                _emit_stab_lead(chord_tones, chord_start_beat, chord_bars * 4,
                                register, vmin, vmax, rng, notes)
            else:
                _emit_melodic_line(chord_tones, tonic_pc, melodic_scale,
                                   chord_start_beat, chord_bars * 4,
                                   register, vmin, vmax, density, rng, notes)

            bar_cursor += int(chord_bars) if chord_bars >= 1 else 1

    return Track(
        name=role.title(),
        instrument=inst.gm or "lead-1-square",
        notes=notes,
    )


def _emit_arpeggio(chord_tones, start_beat, dur_beats, register, vmin, vmax, rng, notes):
    pattern = [midi_in_register(p, register) for p in chord_tones]
    if len(pattern) < 4:
        # Add octave doubling
        pattern = pattern + [midi_in_register(p + 12, register) for p in pattern]
    step = 0.25  # 16ths
    steps = int(dur_beats / step)
    for s in range(steps):
        p = pattern[s % len(pattern)]
        notes.append(Note(
            pitch=midi_to_pitch_str(p), duration="16",
            velocity=round(rng.uniform(vmin, vmax), 3),
            start=round(start_beat + s * step, 4),
        ))


def _emit_stab_lead(chord_tones, start_beat, dur_beats, register, vmin, vmax, rng, notes):
    """Emit short stabs on downbeats and one syncopation."""
    top = midi_in_register(chord_tones[-1], register)
    rhythm = [0, 1.5, 2, 3.5]
    for r in rhythm:
        if r >= dur_beats:
            break
        notes.append(Note(
            pitch=midi_to_pitch_str(top), duration="16",
            velocity=round(rng.uniform(vmin, vmax), 3),
            start=round(start_beat + r, 4),
        ))


def _emit_melodic_line(chord_tones, tonic_pc, scale_name, start_beat, dur_beats,
                       register, vmin, vmax, density, rng, notes):
    """Sparse pentatonic-style line: chord tones on strong beats, occasional
    passing tones on weak beats, rests when density is moderate."""
    intervals = SCALES.get(scale_name, SCALES["minor_pentatonic"])
    base = (4 + 1) * 12 + tonic_pc  # octave 4
    scale_pitches = [base + i for i in intervals]

    # Pick the chord tone closest to register midpoint as melodic anchor
    mid = (register[0] + register[1]) / 2
    anchor = min((midi_in_register(p, register) for p in chord_tones), key=lambda p: abs(p - mid))

    # Rhythmic grid: 8 eighth-note slots per bar
    slots_per_bar = 8
    total_slots = int(dur_beats * 2)
    # Probability of playing per slot, modulated by density and beat strength
    last_pitch = anchor
    for slot in range(total_slots):
        beat_pos = slot / 2.0
        is_downbeat = (slot % 8 == 0)
        is_strong = (slot % 4 == 0)
        play_prob = 0.85 if is_downbeat else 0.55 if is_strong else 0.35 * density
        if rng.random() > play_prob:
            continue

        if is_strong and chord_tones:
            # Use a chord tone, prefer near last_pitch
            choices = [midi_in_register(p, register) for p in chord_tones]
            pitch = min(choices, key=lambda p: abs(p - last_pitch))
        else:
            # Scale-tone passing: step up or down from last_pitch
            direction = rng.choice([-1, 1])
            # Find closest scale pitch in register
            scaled = [midi_in_register(p, register) for p in scale_pitches]
            scaled_sorted = sorted(set(scaled))
            closest_idx = min(range(len(scaled_sorted)), key=lambda i: abs(scaled_sorted[i] - last_pitch))
            new_idx = max(0, min(len(scaled_sorted) - 1, closest_idx + direction))
            pitch = scaled_sorted[new_idx]

        # Duration: 8th by default, occasional quarter
        dur = "8" if rng.random() > 0.25 else "4"
        notes.append(Note(
            pitch=midi_to_pitch_str(pitch), duration=dur,
            velocity=round(rng.uniform(vmin, vmax), 3),
            start=round(start_beat + beat_pos, 4),
        ))
        last_pitch = pitch
