"""
Bass track composer.

Reads the harmony timeline + the tracks already built (drums, rhythm
guitar). Style comes from section binding `bass_style` → instrument style:
  - sustained_root      → root held for the chord's duration
  - walking             → root + scale-tone walk on every beat
  - kick_locked         → root on every kick hit (aliases: sub_808*, *808*, *kick*)
  - doubled             → doubles the rhythm guitar's low notes, folded into
                          the bass register (metalcore / djent / punk)
  - root_eighths        → driving 8th-note roots (punk, rock, metal choruses)
  - octave              → root / octave-up 8ths (disco, dance, scenecore)
  - arpeggio            → root-fifth-octave-fifth 8ths
Stop sections get a single held root; muted sections are silent.
"""

from typing import List, Optional
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile
from .theory import (
    chord_pitches, parse_key, midi_in_register, midi_to_pitch_str, SCALES, lowest_string,
    pedal_note,
)
from .profile import get_instrument, velocity_range, style_for
from .harmony import ChordSpan
from .structure import is_muted, is_stop
from .chords import beats_to_duration


def generate_bass(
    profile: GenreProfile,
    sections: List[Section],
    timeline: List[ChordSpan],
    drum_track: Optional[Track] = None,
    key: Optional[str] = None,
    rng: Optional[random.Random] = None,
    riff_track: Optional[Track] = None,
) -> Track:
    rng = rng or random.Random()
    inst = get_instrument(profile, "bass")
    if inst is None:
        return Track(notes=[], name="Bass", role="bass")

    tonic_pc, _ = parse_key(key or profile.default_key)
    register = inst.register
    vmin, vmax = velocity_range(profile, "bass")
    guitar_low = lowest_string(profile.tuning)
    bass_low = max(register[0], guitar_low - 12) if guitar_low else register[0]

    kicks = sorted(
        n.start for n in (drum_track.notes if drum_track else [])
        if n.start is not None and (n.drum == "kick" or (n.drum is None and str(n.pitch) == "36"))
    )

    notes: List[Note] = []
    stopped = set()

    def add(p: int, start: float, beats: float, vel: Optional[float] = None, art=None):
        notes.append(Note(
            pitch=midi_to_pitch_str(p), duration=beats_to_duration(beats),
            velocity=round(vel if vel is not None else rng.uniform(vmin, vmax), 3),
            start=round(start, 4), articulation=art,
        ))

    for span in timeline:
        sec = span.section
        if is_muted(sec, "bass"):
            continue
        style = (style_for(sec, "bass", inst.style or "sustained_root") or "").lower()
        tones = chord_pitches(span.roman, tonic_pc, span.scale, base_octave=2)
        root = pedal_note(tones[0] % 12, bass_low) if guitar_low else midi_in_register(tones[0], register)
        fifth = midi_in_register(root + 7, register)
        octave = midi_in_register(root + 12, register) if root + 12 <= register[1] else root
        dur = span.end - span.start

        if is_stop(sec):
            if sec.name not in stopped:
                stopped.add(sec.name)
                add(root, sec.start_beat(), 1.5, vmax)
            continue

        if style in ("sustained_root", "sustained"):
            add(root, span.start, dur)
        elif style == "walking":
            scale_intervals = SCALES.get(span.scale, SCALES["minor"])
            for beat in range(int(dur)):
                if beat % 4 == 0:
                    p = root
                elif beat % 4 == 2:
                    p = fifth
                else:
                    p = midi_in_register(root + rng.choice(scale_intervals[:4]), register)
                add(p, span.start + beat, 1.0)
        elif style == "doubled" and riff_track is not None:
            _double(riff_track, span, register, bass_low, vmin, vmax, rng, notes)
        elif "808" in style or "sub" in style or "kick" in style or style == "doubled":
            hits = [t for t in kicks if span.start <= t < span.end]
            if not hits:
                hits = [span.start + b for b in range(int(dur))]
            for i, t in enumerate(hits):
                nxt = next((h for h in hits[i + 1:]), span.end)
                add(root if i % 4 != 2 else fifth, t, max(0.25, min(1.0, nxt - t)))
        elif style == "root_eighths":
            for s in range(int(dur * 2)):
                add(root, span.start + s * 0.5, 0.5,
                    rng.uniform(vmin, vmax) * (1.0 if s % 2 == 0 else 0.9))
        elif style == "octave":
            for s in range(int(dur * 2)):
                add(root if s % 2 == 0 else octave, span.start + s * 0.5, 0.5)
        elif style == "arpeggio":
            pattern = [root, fifth, octave, fifth]
            for s in range(int(dur * 2)):
                add(pattern[s % 4], span.start + s * 0.5, 0.5)
        else:
            add(root, span.start, min(2.0, dur))
            if dur >= 4:
                add(fifth, span.start + 2, 2.0)

    return Track(name="Bass", instrument=inst.gm or "electric-bass-finger", notes=notes, role="bass")


def _double(riff: Track, span: ChordSpan, register, bass_low, vmin, vmax, rng, out) -> None:
    """Copy the riff's lowest note at each onset into the bass register.
    High 'lead' notes above the pedal are skipped (metalerator's bass rule:
    the bass follows the chugs and the roots, not the melodic fills)."""
    from skills.generate_midi import parse_pitch, parse_duration
    by_start = {}
    for n in riff.notes:
        if n.start is None or not (span.start <= n.start < span.end):
            continue
        p = parse_pitch(n.pitch)
        if n.start not in by_start or p < by_start[n.start][0]:
            by_start[n.start] = (p, parse_duration(n.duration), n.articulation)
    if not by_start:
        return
    floor = min(p for p, _, _ in by_start.values())
    for t in sorted(by_start):
        p, d, art = by_start[t]
        if p > floor + 12:
            continue
        bp = p
        while bp > register[1] or bp - 12 >= bass_low:
            bp -= 12
        while bp < register[0]:
            bp += 12
        vel = rng.uniform(vmin + (vmax - vmin) * 0.5, vmax)
        out.append(Note(pitch=midi_to_pitch_str(bp), duration=beats_to_duration(d),
                        velocity=round(vel, 3), start=round(t, 4),
                        articulation="palm_mute" if art == "palm_mute" else None))
