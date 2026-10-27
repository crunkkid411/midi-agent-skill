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
    name: Optional[str] = None          # addressable by sections: "progression": "breakdown-pedal"


DRUM_GRID_FIELDS = ("kick", "snare", "hh", "oh", "clap", "perc")


@dataclass
class DrumPattern:
    """16-step grid notation. Each string is 16 chars, one per 16th note.
    'x' = hit, '.' = rest, 'g' = ghost note (low velocity), 'a' = accent.
    Patterns can also be 8, 12, 24 or 32 steps; length sets the resolution
    (12/24 = triplet grids).

    The six classic lanes are fields; any other lane (crash, china, ride,
    ride_bell, splash, tom_high, tom_mid, tom_low, floor_tom,
    snare_rimshot, cowbell, ...) lives in `lanes`.

    kick_follow=True replaces the kick grid with the rhythm guitar's
    attacks (metalcore/djent breakdowns: the kick doubles the chugs)."""
    name: str
    kick: str = "................"
    snare: str = "................"
    hh: str = "................"        # closed hi-hat
    oh: str = "................"        # open hi-hat
    clap: str = "................"
    perc: str = "................"      # extra percussion
    lanes: Dict[str, str] = field(default_factory=dict)
    swing: float = 0.5                  # 0.5 = straight, 0.66 = hard swing
    ghost_density: float = 0.0          # probability of inserting ghost notes
    weight: float = 1.0
    kick_follow: bool = False
    feel: Optional[str] = None          # "halftime", "double_time", "blast" — informational
    layer: str = "acoustic"             # "acoustic" (role drums) | "electronic" (role electronic_drums)

    def all_lanes(self) -> Dict[str, str]:
        out = {k: getattr(self, k) for k in DRUM_GRID_FIELDS}
        out.update(self.lanes)
        return {k: v for k, v in out.items() if v and any(c not in ". " for c in v)}

    @classmethod
    def from_dict(cls, p: Dict[str, Any]) -> "DrumPattern":
        known = {k: v for k, v in p.items() if k in cls.__dataclass_fields__ and k != "lanes"}
        lanes = dict(p.get("lanes", {}))
        for k, v in p.items():
            if k not in cls.__dataclass_fields__ and isinstance(v, str):
                lanes[k] = v
        return cls(lanes=lanes, **known)


# Keys a section / micro-section / form entry may set to steer composers.
SECTION_BINDING_KEYS = (
    "energy", "drum_pattern", "electronic_pattern", "progression", "scale", "riff_style",
    "bass_style", "melody_style", "chords_style", "voicing", "fill", "stop", "mute", "crash",
    "kick_follow", "double_track", "automation", "tempo", "tempo_scale",
)


@dataclass
class SectionBindings:
    """Per-section composer steering. Everything is optional; unset values
    fall through micro → parent section → instrument/profile defaults.

    energy: 0-1 intensity (how hard), independent of density (how busy).
      A breakdown is low density, high energy.
    drum_pattern: pattern name, or a list played one per phrase.
    progression: chord progression name or index.
    scale: per-section scale override ("phrygian" in the verse, "minor" in the chorus).
    riff_style / bass_style / melody_style / voicing: override the role's style here.
    fill: False disables phrase-end fills; True forces one at the section end.
    stop: True → one hit on beat 1, then silence (the metalcore "stop").
    mute: roles silent in this section (["melody", "lead"]).
    crash: False suppresses the crash on the section's downbeat.
    kick_follow: force the kick to double the rhythm guitar.
    electronic_pattern: pattern(s) for the electronic drum layer here (silent where unset).
    chords_style: chords/pad style override here (stab, trance_gate, follow_riff, ...).
    double_track: take relationship here: identical | variation | octave | harmony.
    automation: controller moves over the section, e.g.
      [{"param": "filter", "shape": "ramp_up", "roles": ["pad"], "from": 0.2, "to": 1.0}]
    tempo / tempo_scale: absolute BPM, or a multiple of the song tempo, for this section.
    """
    energy: Optional[float] = None
    drum_pattern: Any = None
    electronic_pattern: Any = None
    chords_style: Optional[str] = None
    double_track: Optional[str] = None
    automation: List[Dict[str, Any]] = field(default_factory=list)
    tempo: Optional[float] = None
    tempo_scale: Optional[float] = None
    progression: Any = None
    scale: Optional[str] = None
    riff_style: Optional[str] = None
    bass_style: Optional[str] = None
    melody_style: Optional[str] = None
    voicing: Optional[str] = None
    fill: Optional[bool] = None
    stop: Optional[bool] = None
    mute: List[str] = field(default_factory=list)
    crash: Optional[bool] = None
    kick_follow: Optional[bool] = None

    @staticmethod
    def kwargs(d: Dict[str, Any]) -> Dict[str, Any]:
        return {k: d[k] for k in SECTION_BINDING_KEYS if k in d}


