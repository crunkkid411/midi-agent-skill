"""
Type definitions for music composition.

Backwards-compatible extensions:
- Note.velocity (0.0-1.0) and Note.start (beats from track start) are optional.
  Old `{"pitch", "duration"}` notes still work — velocity defaults at render time
  and notes without `start` are placed sequentially.
- Track.is_drum=True routes the track to MIDI channel 9 (GM drums); pitches are
  interpreted as GM drum note numbers (kick=36, snare=38, closed-hh=42, etc).
- Composition.sections is a flat list of named bar ranges (1-indexed, inclusive).
  Micro-sections use dotted names: "verse1.A", "verse2.B", "chorus1.drop".
- Note.articulation ("palm_mute", "open", "pinch_harmonic", "dead", "slide",
  ...) and Note.drum (lane name: "kick", "china", "ride_bell", ...) are
  instrument-agnostic intent. The renderer maps them onto whichever VST the
  track targets (instruments/*.json): keyswitches, velocity bands, note maps.
- Track.role / Track.vst say which pipeline layer produced the track and
  which instrument map renders it. Track.pan (-1.0..1.0) is used for
  double-tracked parts.
"""

from dataclasses import dataclass, field
from typing import List, Optional, Any, Dict, Tuple


@dataclass
class Note:
    pitch: str                          # "C4", "F#5", "Bb3", or GM drum number for is_drum tracks
    duration: str                       # "4", "8", "d4", "T8" — see DURATION_MAP
    velocity: Optional[float] = None    # 0.0-1.0; None → default at render
    start: Optional[float] = None       # beats from track start; None → sequential append
    articulation: Optional[str] = None  # "palm_mute", "open", "pinch_harmonic", "dead", "slide", ...
    drum: Optional[str] = None          # drum lane name for is_drum tracks ("kick", "china", ...)


@dataclass
class Track:
    notes: List[Note] = field(default_factory=list)
    instrument: Optional[str] = None    # GM instrument name (ignored if is_drum)
    is_drum: bool = False               # True → MIDI channel 9
    name: Optional[str] = None          # display name (e.g. "Bass", "Lead")
    role: Optional[str] = None          # pipeline role: "drums", "rhythm_guitar", "bass", ...
    vst: Optional[str] = None           # instrument map slug from instruments/ (None → General MIDI)
    pan: Optional[float] = None         # -1.0 (left) .. 1.0 (right)
    # Controller automation: [{"param": "filter", "time": beats, "value": 0..1}, ...]
    # Params are abstract (filter, volume, expression, mod_wheel, pitch, macro1-8);
    # the track's instrument map turns them into CC numbers / pitch bend.
    automation: List[Dict[str, Any]] = field(default_factory=list)


@dataclass
class Section:
    name: str                           # "verse1", "chorus1.drop", "bridge.B"
    start_bar: int                      # 1-indexed inclusive
    end_bar: int                        # 1-indexed inclusive
    density: Optional[float] = None     # resolved from the profile by the structure planner
    bindings: Dict[str, Any] = field(default_factory=dict)  # resolved SectionBindings (energy, drum_pattern, ...)
    beats_per_bar: float = 4.0          # quarter-note beats per bar (3/4 → 3, 6/8 → 3, 7/8 → 3.5)
    beat_offset: Optional[float] = None # absolute start in beats (set by the planner; meters vary)
    beat_length: Optional[float] = None # total beats when micro-sections mix meters

    def bars(self) -> int:
        return self.end_bar - self.start_bar + 1

    def bar_start(self, bar: int) -> float:
        """Absolute beat where `bar` (1-indexed song bar inside this section) starts."""
        return self.start_beat() + (bar - self.start_bar) * self.beats_per_bar

    @property
    def is_micro(self) -> bool:
        return "." in self.name

    @property
    def base(self) -> str:
        """'verse2.B' → 'verse'"""
        return self.name.partition(".")[0].rstrip("0123456789") or self.name

    def start_beat(self, beats_per_bar: Optional[float] = None) -> float:
        if self.beat_offset is not None:
            return self.beat_offset
        return (self.start_bar - 1) * (beats_per_bar or self.beats_per_bar)

    def end_beat(self, beats_per_bar: Optional[float] = None) -> float:
        if self.beat_length is not None:
            return self.start_beat() + self.beat_length
        return self.start_beat(beats_per_bar) + self.bars() * (beats_per_bar or self.beats_per_bar)


