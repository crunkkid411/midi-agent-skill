"""
Chord track composer.

Reads a profile + section plan, picks a chord progression, voices each chord
in the chord-instrument's register, emits a Track of held chord stabs.
"""

from typing import List, Tuple, Optional
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile, ChordProgression
from .theory import chord_pitches, parse_key, midi_in_register, midi_to_pitch_str
from .profile import pick_weighted, get_instrument, velocity_range


def generate_chords(
    profile: GenreProfile,
    sections: List[Section],
    key: Optional[str] = None,
    rng: Optional[random.Random] = None,
) -> Tuple[Track, List[Tuple[Section, ChordProgression]]]:
    """Build the chord track. Returns (track, [(section, progression)]) so
    downstream composers (bass, melody) can reference what was played where."""
    rng = rng or random.Random()
    inst = get_instrument(profile, "chords")
    if inst is None:
        return Track(notes=[], name="Chords"), []

    tonic_pc, scale_name = parse_key(key or profile.default_key)
    register = inst.register
    vmin, vmax = velocity_range(profile, "chords")
    style = inst.style or "sustained"

    notes: List[Note] = []
    plan: List[Tuple[Section, ChordProgression]] = []

    # Voicing octave: aim near the middle of the chord register
    base_octave = max(2, min(5, ((register[0] + register[1]) // 2) // 12 - 1))

    # Only consider top-level sections for chord planning (skip micro sub-sections,
    # they share the parent's progression)
    top_sections = [s for s in sections if "." not in s.name]

    for section in top_sections:
        if not profile.chord_progressions:
            continue
        prog = pick_weighted(profile.chord_progressions, rng=rng)
        plan.append((section, prog))

        bars_per_chord = max(0.25, prog.bars_per_chord)
        # Repeat the progression to fill the section
        chord_idx = 0
        bar_cursor = section.start_bar
        while bar_cursor <= section.end_bar:
            roman = prog.roman[chord_idx % len(prog.roman)]
            chord_idx += 1
            pitches = chord_pitches(roman, tonic_pc, scale_name, base_octave=base_octave)
            voiced = sorted(set(midi_in_register(p, register) for p in pitches))
            chord_bars = min(bars_per_chord, section.end_bar - bar_cursor + 1)

            # Emit each pitch as a note at the same start time = chord stab/hold
            beats_per_bar = 4  # assume 4/4 for now
            start_beat = (bar_cursor - 1) * beats_per_bar
            duration_beats = chord_bars * beats_per_bar
            duration_str = _beats_to_duration(duration_beats)

            # Velocity per chord (with small variation)
            vel = rng.uniform(vmin, vmax)

            if style == "stab":
                # Short percussive hits at start of each beat group
                stab_dur = "16"
                for offset in range(int(chord_bars * beats_per_bar * 2)):  # 8th-note stabs
                    if offset % 4 == 0 or rng.random() < 0.4:  # downbeats + occasional offbeat
                        for p in voiced:
                            notes.append(Note(
                                pitch=midi_to_pitch_str(p), duration=stab_dur,
                                velocity=round(vel * rng.uniform(0.85, 1.0), 3),
                                start=round(start_beat + offset * 0.5, 4),
                            ))
            else:
                # Sustained chord
                for p in voiced:
                    notes.append(Note(
                        pitch=midi_to_pitch_str(p), duration=duration_str,
                        velocity=round(vel, 3), start=round(start_beat, 4),
                    ))

            bar_cursor += int(chord_bars) if chord_bars >= 1 else 1

    return Track(
        name="Chords",
        instrument=inst.gm or "electric-piano-1",
        notes=notes,
    ), plan


def _beats_to_duration(beats: float) -> str:
    """Map beat counts to closest duration token."""
    candidates = [
        (4.0, "1"), (3.0, "d2"), (2.0, "2"),
        (1.5, "d4"), (1.0, "4"), (0.75, "d8"),
        (0.5, "8"), (0.25, "16"), (0.125, "32"),
    ]
    for b, tok in candidates:
        if beats >= b - 0.01:
            return tok
    return "16"