@dataclass
class MicroSection(SectionBindings):
    """A sub-block within a song section — e.g. an 8-bar verse split into
    A (bars 1-4) and B (bars 5-8). The agent and user can address these by
    dotted name: 'verse.B', 'chorus.drop'."""
    name: str = "A"                     # "A", "B", "drop", "break"
    bars: int = 4                       # length within the parent section
    density: float = 0.7                # 0.0-1.0 how filled the section is
    note: Optional[str] = None          # human description ("drum break", "stripped")


@dataclass
class SongSection(SectionBindings):
    name: str = "verse"                 # "intro", "verse", "chorus", "breakdown", "outro"
    bars: int = 8                       # total bars in this section
    density: float = 0.7
    micro: List[MicroSection] = field(default_factory=list)
    note: Optional[str] = None


@dataclass
class FormEntry:
    """One slot in a profile's song form. `section` names a song_sections
    entry; `bars` overrides its length; `probability` < 1 makes the slot
    optional (rolled with the composition seed); any SECTION_BINDING_KEYS
    in `overrides` apply to this occurrence only (e.g. the final chorus at
    higher energy)."""
    section: str
    bars: Optional[int] = None
    probability: float = 1.0
    overrides: Dict[str, Any] = field(default_factory=dict)


@dataclass
class DoubleTrack:
    """Multi-take recording of one part. takes: 2 (L/R) or 4 (L/R + inner L2/R2).
    mode (overridable per section via the `double_track` binding):
      identical — same notes, independent timing/velocity (tight, breakdowns)
      variation — the right take changes some melodic notes / pinches (verses)
      octave    — the right take plays its open (non-chug) notes an octave up (choruses)
      harmony   — the right take plays its open notes a diatonic 3rd up
    Chugs (palm mutes) stay identical in every mode — that is what keeps it tight."""
    takes: int = 2
    mode: str = "identical"
    pan: float = 1.0
    inner_pan: float = 0.6
    variation_rate: float = 0.15

    @classmethod
    def parse(cls, value) -> Optional["DoubleTrack"]:
        if not value:
            return None
        if value is True:
            return cls()
        if isinstance(value, str):
            return cls(mode=value)
        return cls(**{k: v for k, v in value.items() if k in cls.__dataclass_fields__})


@dataclass
class InstrumentRole:
    role: str                           # "drums", "rhythm_guitar", "bass", "chords", "melody", "pad", "lead"
    gm: Optional[str] = None            # GM instrument name (None for drum kits)
    register: Tuple[int, int] = (36, 84)  # MIDI note range
    style: Optional[str] = None         # "sustained_root", "walking", "pedal_tone", "stab", ...
    velocity_range: Tuple[float, float] = (0.5, 0.8)
    vst: Optional[str] = None           # instrument map slug (instruments/<slug>.json)
    voicing: Optional[str] = None       # overrides profile.voicing_style for this role
    double_track: Optional[DoubleTrack] = None  # multi-take recording (see DoubleTrack)
    articulations: Dict[str, Any] = field(default_factory=dict)  # e.g. {"pinch_harmonic_rate": 0.05}
    play_in: List[str] = field(default_factory=list)  # section names this role plays in (empty = all)


