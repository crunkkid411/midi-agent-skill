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
"""

from dataclasses import dataclass, field
from typing import List, Optional, Any, Dict, Tuple


@dataclass
class Note:
    pitch: str                          # "C4", "F#5", "Bb3", or GM drum number for is_drum tracks
    duration: str                       # "4", "8", "d4", "T8" — see DURATION_MAP
    velocity: Optional[float] = None    # 0.0-1.0; None → default at render
    start: Optional[float] = None       # beats from track start; None → sequential append


@dataclass
class Track:
    notes: List[Note] = field(default_factory=list)
    instrument: Optional[str] = None    # GM instrument name (ignored if is_drum)
    is_drum: bool = False               # True → MIDI channel 9
    name: Optional[str] = None          # display name (e.g. "Bass", "Lead")


@dataclass
class Section:
    name: str                           # "verse1", "chorus1.drop", "bridge.B"
    start_bar: int                      # 1-indexed inclusive
    end_bar: int                        # 1-indexed inclusive

    def bars(self) -> int:
        return self.end_bar - self.start_bar + 1


@dataclass
class Composition:
    title: str
    bpm: int
    tracks: List[Track] = field(default_factory=list)
    bars: Optional[int] = None
    time_signature: Tuple[int, int] = (4, 4)
    sections: List[Section] = field(default_factory=list)
    key: Optional[str] = None           # e.g. "D Dorian", "C minor"

    def to_dict(self) -> Dict[str, Any]:
        out: Dict[str, Any] = {
            "title": self.title,
            "bpm": self.bpm,
            "tracks": [
                {
                    "name": t.name,
                    "instrument": t.instrument,
                    "is_drum": t.is_drum,
                    "notes": [
                        {k: v for k, v in {
                            "pitch": n.pitch,
                            "duration": n.duration,
                            "velocity": n.velocity,
                            "start": n.start,
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
                {"name": s.name, "start_bar": s.start_bar, "end_bar": s.end_bar}
                for s in self.sections
            ]
        if self.key is not None:
            out["key"] = self.key
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
                )
                for n in t.get("notes", [])
                if "pitch" in n and "duration" in n
            ]
            tracks.append(Track(
                notes=notes,
                instrument=t.get("instrument"),
                is_drum=bool(t.get("is_drum", False)),
                name=t.get("name"),
            ))

        sections = [
            Section(name=s["name"], start_bar=int(s["start_bar"]), end_bar=int(s["end_bar"]))
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
        )
