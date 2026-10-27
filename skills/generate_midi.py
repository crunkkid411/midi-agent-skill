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
Track automation (filter, volume, expression, mod_wheel, pitch, macro1-8)
becomes CC / pitch-bend events through the map's cc_map (GM defaults
otherwise). Glide notes overlap the next note by the map's legato_overlap.
Phrase maps (UJAM Beatmaker "Style" mode) replace a drum track's notes with
held song-part keys per section. The composition's tempo_map becomes tempo
events. No program change is sent to VST tracks. `split_tracks=True` also writes one
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

# Automation param → CC for tracks without a map (GM / most synths' defaults).
DEFAULT_CC = {
    "mod_wheel": 1, "volume": 7, "pan": 10, "expression": 11, "filter": 74, "resonance": 71,
    "macro1": 21, "macro2": 22, "macro3": 23, "macro4": 24,
    "macro5": 25, "macro6": 26, "macro7": 27, "macro8": 28,
}


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
    for beat, bpm in composition.tempo_map:
        midi.addTempo(0, beat, bpm)
    for beat, num, den in _meter_changes(composition):
        midi.addTimeSignature(0, beat, num, {1: 0, 2: 1, 4: 2, 8: 3, 16: 4}.get(den, 2), 24)
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
        _write_automation(midi, track_idx, channel, track, imap, warned)

        if imap is not None and imap.type == "phrase":
            for pitch, start, end in _phrase_events(composition, track, imap):
                midi.addNote(track=track_idx, channel=channel, pitch=pitch, time=start,
                             duration=end - start, volume=100)
            continue

        keyswitch_spans = []
        pending = []
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

                if note.articulation == "glide" and imap is not None and imap.legato_overlap:
                    duration += imap.legato_overlap
                pending.append((pitch, time, duration, velocity))

                # Sequential cursor only advances when start was implicit
                if note.start is None:
                    cursor += duration
            except Exception as e:
                print(f"Warning: skipping note {note.pitch} {note.duration}: {e}", file=sys.stderr)

        for pitch, time, duration, velocity in _untangle(pending):
            midi.addNote(track=track_idx, channel=channel, pitch=pitch, time=time,
                         duration=duration, volume=velocity)

        if imap is not None:
            for ks, start, end in _keyswitch_events(keyswitch_spans, imap.keyswitch_latching):
                midi.addNote(track=track_idx, channel=channel, pitch=ks, time=start,
                             duration=end - start, volume=100)

    with open(path, 'wb') as f:
        midi.writeFile(f)


def _untangle(notes):
    """Same-pitch notes on one channel can't overlap in MIDI: a later note-on
    would be cut by the earlier note's note-off (and midiutil can crash on
    it). Hits closer than a 64th note (duplicates, or humanized flams of the
    same drum) merge into one, keeping the louder velocity; an overlapped
    note is shortened to end where the next one starts."""
    by_pitch: dict = {}
    for p, t, d, v in notes:
        by_pitch.setdefault(p, []).append([t, d, v])
    out = []
    for p, items in by_pitch.items():
        items.sort(key=lambda x: (x[0], -x[2]))
        kept = []
        for t, d, v in items:
            if kept and t - kept[-1][0] < 0.0625:
                kept[-1][1] = max(kept[-1][1], t + d - kept[-1][0])
                kept[-1][2] = max(kept[-1][2], v)
                continue
            if kept and kept[-1][0] + kept[-1][1] > t:
                kept[-1][1] = t - kept[-1][0]
            kept.append([t, d, v])
        out.extend((p, t, d, v) for t, d, v in kept)
    return sorted(out, key=lambda x: (x[1], x[0]))


def _write_automation(midi, track_idx, channel, track: Track, imap, warned: set) -> None:
    cc_map = dict(DEFAULT_CC)
    if imap is not None:
        cc_map.update(imap.cc_map)
    for ev in track.automation:
        param, t, v = ev["param"], float(ev["time"]), float(ev["value"])
        if param == "pitch":
            if "semitones" in ev:
                v = float(ev["semitones"]) / (imap.pitch_bend_range if imap else 2)
            midi.addPitchWheelEvent(track_idx, channel, t, int(max(-8192, min(8191, round(v * 8191)))))
            continue
        cc = cc_map.get(param)
        if cc is None:
            if (param, track.name) not in warned:
                warned.add((param, track.name))
                print(f"Note: no CC for automation '{param}' on {track.name}", file=sys.stderr)
            continue
        midi.addControllerEvent(track_idx, channel, t, cc, int(max(0, min(127, round(v * 127)))))


def _phrase_events(composition: Composition, track: Track, imap: InstrumentMap):
    """One held song-part key per top-level section the track plays in.
    Occurrences cycle through the part's keys (Verse 1, Verse 2, ...); a
    section with a phrase-end fill gets the next Fill key for its last beat;
    the final section ends on an End key."""
    tops = [s for s in composition.sections if not s.is_micro]
    starts = sorted(n.start for n in track.notes if n.start is not None)
    counts: dict = {}
    out = []
    for i, sec in enumerate(tops):
        start, end = sec.start_beat(), sec.end_beat()
        if not any(start <= t < end for t in starts) or (track.role or "drums") in (sec.bindings.get("mute") or []):
            continue
        part = imap.part_map.get(sec.base, sec.base)
        if sec.bindings.get("stop"):
            part = "off"
        keys = imap.phrase_keys.get(part) or imap.phrase_keys.get("verse") or []
        if not keys:
            continue
        n = counts.get(part, 0)
        counts[part] = n + 1
        key = keys[n % len(keys)]
        last = i == len(tops) - 1
        fill_keys = imap.phrase_keys.get("fill") or []
        end_keys = imap.phrase_keys.get("end") or []
        tail = 1.0 if (fill_keys or (last and end_keys)) and sec.bars() >= 2 and part != "off" else 0.0
        out.append((key, start, end - tail))
        if tail:
            if last and end_keys:
                out.append((end_keys[0], end - tail, end))
            elif sec.bindings.get("fill") is not False:
                out.append((fill_keys[counts[part] % len(fill_keys)], end - tail, end))
    return out


def _meter_changes(composition: Composition):
    """(beat, numerator, denominator) wherever the bar length changes."""
    out, current = [], None
    leaves = sorted((s for s in composition.sections
                     if s.is_micro or not any(k.name.startswith(s.name + ".") for k in composition.sections)),
                    key=lambda s: s.start_beat())
    for sec in leaves:
        meter = sec.bindings.get("meter") or None
        if meter is None:
            num, den = (4, 4) if abs(sec.beats_per_bar - 4.0) < 1e-6 else (int(sec.beats_per_bar), 4)
        else:
            a, _, b = str(meter).partition("/")
            num, den = int(a), int(b or 4)
        if (num, den) != current and not (current is None and (num, den) == (4, 4)):
            out.append((sec.start_beat(), num, den))
        current = (num, den)
    return out


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
