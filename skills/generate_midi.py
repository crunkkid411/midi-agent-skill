#!/usr/bin/env python3
"""
Generate MIDI file from a Composition object using midiutil.
Uses General MIDI program changes compatible with A320U.sf2 SoundFont.

Drum tracks (track.is_drum=True) route to MIDI channel 9 (GM drums); their
note pitches may be GM drum numbers (e.g. "36" for kick) or pitch strings
("C2" → 36) — both work.

Tracks with `vst` set render through instruments/<vst>.json:
  - drums: Note.drum lane → the kit's note number (Superior Drummer 3, GM, ...)
  - melodic: Note.articulation → keyswitch notes (held across the note for
    non-latching instruments such as Odin III) and/or a velocity band
    (Odin III Swift Mode); pitches are folded into the instrument's range.
No program change is sent to VST tracks. `split_tracks=True` also writes one
.mid per track (output/<title>/NN-<track>.mid) for dragging into a DAW
template, one instrument per track.
"""

import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from midiutil import MIDIFile
from midi_types.music import Composition, Track, Note
from midi_types.gm_instruments import resolve_instrument
from midi_types.instrument_map import load_instrument_map, InstrumentMap


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


def generate_midi(composition: Composition, split_tracks: bool = False,
                  out_dir: Path | None = None) -> str:
    """Generate a MIDI file from a Composition. Returns the combined file's path.
    With split_tracks, also writes one file per track next to it."""
    if not composition.tracks:
        raise ValueError("Composition has no tracks")

    out_dir = Path(out_dir) if out_dir else Path(__file__).parent.parent / "output"
    out_dir.mkdir(parents=True, exist_ok=True)
    safe_title = _safe(composition.title)
    file_path = out_dir / f"{safe_title}.mid"
    warned: set = set()
    _write(composition, composition.tracks, file_path, warned=warned)

    if split_tracks:
        track_dir = out_dir / safe_title
        track_dir.mkdir(parents=True, exist_ok=True)
        for i, track in enumerate(composition.tracks, 1):
            name = track.name or track.role or f"track{i}"
            _write(composition, [track], track_dir / f"{i:02d}-{_safe(name)}.mid",
                   channel_base=i - 1, warned=warned)

    return str(file_path)


def _safe(s: str) -> str:
    return re.sub(r'[^a-zA-Z0-9_-]', '_', s)


def _write(composition: Composition, tracks, path: Path, channel_base: int = 0,
           warned: set | None = None) -> None:
    midi = MIDIFile(len(tracks))
    midi.addTempo(0, 0, composition.bpm)
    warned = warned if warned is not None else set()
    for track_idx, track in enumerate(tracks):
        channel = _assign_channel(track_idx + channel_base, track.is_drum)
        midi.addTrackName(track_idx, 0, track.name or track.instrument or f"Track {track_idx + 1}")
        imap = _map_for(track, warned)

        # Drums use channel 9; no program change needed (GM drum kit is implicit).
        if not track.is_drum and imap is None:
            program = resolve_instrument(track.instrument) if track.instrument else 0
            midi.addProgramChange(track_idx, channel, 0, program)
        if track.pan is not None:
            midi.addControllerEvent(track_idx, channel, 0, 10, int(round(64 + 63 * track.pan)))

        keyswitch_spans = []
        cursor = 0.0
        for note in track.notes:
            try:
                pitch = parse_pitch(note.pitch)
                duration = parse_duration(note.duration)
                velocity = _resolve_velocity(note.velocity)
                time = note.start if note.start is not None else cursor

                if imap is not None:
                    if track.is_drum:
                        pitch = imap.drum_note(note.drum, pitch)
                    else:
                        pitch = imap.fit_pitch(pitch)
                        rel = (note.velocity if note.velocity is not None else 0.75)
                        rel = rel if rel <= 1.0 else rel / 127
                        banded = imap.velocity_for(note.articulation, rel)
                        if banded is not None:
                            velocity = max(1, min(127, banded))
                        ks = imap.keyswitch_for(note.articulation)
                        if ks is not None:
                            keyswitch_spans.append((ks, max(0.0, time - imap.keyswitch_lead), time + duration))
                        elif note.articulation and "keyswitch" in imap.articulation_mode \
                                and imap.velocity_for(note.articulation, rel) is None:
                            tag = (imap.name, note.articulation)
                            if tag not in warned:
                                warned.add(tag)
                                print(f"Note: {imap.name} has no keyswitch or velocity band for "
                                      f"'{note.articulation}' — edit instruments/{imap.name}.json",
                                      file=sys.stderr)

                midi.addNote(track=track_idx, channel=channel, pitch=pitch, time=time,
                             duration=duration, volume=velocity)

                # Sequential cursor only advances when start was implicit
                if note.start is None:
                    cursor += duration
            except Exception as e:
                print(f"Warning: skipping note {note.pitch} {note.duration}: {e}", file=sys.stderr)

        if imap is not None:
            for ks, start, end in _keyswitch_events(keyswitch_spans, imap.keyswitch_latching):
                midi.addNote(track=track_idx, channel=channel, pitch=ks, time=start,
                             duration=end - start, volume=100)

    with open(path, 'wb') as f:
        midi.writeFile(f)


def _map_for(track: Track, warned: set) -> InstrumentMap | None:
    if not track.vst:
        return None
    try:
        return load_instrument_map(track.vst)
    except FileNotFoundError as e:
        if track.vst not in warned:
            warned.add(track.vst)
            print(f"Warning: {e}; rendering '{track.name}' as General MIDI", file=sys.stderr)
        return None


def _keyswitch_events(spans, latching: bool):
    """Non-latching: merge touching spans of the same keyswitch into one held
    note (so a run of palm mutes is one long keyswitch press). Latching: one
    short press whenever the articulation changes."""
    spans = sorted(spans, key=lambda s: s[1])
    out = []
    if latching:
        current = None
        for ks, start, _ in spans:
            if ks != current:
                out.append((ks, start, start + 0.0625))
                current = ks
        return out
    for ks, start, end in spans:
        if out and out[-1][0] == ks and start <= out[-1][2] + 1e-6:
            out[-1] = (ks, out[-1][1], max(out[-1][2], end))
        else:
            if out and out[-1][2] > start:
                # release the previous articulation before the next one engages
                out[-1] = (out[-1][0], out[-1][1], max(out[-1][1] + 0.01, start))
            out.append((ks, start, end))
    return out


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
