"""
Genre Profile schema — the cacheable artifact produced by the research workflow.

A genre profile is a JSON file in `genres/` that captures everything the
deterministic composers need to render a song in that style: tempo, scales,
chord progressions, drum patterns, instrumentation, micro-section structure,
velocity ranges, and human-readable principles.

The agent's job (per workflows/music-theory-research.md) is to fill one of
these once per genre. Subsequent compositions reuse the cached profile.
"""

from dataclasses import dataclass, field
from typing import List, Dict, Optional, Tuple, Any
import json
from pathlib import Path


@dataclass
class ChordProgression:
    roman: List[str]                    # ["i9", "iv9", "iiø7", "V7b9"]
    weight: float = 1.0
    source: Optional[str] = None        # reference song that uses this
    bars_per_chord: float = 1.0         # how long each chord is held


@dataclass
class DrumPattern:
    """16-step grid notation. Each string is 16 chars, one per 16th note.
    'x' = hit, '.' = rest, 'g' = ghost note (low velocity).
    Patterns can also be 8 or 32 steps; length sets the resolution."""
    name: str
    kick: str = "................"
    snare: str = "................"
    hh: str = "................"        # closed hi-hat
    oh: str = "................"        # open hi-hat
    clap: str = "................"
    perc: str = "................"      # extra percussion
    swing: float = 0.5                  # 0.5 = straight, 0.66 = hard swing
    ghost_density: float = 0.0          # probability of inserting ghost notes
    weight: float = 1.0


@dataclass
class MicroSection:
    """A sub-block within a song section — e.g. an 8-bar verse split into
    A (bars 1-4) and B (bars 5-8). The agent and user can address these by
    dotted name: 'verse.B', 'chorus.drop'."""
    name: str                           # "A", "B", "drop", "break"
    bars: int                           # length within the parent section
    density: float = 0.7                # 0.0-1.0 how filled the section is
    note: Optional[str] = None          # human description ("drum break", "stripped")


@dataclass
class SongSection:
    name: str                           # "intro", "verse", "chorus", "bridge", "outro"
    bars: int                           # total bars in this section
    density: float = 0.7
    micro: List[MicroSection] = field(default_factory=list)
    note: Optional[str] = None


@dataclass
class InstrumentRole:
    role: str                           # "drums", "bass", "chords", "melody", "pad", "lead"
    gm: Optional[str] = None            # GM instrument name (None for drum kits)
    register: Tuple[int, int] = (36, 84)  # MIDI note range
    style: Optional[str] = None         # "sustained_root", "walking", "arpeggio", "stab"
    velocity_range: Tuple[float, float] = (0.5, 0.8)


@dataclass
class GenreProfile:
    name: str
    aliases: List[str] = field(default_factory=list)
    tempo_range: Tuple[int, int] = (80, 120)
    default_key: str = "C minor"
    common_keys: List[str] = field(default_factory=list)
    scales: List[str] = field(default_factory=list)
    chord_progressions: List[ChordProgression] = field(default_factory=list)
    voicing_style: str = "triads"       # "triads", "extended_7th_9th", "power_chords", "open_voicings"
    drum_patterns: List[DrumPattern] = field(default_factory=list)
    song_sections: List[SongSection] = field(default_factory=list)
    instruments: List[InstrumentRole] = field(default_factory=list)
    velocity_ranges: Dict[str, Tuple[float, float]] = field(default_factory=dict)
    principles: List[str] = field(default_factory=list)
    references: List[Dict[str, str]] = field(default_factory=list)
    research_sources: List[str] = field(default_factory=list)
    sound_design: Dict[str, Any] = field(default_factory=dict)  # forward-compat: synth presets, sample paths
    notes: Optional[str] = None         # free-form agent notes

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "GenreProfile":
        return cls(
            name=data["name"],
            aliases=data.get("aliases", []),
            tempo_range=tuple(data.get("tempo_range", [80, 120])),
            default_key=data.get("default_key", "C minor"),
            common_keys=data.get("common_keys", []),
            scales=data.get("scales", []),
            chord_progressions=[
                ChordProgression(
                    roman=p["roman"],
                    weight=p.get("weight", 1.0),
                    source=p.get("source"),
                    bars_per_chord=p.get("bars_per_chord", 1.0),
                )
                for p in data.get("chord_progressions", [])
            ],
            voicing_style=data.get("voicing_style", "triads"),
            drum_patterns=[
                DrumPattern(**{k: v for k, v in p.items() if k in DrumPattern.__dataclass_fields__})
                for p in data.get("drum_patterns", [])
            ],
            song_sections=[
                SongSection(
                    name=s["name"],
                    bars=s["bars"],
                    density=s.get("density", 0.7),
                    micro=[MicroSection(**m) for m in s.get("micro", [])],
                    note=s.get("note"),
                )
                for s in data.get("song_sections", [])
            ],
            instruments=[
                InstrumentRole(
                    role=i["role"],
                    gm=i.get("gm"),
                    register=tuple(i.get("register", [36, 84])),
                    style=i.get("style"),
                    velocity_range=tuple(i.get("velocity_range", [0.5, 0.8])),
                )
                for i in data.get("instruments", [])
            ],
            velocity_ranges={k: tuple(v) for k, v in data.get("velocity_ranges", {}).items()},
            principles=data.get("principles", []),
            references=data.get("references", []),
            research_sources=data.get("research_sources", []),
            sound_design=data.get("sound_design", {}),
            notes=data.get("notes"),
        )

    @classmethod
    def load(cls, path: str | Path) -> "GenreProfile":
        with open(path, "r") as f:
            return cls.from_dict(json.load(f))


def find_profile(slug: str, genres_dir: Optional[Path] = None) -> Optional[Path]:
    """Resolve a genre slug or alias to a file path under genres/."""
    if genres_dir is None:
        genres_dir = Path(__file__).parent.parent / "genres"

    candidate = genres_dir / f"{slug}.json"
    if candidate.exists():
        return candidate

    # Search by alias
    for f in genres_dir.glob("*.json"):
        if f.name.startswith("_"):
            continue
        try:
            with open(f) as fh:
                data = json.load(fh)
            if data.get("name") == slug or slug in data.get("aliases", []):
                return f
        except (json.JSONDecodeError, OSError):
            continue
    return None
