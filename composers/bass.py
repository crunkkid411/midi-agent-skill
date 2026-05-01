"""
Bass track composer.

Reads the chord plan + drum track to lock the bass to chord roots and the
kick pattern. Style is informed by the profile's bass instrument:
  - sustained_root  → root note held for chord duration
  - walking         → root + scale-tone walk on every beat
  - sub_808_distorted → root on every kick hit (locked to drums)
  - arpeggio        → root-fifth-octave-fifth pattern
"""

from typing import List, Optional, Tuple
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile, ChordProgression
from .theory import (
    chord_root, chord_pitches, parse_key, midi_in_register, midi_to_pitch_str,
    SCALES,
)
from .profile import get_instrument, velocity_range


def generate_bass(
    profile: GenreProfile,
    sections: List[Section],
    chord_plan: List[Tuple[Section, ChordProgression]],
    drum_track: Optional[Track] = None,
    key: Optional[str] = None,
    rng: Optional[random.Random] = None,
) -> Track:
    rng = rng or random.Random()
    inst = get_instrument(profile, "bass")
    if inst is None:
        return Track(notes=[], name="Bass")

    tonic_pc, scale_name = parse_key(key or profile.default_key)
    register = inst.register
    style = (inst.style or "sustained_root").lower()
    vmin, vmax = velocity_range(profile, "bass")

    # Index drum hits by bar for "lock to kick" styles
    kick_times_by_bar: dict[int, List[float]] = {}
    if drum_track and "808" in style or "kick" in style:
        # Capture kick hits (GM 36)
        for n in drum_track.notes if drum_track else []:
            if str(n.pitch) == "36" and n.start is not None:
                bar = int(n.start // 4) + 1
                kick_times_by_bar.setdefault(bar, []).append(n.start)

    notes: List[Note] = []

    for section, prog in chord_plan:
        bars_per_chord = max(0.25, prog.bars_per_chord)
        chord_idx = 0
        bar_cursor = section.start_bar
        while bar_cursor <= section.end_bar:
            roman = prog.roman[chord_idx % len(prog.roman)]
            chord_idx += 1
            chord_bars = min(bars_per_chord, section.end_bar - bar_cursor + 1)
            beats_per_bar = 4

            chord_tones = chord_pitches(roman, tonic_pc, scale_name, base_octave=2)
            root = chord_tones[0]
            # Place root in the bass register
            root_in_reg = midi_in_register(root, register)
            fifth = midi_in_register(chord_tones[2] if len(chord_tones) > 2 else root + 7, register)

            chord_start_beat = (bar_cursor - 1) * beats_per_bar
            chord_dur_beats = chord_bars * beats_per_bar

            if style in ("sustained_root", "sustained"):
                notes.append(Note(
                    pitch=midi_to_pitch_str(root_in_reg),
                    duration=_beats_to_duration(chord_dur_beats),
                    velocity=round(rng.uniform(vmin, vmax), 3),
                    start=round(chord_start_beat, 4),
                ))
            elif style == "walking":
                # Quarter notes: root, scale tone, fifth, scale tone
                scale_intervals = SCALES.get(scale_name, SCALES["minor"])
                for beat in range(int(chord_dur_beats)):
                    if beat == 0:
                        p = root_in_reg
                    elif beat == 2:
                        p = fifth
                    else:
                        # Pick a random scale tone within a fourth of root
                        offset = rng.choice(scale_intervals[:4])
                        p = midi_in_register(root_in_reg + offset, register)
                    notes.append(Note(
                        pitch=midi_to_pitch_str(p), duration="4",
                        velocity=round(rng.uniform(vmin, vmax), 3),
                        start=round(chord_start_beat + beat, 4),
                    ))
            elif "808" in style or "sub" in style:
                # Lock to kick — emit a bass note at every kick time within this chord
                bars_covered = list(range(bar_cursor, bar_cursor + int(chord_bars) + 1))
                hits = [t for b in bars_covered for t in kick_times_by_bar.get(b, [])
                        if chord_start_beat <= t < chord_start_beat + chord_dur_beats]
                if not hits:
                    # Fallback: hit on each beat
                    hits = [chord_start_beat + b for b in range(int(chord_dur_beats))]
                for i, t in enumerate(hits):
                    # Alternate root and fifth for movement
                    p = root_in_reg if i % 4 != 2 else fifth
                    notes.append(Note(
                        pitch=midi_to_pitch_str(p), duration="8",
                        velocity=round(rng.uniform(vmin, vmax), 3),
                        start=round(t, 4),
                    ))
            elif style == "arpeggio":
                pattern = [root_in_reg, fifth, midi_in_register(root_in_reg + 12, register), fifth]
                step = 0.5  # 8ths
                steps = int(chord_dur_beats / step)
                for s in range(steps):
                    p = pattern[s % 4]
                    notes.append(Note(
                        pitch=midi_to_pitch_str(p), duration="8",
                        velocity=round(rng.uniform(vmin, vmax), 3),
                        start=round(chord_start_beat + s * step, 4),
                    ))
            else:
                # Default: half-note root + fifth
                notes.append(Note(
                    pitch=midi_to_pitch_str(root_in_reg), duration="2",
                    velocity=round(rng.uniform(vmin, vmax), 3),
                    start=round(chord_start_beat, 4),
                ))
                if chord_dur_beats >= 4:
                    notes.append(Note(
                        pitch=midi_to_pitch_str(fifth), duration="2",
                        velocity=round(rng.uniform(vmin, vmax), 3),
                        start=round(chord_start_beat + 2, 4),
                    ))

            bar_cursor += int(chord_bars) if chord_bars >= 1 else 1

    return Track(
        name="Bass",
        instrument=inst.gm or "fingered-bass",
        notes=notes,
    )


def _beats_to_duration(beats: float) -> str:
    candidates = [
        (4.0, "1"), (3.0, "d2"), (2.0, "2"),
        (1.5, "d4"), (1.0, "4"), (0.75, "d8"),
        (0.5, "8"), (0.25, "16"),
    ]
    for b, tok in candidates:
        if beats >= b - 0.01:
            return tok
    return "16"
