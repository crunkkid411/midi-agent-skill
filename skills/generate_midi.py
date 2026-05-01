#!/usr/bin/env python3
"""
Generate MIDI file from a Composition object using midiutil.
Uses General MIDI program changes compatible with A320U.sf2 SoundFont.

Drum tracks (track.is_drum=True) route to MIDI channel 9 (GM drums); their
note pitches may be GM drum numbers (e.g. "36" for kick) or pitch strings
("C2" → 36) — both work.
"""

import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from midiutil import MIDIFile
from midi_types.music import Composition, Track, Note
from midi_types.gm_instruments import resolve_instrument


DURATION_MAP = {
    "1": 4.0, "2": 2.0, "d2": 3.0,
    "4": 1.0, "d4": 1.5,
    "8": 0.5, "d8": 0.75,
    "16": 0.25, "32": 0.125,
    "T4": 1/3, "T8": 1/6,
}

DEFAULT_VELOCITY = 100  # 0-127 MIDI


def parse_pitch(pitch_str: str) -> int:
    """Convert pitch string ("C4", "F#5", "Bb3") or numeric string to MIDI 0-127."""
    s = str(pitch_str).strip()
    # Numeric drum-note shortcut: "36" → 36
    if s.lstrip("-").isdigit():
        return max(0, min(127, int(s)))

    note_map = {'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11}
    match = re.match(r'^([A-Ga-g])([#b]?)(-?\d+)$', s)
    if not match:
        raise ValueError(f"Invalid pitch format: {pitch_str}")

    note, accidental, octave = match.groups()
    midi_note = (int(octave) + 1) * 12 + note_map[note.upper()]
    if accidental == '#':
        midi_note += 1
    elif accidental == 'b':
        midi_note -= 1
    return max(0, min(127, midi_note))


def parse_duration(duration_str: str) -> float:
    if duration_str in DURATION_MAP:
        return DURATION_MAP[duration_str]
    try:
        division = float(duration_str)
        if division > 0:
            return 4.0 / division
    except ValueError:
        pass
    return 1.0


def _resolve_velocity(v: float | None) -> int:
    if v is None:
        return DEFAULT_VELOCITY
    # Accept 0.0-1.0 or 0-127
    if v <= 1.0:
        return max(1, min(127, int(round(v * 127))))
    return max(1, min(127, int(round(v))))


def _assign_channel(track_idx: int, is_drum: bool) -> int:
    if is_drum:
        return 9
    # Skip channel 9 (drums) for melodic tracks
    if track_idx < 9:
        return track_idx
    return min(15, track_idx + 1)


def generate_midi(composition: Composition) -> str:
    """Generate a MIDI file from a Composition. Returns the output path."""
    num_tracks = len(composition.tracks)
    if num_tracks == 0:
        raise ValueError("Composition has no tracks")

    midi = MIDIFile(num_tracks)
    midi.addTempo(0, 0, composition.bpm)

    for track_idx, track in enumerate(composition.tracks):
        channel = _assign_channel(track_idx, track.is_drum)
        track_name = track.name or track.instrument or f"Track {track_idx + 1}"
        midi.addTrackName(track_idx, 0, track_name)

        # Drums use channel 9; no program change needed (GM drum kit is implicit).
        if not track.is_drum:
            program = resolve_instrument(track.instrument) if track.instrument else 0
            midi.addProgramChange(track_idx, channel, 0, program)

        cursor = 0.0
        for note in track.notes:
            try:
                pitch = parse_pitch(note.pitch)
                duration = parse_duration(note.duration)
                velocity = _resolve_velocity(note.velocity)
                time = note.start if note.start is not None else cursor

                midi.addNote(
                    track=track_idx,
                    channel=channel,
                    pitch=pitch,
                    time=time,
                    duration=duration,
                    volume=velocity,
                )

                # Sequential cursor only advances when start was implicit
                if note.start is None:
                    cursor += duration
            except Exception as e:
                print(f"Warning: skipping note {note.pitch} {note.duration}: {e}", file=sys.stderr)

    out_dir = Path(__file__).parent.parent / "output"
    out_dir.mkdir(parents=True, exist_ok=True)
    safe_title = re.sub(r'[^a-zA-Z0-9_-]', '_', composition.title)
    file_path = out_dir / f"{safe_title}.mid"

    with open(file_path, 'wb') as f:
        midi.writeFile(f)

    return str(file_path)


def generate_midi_from_dict(data: dict) -> str:
    return generate_midi(Composition.from_dict(data))


if __name__ == "__main__":
    example = {
        "title": "Test Multi-Instrument",
        "bpm": 120,
        "tracks": [
            {
                "instrument": "acoustic-grand-piano",
                "notes": [
                    {"pitch": "C4", "duration": "4"},
                    {"pitch": "E4", "duration": "4"},
                    {"pitch": "G4", "duration": "4"},
                    {"pitch": "C5", "duration": "2"},
                ],
            },
            {
                "instrument": "acoustic-bass",
                "notes": [
                    {"pitch": "C2", "duration": "2", "velocity": 0.7},
                    {"pitch": "G2", "duration": "2", "velocity": 0.5},
                ],
            },
        ],
    }
    print(f"Generated: {generate_midi_from_dict(example)}")