@dataclass
class FillConfig:
    """Phrase-end drum fills. Every `phrase_bars`, the last `beats` of the
    phrase are replaced with a fill picked from `types` with `probability`."""
    phrase_bars: int = 4
    probability: float = 0.7
    types: List[str] = field(default_factory=lambda: ["snare_roll", "tom_run"])
    beats: List[float] = field(default_factory=lambda: [1.0, 2.0])
    section_end_probability: float = 1.0


@dataclass
class MotifConfig:
    """Write `bars` of an idea once per section, then repeat it with
    variation instead of re-rolling every note. `variations` are applied
    cyclically to later repetitions:
      'exact'       — repeat as-is
      'last_bar'    — regenerate the final bar of the repetition
      'last_half'   — regenerate the second half
      'octave_drop' — high notes drop an octave
      'last_root'   — move the final chord root to another scale degree
    """
    bars: int = 4
    variations: List[str] = field(default_factory=lambda: ["exact", "last_bar", "octave_drop", "last_half"])
    roles: List[str] = field(default_factory=lambda: ["rhythm_guitar", "melody", "lead"])


@dataclass
class Humanize:
    timing: float = 0.0                 # max +/- beats of start jitter
    velocity: float = 0.0               # max +/- velocity (0-1 scale)


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
    sound_design: Dict[str, Any] = field(default_factory=dict)  # synth presets, sample paths, fx
    notes: Optional[str] = None         # free-form agent notes
    tuning: Any = None                  # "drop_c" or {"strings": [...]} — guitar/bass pedal notes
    layer_order: List[str] = field(default_factory=list)  # e.g. ["rhythm_guitar", "drums", "bass", ...]
    form: List[FormEntry] = field(default_factory=list)
    fills: Optional[FillConfig] = None
    motif: Optional[MotifConfig] = None
    humanize: Humanize = field(default_factory=Humanize)

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
                    name=p.get("name"),
                )
                for p in data.get("chord_progressions", [])
            ],
            voicing_style=data.get("voicing_style", "triads"),
            drum_patterns=[DrumPattern.from_dict(p) for p in data.get("drum_patterns", [])],
            song_sections=[
                SongSection(
                    name=s["name"],
                    bars=s["bars"],
                    density=s.get("density", 0.7),
                    micro=[
                        MicroSection(
                            name=m["name"], bars=m["bars"],
                            density=m.get("density", s.get("density", 0.7)),
                            note=m.get("note"), **SectionBindings.kwargs(m),
                        )
                        for m in s.get("micro", [])
                    ],
                    note=s.get("note"),
                    **SectionBindings.kwargs(s),
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
                    vst=i.get("vst"),
                    voicing=i.get("voicing"),
                    double_track=DoubleTrack.parse(i.get("double_track")),
                    articulations=i.get("articulations", {}),
                    play_in=i.get("play_in", []),
                )
                for i in data.get("instruments", [])
            ],
            velocity_ranges={k: tuple(v) for k, v in data.get("velocity_ranges", {}).items()},
            principles=data.get("principles", []),
            references=data.get("references", []),
            research_sources=data.get("research_sources", []),
            sound_design=data.get("sound_design", {}),
            notes=data.get("notes"),
            tuning=data.get("tuning"),
            layer_order=data.get("layer_order", []),
            form=[
                FormEntry(
                    section=f["section"], bars=f.get("bars"),
                    probability=f.get("probability", 1.0),
                    overrides=SectionBindings.kwargs(f),
                )
                for f in data.get("form", [])
            ],
            fills=FillConfig(**data["fills"]) if data.get("fills") else None,
            motif=MotifConfig(**data["motif"]) if data.get("motif") else None,
            humanize=Humanize(**data.get("humanize", {})),
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
