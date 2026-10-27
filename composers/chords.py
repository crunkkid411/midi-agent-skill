"""
Chord / pad track composer.

Renders the harmony timeline in the role's register. Voicing comes from
section binding → instrument `voicing` → profile `voicing_style`:
triads, extended_7th_9th, power_chords (root-5-8, no 3rd), open_voicings.
Chords stay in root position (theory.voice_chord) instead of folding each
tone into the register independently.

Styles (instrument `style`, overridable per section via `chords_style`,
which may also be a {role: style} dict):
  stab        — 8th-note stabs
  sustained   — held
  sparse_held — held, some chords skipped at low density
  strum8      — 8th strums with accented downbeats
  pulse4      — quarter-note pulses
  trance_gate — 16th-note gated chords (instrument articulations.gate, a
                16-step x/. string; default "x.xxx.xxx.xx.xx.")
  follow_riff — stabs on the rhythm guitar's accents and low attacks
                (synths doubling breakdown chugs); needs the riff first in
                layer_order, falls back to stab otherwise
"""

from typing import List, Optional
import random

from midi_types.music import Track, Note, Section
from midi_types.genre_profile import GenreProfile
from .theory import voice_chord, parse_key, midi_to_pitch_str, lowest_string
from .profile import get_instrument, velocity_range, style_for
from .harmony import ChordSpan
from .structure import section_density, section_energy, is_muted, is_stop


def generate_chords(
    profile: GenreProfile,
    sections: List[Section],
    timeline: List[ChordSpan],
    key: Optional[str] = None,
    rng: Optional[random.Random] = None,
    role: str = "chords",
    riff_attacks: Optional[list] = None,
) -> Track:
    rng = rng or random.Random()
    inst = get_instrument(profile, role)
    if inst is None:
        return Track(notes=[], name=role.title(), role=role)

    tonic_pc, _ = parse_key(key or profile.default_key)
    register = inst.register
    vmin, vmax = velocity_range(profile, role)
    low = lowest_string(profile.tuning) if "guitar" in (inst.gm or "") else None
    gate = str(inst.articulations.get("gate", "x.xxx.xxx.xx.xx."))

    notes: List[Note] = []
    stopped = set()
    for span in timeline:
        sec = span.section
        if is_muted(sec, role):
            continue
        style = style_for(sec, role, inst.style or "sustained")
        voicing = sec.bindings.get("voicing") or inst.voicing or profile.voicing_style
        voiced = voice_chord(span.roman, tonic_pc, span.scale, register, voicing, low_root=low)
        energy = section_energy(profile, sec)
        vel = rng.uniform(vmin, vmax) * (0.85 + 0.15 * energy)
        dur_beats = span.end - span.start

        if is_stop(sec):
            if sec.name not in stopped:
                stopped.add(sec.name)
                _hit(notes, voiced, sec.start_beat(), 1.5, vel)
            continue

        if style == "follow_riff" and riff_attacks:
            by_start = {}
            for a in riff_attacks:
                if span.start <= a.start < span.end and (a.accent or a.low):
                    prev = by_start.get(a.start)
                    if prev is None or (a.accent and not prev.accent):
                        by_start[a.start] = a
            hits = [by_start[t] for t in sorted(by_start)]
            for i, a in enumerate(hits):
                nxt = hits[i + 1].start if i + 1 < len(hits) else span.end
                _hit(notes, voiced, a.start, max(0.25, min(a.duration, nxt - a.start)),
                     vel * (1.0 if a.accent else 0.85))
        elif style == "trance_gate":
            step = 4.0 / len(gate)
            for i in range(int(dur_beats / step)):
                if gate[i % len(gate)] in "xa":
                    _hit(notes, voiced, span.start + i * step, step * 0.9,
                         vel * (1.0 if i % 4 == 0 else 0.88))
        elif style in ("stab", "follow_riff"):
            for offset in range(int(dur_beats * 2)):  # 8th-note grid
                if offset % 4 == 0 or rng.random() < 0.4:
                    _hit(notes, voiced, span.start + offset * 0.5, 0.25,
                         vel * rng.uniform(0.85, 1.0))
        elif style == "strum8":
            for offset in range(int(dur_beats * 2)):
                accent = offset % 2 == 0
                _hit(notes, voiced, span.start + offset * 0.5, 0.5,
                     vel * (1.0 if accent else 0.8), "open" if accent else "palm_mute")
        elif style == "pulse4":
            for beat in range(int(dur_beats)):
                _hit(notes, voiced, span.start + beat, 1.0, vel * (1.0 if beat % 2 == 0 else 0.9))
        elif style == "sparse_held" and section_density(profile, sec) < 0.5 and rng.random() < 0.3:
            continue
        else:
            _hit(notes, voiced, span.start, dur_beats, vel)

    return Track(
        name=role.title(),
        instrument=inst.gm or "electric-piano-1",
        notes=notes,
        role=role,
    )


def _hit(notes: List[Note], pitches: List[int], start: float, beats: float, vel: float,
         articulation: Optional[str] = None) -> None:
    for p in pitches:
        notes.append(Note(
            pitch=midi_to_pitch_str(p), duration=beats_to_duration(beats),
            velocity=round(min(1.0, vel), 3), start=round(start, 4), articulation=articulation,
        ))


def beats_to_duration(beats: float) -> str:
    """Map beat counts to a duration token. Exact tokens where possible,
    otherwise a numeric division (4/beats) that parse_duration understands."""
    table = {4.0: "1", 3.0: "d2", 2.0: "2", 1.5: "d4", 1.0: "4", 0.75: "d8",
             0.5: "8", 0.25: "16", 0.125: "32"}
    for b, tok in table.items():
        if abs(beats - b) < 0.01:
            return tok
    if beats > 0:
        return f"{4.0 / beats:.6g}"
    return "16"
