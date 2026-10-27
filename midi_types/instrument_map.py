"""
Instrument maps — genre-independent descriptions of a target instrument
(a VST like Superior Drummer 3 or Odin III, or plain General MIDI).

Composers write instrument-agnostic intent: drum lanes ("china",
"ride_bell") and articulations ("palm_mute", "pinch_harmonic"). At render
time the track's map turns that intent into what the instrument actually
responds to:

  drums:   lane → MIDI note (with fallbacks when a kit lacks a piece)
  melodic: articulation → keyswitch note held across the note (non-latching
           instruments like Odin III) and/or a velocity band (Odin III
           "Swift Mode" picks the articulation from velocity)
  phrase:  the instrument plays its own grooves; the track becomes held
           phrase-trigger keys per section (UJAM Beatmaker "Style" keys)
  all:     abstract automation params (filter, volume, mod_wheel, pitch,
           macro1-8) → CC numbers / pitch bend via `cc_map`

Maps live in instruments/<slug>.json. Any track can target any map, in any
genre: profile `instruments[].vst`, or `--vst role=slug` on the CLI.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import json

INSTRUMENTS_DIR = Path(__file__).parent.parent / "instruments"


@dataclass
class InstrumentMap:
    name: str
    type: str = "melodic"                         # "drums" | "melodic" | "phrase"
    vendor: Optional[str] = None
    aliases: List[str] = field(default_factory=list)
    lanes: Dict[str, int] = field(default_factory=dict)          # drums: lane → note
    fallbacks: Dict[str, str] = field(default_factory=dict)      # drums: missing lane → lane
    range: Optional[Tuple[int, int]] = None                      # melodic: playable notes
    octave_offset: int = 0                                       # added to every melodic note
    articulation_mode: str = "none"               # none | velocity | keyswitch | keyswitch+velocity
    velocity_bands: Dict[str, Tuple[int, int]] = field(default_factory=dict)
    keyswitches: Dict[str, Optional[int]] = field(default_factory=dict)
    articulation_aliases: Dict[str, str] = field(default_factory=dict)  # composer name → map name
    keyswitch_latching: bool = False
    keyswitch_lead: float = 0.03                  # beats a keyswitch starts before its note
    default_articulation: Optional[str] = None
    program: Optional[int] = None                 # GM program change, only for GM maps
    channel: Optional[int] = None
    cc_map: Dict[str, int] = field(default_factory=dict)   # automation param → CC number
    pitch_bend_range: int = 2                     # semitones the synth is set to
    legato_overlap: float = 0.0                   # beats glide notes overlap the next (mono glide)
    phrase_keys: Dict[str, List[int]] = field(default_factory=dict)  # phrase: part → keys
    part_map: Dict[str, str] = field(default_factory=dict)  # phrase: section name → part
    notes: Optional[str] = None

    def drum_note(self, lane: Optional[str], gm_pitch: int) -> int:
        seen = set()
        while lane and lane not in seen:
            seen.add(lane)
            if lane in self.lanes:
                return self.lanes[lane]
            lane = self.fallbacks.get(lane)
        return gm_pitch

    def _names(self, articulation: Optional[str]) -> List[str]:
        a = articulation or self.default_articulation
        if not a:
            return []
        alias = self.articulation_aliases.get(a)
        return [alias, a] if alias else [a]

    def resolve_articulation(self, articulation: Optional[str]) -> Optional[str]:
        names = self._names(articulation)
        return names[0] if names else None

    def keyswitch_for(self, articulation: Optional[str]) -> Optional[int]:
        if "keyswitch" not in self.articulation_mode:
            return None
        for name in self._names(articulation):
            if self.keyswitches.get(name) is not None:
                return self.keyswitches[name]
        return None

    def velocity_for(self, articulation: Optional[str], rel: float) -> Optional[int]:
        """Map a 0-1 relative velocity into the articulation's band.
        In keyswitch+velocity mode the band is used only when no keyswitch is known."""
        if "velocity" not in self.articulation_mode:
            return None
        if self.articulation_mode == "keyswitch+velocity" and self.keyswitch_for(articulation) is not None:
            return None
        band = next((self.velocity_bands[n] for n in self._names(articulation)
                     if n in self.velocity_bands), None)
        if not band:
            return None
        lo, hi = band
        return int(round(lo + (hi - lo) * max(0.0, min(1.0, rel))))

    def fit_pitch(self, pitch: int) -> int:
        pitch += self.octave_offset
        if self.range:
            lo, hi = self.range
            while pitch < lo:
                pitch += 12
            while pitch > hi:
                pitch -= 12
        return max(0, min(127, pitch))

    @classmethod
    def from_dict(cls, d: Dict) -> "InstrumentMap":
        fields = cls.__dataclass_fields__
        kw = {k: v for k, v in d.items() if k in fields}
        if "range" in kw and kw["range"]:
            kw["range"] = tuple(kw["range"])
        kw["velocity_bands"] = {k: tuple(v) for k, v in d.get("velocity_bands", {}).items()}
        return cls(**kw)


_cache: Dict[str, InstrumentMap] = {}


def find_instrument_map(slug: str, directory: Optional[Path] = None) -> Optional[Path]:
    directory = directory or INSTRUMENTS_DIR
    candidate = directory / f"{slug}.json"
    if candidate.exists():
        return candidate
    for f in directory.glob("*.json"):
        if f.name.startswith("_"):
            continue
        try:
            data = json.loads(f.read_text())
        except (json.JSONDecodeError, OSError):
            continue
        if data.get("name") == slug or slug in data.get("aliases", []):
            return f
    return None


def load_instrument_map(slug: str) -> InstrumentMap:
    if slug in _cache:
        return _cache[slug]
    path = find_instrument_map(slug)
    if path is None:
        available = ", ".join(sorted(p.stem for p in INSTRUMENTS_DIR.glob("*.json")
                                     if not p.name.startswith("_")))
        raise FileNotFoundError(f"No instrument map '{slug}' in instruments/. Available: {available}")
    imap = InstrumentMap.from_dict(json.loads(path.read_text()))
    _cache[slug] = imap
    return imap


def list_instrument_maps() -> List[InstrumentMap]:
    return [load_instrument_map(p.stem) for p in sorted(INSTRUMENTS_DIR.glob("*.json"))
            if not p.name.startswith("_")]