@dataclass
class Composition:
    title: str
    bpm: int
    tracks: List[Track] = field(default_factory=list)
    bars: Optional[int] = None
    time_signature: Tuple[int, int] = (4, 4)
    sections: List[Section] = field(default_factory=list)
    key: Optional[str] = None           # e.g. "D Dorian", "C minor"
    tuning: Optional[str] = None        # e.g. "drop_c" — informs riff/bass pedal notes
    tempo_map: List[Tuple[float, float]] = field(default_factory=list)  # [(beat, bpm)] after beat 0

    def to_dict(self) -> Dict[str, Any]:
        out: Dict[str, Any] = {
            "title": self.title,
            "bpm": self.bpm,
            "tracks": [
                {
                    "name": t.name,
                    "instrument": t.instrument,
                    "is_drum": t.is_drum,
                    **{k: v for k, v in {"role": t.role, "vst": t.vst, "pan": t.pan,
                                         "automation": t.automation or None}.items()
                       if v is not None},
                    "notes": [
                        {k: v for k, v in {
                            "pitch": n.pitch,
                            "duration": n.duration,
                            "velocity": n.velocity,
                            "start": n.start,
                            "articulation": n.articulation,
                            "drum": n.drum,
                        }.items() if v is not None or k in ("pitch", "duration")}
                        for n in t.notes
                    ],
                }
                for t in self.tracks
            ],
        }
        if self.bars is not None:
            out["bars"] = self.bars
        if self.time_signature != (4, 4):
            out["time_signature"] = list(self.time_signature)
        if self.sections:
            out["sections"] = [
                {k: v for k, v in {
                    "name": s.name, "start_bar": s.start_bar, "end_bar": s.end_bar,
                    "density": s.density, "bindings": s.bindings or None,
                    "beats_per_bar": s.beats_per_bar if s.beats_per_bar != 4.0 else None,
                    "beat_offset": s.beat_offset, "beat_length": s.beat_length,
                }.items() if v is not None}
                for s in self.sections
            ]
        if self.key is not None:
            out["key"] = self.key
        if self.tuning is not None:
            out["tuning"] = self.tuning
        if self.tempo_map:
            out["tempo_map"] = [list(t) for t in self.tempo_map]
        return out

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Composition":
        tracks: List[Track] = []
        for t in data.get("tracks", []):
            notes = [
                Note(
                    pitch=str(n["pitch"]),
                    duration=str(n["duration"]),
                    velocity=n.get("velocity"),
                    start=n.get("start"),
                    articulation=n.get("articulation"),
                    drum=n.get("drum"),
                )
                for n in t.get("notes", [])
                if "pitch" in n and "duration" in n
            ]
            tracks.append(Track(
                notes=notes,
                instrument=t.get("instrument"),
                is_drum=bool(t.get("is_drum", False)),
                name=t.get("name"),
                role=t.get("role"),
                vst=t.get("vst"),
                pan=t.get("pan"),
                automation=t.get("automation", []),
            ))

        sections = [
            Section(name=s["name"], start_bar=int(s["start_bar"]), end_bar=int(s["end_bar"]),
                    density=s.get("density"), bindings=s.get("bindings", {}),
                    beats_per_bar=float(s.get("beats_per_bar", 4.0)),
                    beat_offset=s.get("beat_offset"), beat_length=s.get("beat_length"))
            for s in data.get("sections", [])
        ]

        ts = data.get("time_signature", (4, 4))
        if isinstance(ts, list):
            ts = tuple(ts)

        return cls(
            title=data.get("title", "untitled"),
            bpm=data.get("bpm", 90),
            tracks=tracks,
            bars=data.get("bars"),
            time_signature=ts,
            sections=sections,
            key=data.get("key"),
            tuning=data.get("tuning"),
            tempo_map=[tuple(t) for t in data.get("tempo_map", [])],
        )
