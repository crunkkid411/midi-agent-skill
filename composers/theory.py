"""
Music theory primitives shared by all composers.

Scales, key parsing, Roman-numeral → chord-tone resolution, drum-name → MIDI
mapping. Kept small and dependency-free.
"""

from typing import List, Tuple, Optional
import re

# -------- Scales (semitone offsets from tonic) --------

SCALES = {
    "major":            [0, 2, 4, 5, 7, 9, 11],
    "ionian":           [0, 2, 4, 5, 7, 9, 11],
    "minor":            [0, 2, 3, 5, 7, 8, 10],   # natural minor
    "aeolian":          [0, 2, 3, 5, 7, 8, 10],
    "harmonic_minor":   [0, 2, 3, 5, 7, 8, 11],
    "melodic_minor":    [0, 2, 3, 5, 7, 9, 11],
    "dorian":           [0, 2, 3, 5, 7, 9, 10],
    "phrygian":         [0, 1, 3, 5, 7, 8, 10],
    "lydian":           [0, 2, 4, 6, 7, 9, 11],
    "mixolydian":       [0, 2, 4, 5, 7, 9, 10],
    "locrian":          [0, 1, 3, 5, 6, 8, 10],
    "minor_pentatonic": [0, 3, 5, 7, 10],
    "major_pentatonic": [0, 2, 4, 7, 9],
    "blues":            [0, 3, 5, 6, 7, 10],
}

NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
NOTE_TO_PC = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}


def parse_key(key: str) -> Tuple[int, str]:
    """Parse 'D Dorian' or 'C minor' → (tonic_pitch_class, scale_name).
    Returns (0, 'minor') as fallback."""
    if not key:
        return 0, "minor"
    parts = key.strip().split(None, 1)
    tonic_str = parts[0]
    scale_str = parts[1].lower().replace(" ", "_") if len(parts) > 1 else "major"

    m = re.match(r"^([A-Ga-g])([#b]?)$", tonic_str)
    if not m:
        return 0, scale_str if scale_str in SCALES else "minor"
    note, accidental = m.groups()
    pc = NOTE_TO_PC[note.upper()]
    if accidental == "#":
        pc = (pc + 1) % 12
    elif accidental == "b":
        pc = (pc - 1) % 12

    if scale_str not in SCALES:
        scale_str = "minor"
    return pc, scale_str


def scale_pitches(tonic_pc: int, scale_name: str, octave: int = 4) -> List[int]:
    """Return MIDI pitches for one octave of the scale starting at the given octave."""
    intervals = SCALES.get(scale_name, SCALES["minor"])
    base = (octave + 1) * 12 + tonic_pc
    return [base + i for i in intervals]


# -------- Roman numeral chord resolution --------

ROMAN_DEGREE = {"i": 0, "ii": 1, "iii": 2, "iv": 3, "v": 4, "vi": 5, "vii": 6}

def parse_roman(roman: str) -> dict:
    """Parse 'iiø7', 'V7b9', 'bVI', 'i9' → {degree, accidental, quality, extensions}.

    quality: 'major', 'minor', 'dim', 'half_dim', 'aug', or None (use scale default)
    """
    s = roman.strip()
    accidental = ""
    if s.startswith("b") or s.startswith("#"):
        accidental = s[0]
        s = s[1:]

    # Match the longest roman prefix
    m = re.match(r"(III|II|IV|VII|VI|V|I|iii|ii|iv|vii|vi|v|i)(.*)$", s)
    if not m:
        return {"degree": 0, "accidental": "", "quality": None, "extensions": [], "alterations": []}
    roman_part, rest = m.group(1), m.group(2)
    degree = ROMAN_DEGREE[roman_part.lower()]

    # Quality marker
    quality = None
    if "°" in rest or "dim" in rest:
        quality = "dim"
        rest = rest.replace("°", "").replace("dim", "")
    elif "ø" in rest:
        quality = "half_dim"
        rest = rest.replace("ø", "")
    elif "+" in rest or "aug" in rest:
        quality = "aug"
        rest = rest.replace("+", "").replace("aug", "")
    elif roman_part.isupper():
        quality = "major"
    elif roman_part.islower():
        quality = "minor"

    # Extensions / alterations
    extensions = []
    alterations = []
    # Match things like "7", "9", "11", "13", "b9", "#11", "b5"
    for tok in re.finditer(r"([b#]?)(\d+)", rest):
        acc, num = tok.group(1), int(tok.group(2))
        if num in (7, 9, 11, 13) and not acc:
            extensions.append(num)
        elif num in (5,) and acc:
            alterations.append((acc, num))
        else:
            alterations.append((acc, num))
    return {
        "degree": degree,
        "accidental": accidental,
        "quality": quality,
        "extensions": extensions,
        "alterations": alterations,
    }


def chord_pitches(roman: str, tonic_pc: int, scale_name: str, base_octave: int = 4) -> List[int]:
    """Resolve a Roman numeral to a list of MIDI pitches (root + chord tones).

    Works for major, minor, dim, half-dim, augmented; supports 7, 9, 11, 13
    extensions and chromatic accidental (b/#) on the root.
    """
    parsed = parse_roman(roman)
    intervals = SCALES.get(scale_name, SCALES["minor"])

    # Root pitch class
    if parsed["degree"] < len(intervals):
        root_offset = intervals[parsed["degree"]]
    else:
        root_offset = 0
    if parsed["accidental"] == "b":
        root_offset -= 1
    elif parsed["accidental"] == "#":
        root_offset += 1
    root = (base_octave + 1) * 12 + tonic_pc + root_offset

    quality = parsed["quality"] or "major"
    # Chord interval pattern (relative to root)
    triad = {
        "major":    [0, 4, 7],
        "minor":    [0, 3, 7],
        "dim":      [0, 3, 6],
        "half_dim": [0, 3, 6],
        "aug":      [0, 4, 8],
    }.get(quality, [0, 4, 7])

    pitches = [root + i for i in triad]

    # Extensions
    for ext in parsed["extensions"]:
        if ext == 7:
            if quality == "major":
                pitches.append(root + 10)  # dominant-7 default for upper-case 7 chords
            elif quality == "minor":
                pitches.append(root + 10)
            elif quality == "dim":
                pitches.append(root + 9)   # diminished 7
            elif quality == "half_dim":
                pitches.append(root + 10)  # m7b5
        elif ext == 9:
            if 7 not in parsed["extensions"]:
                pitches.append(root + 10)  # implicit 7
            pitches.append(root + 14)
        elif ext == 11:
            pitches.append(root + 17)
        elif ext == 13:
            pitches.append(root + 21)

    # Alterations (b9, #11, b5, etc)
    for acc, num in parsed["alterations"]:
        offset = {5: 7, 9: 14, 11: 17, 13: 21}.get(num)
        if offset is None:
            continue
        if acc == "b":
            offset -= 1
        elif acc == "#":
            offset += 1
        pitch = root + offset
        if pitch not in pitches:
            pitches.append(pitch)

    return sorted(set(pitches))


def chord_root(roman: str, tonic_pc: int, scale_name: str, octave: int = 2) -> int:
    """Just the root note of a Roman numeral chord, useful for bass lines."""
    return chord_pitches(roman, tonic_pc, scale_name, base_octave=octave)[0]


# -------- Voicing --------

VOICING_STYLES = ("triads", "extended_7th_9th", "power_chords", "open_voicings", "stab", "sustained")


def voice_chord(roman: str, tonic_pc: int, scale_name: str, register: Tuple[int, int],
                voicing: str = "triads", low_root: Optional[int] = None) -> List[int]:
    """Voice a Roman numeral inside `register`, root always lowest.

    Folding each chord tone into the register independently produces
    inversions (C4 over Eb3-G3). Here the root is placed first, then every
    other tone is stacked above it, so the chord keeps its root position.

    voicing:
      triads / stab / sustained → root + 3rd + 5th (+ extensions the numeral asks for)
      extended_7th_9th          → same, extensions kept
      power_chords              → root + 5th + octave (no 3rd: distortion-safe)
      open_voicings             → root, 5th, 10th (3rd up an octave), + extensions
    low_root: if given (e.g. a guitar's lowest open string), the root is the
      lowest pitch with the right pitch class at or above it.
    """
    pitches = chord_pitches(roman, tonic_pc, scale_name, base_octave=4)
    root_pc = pitches[0] % 12
    intervals = sorted({p - pitches[0] for p in pitches})

    if voicing == "power_chords":
        intervals = [0, 7, 12]
    elif voicing == "open_voicings":
        intervals = sorted({0, 7} | {i + 12 for i in intervals if i not in (0, 7)})
    elif voicing in ("triads", "stab", "sustained"):
        intervals = [i for i in intervals if i <= 7] or [0, 4, 7]

    floor = low_root if low_root is not None else register[0]
    root = floor + ((root_pc - floor) % 12)
    while root + intervals[-1] > register[1] and root - 12 >= floor:
        root -= 12
    return [root + i for i in intervals]


# -------- Tunings (lowest → highest string, MIDI) --------

TUNINGS = {
    "standard_e":     [40, 45, 50, 55, 59, 64],
    "drop_d":         [38, 45, 50, 55, 59, 64],
    "standard_d":     [38, 43, 48, 53, 57, 62],
    "drop_c":         [36, 43, 48, 53, 57, 62],
    "drop_c#":        [37, 44, 49, 54, 58, 63],
    "drop_c_sharp":   [37, 44, 49, 54, 58, 63],
    "drop_db":        [37, 44, 49, 54, 58, 63],
    "standard_c":     [36, 41, 46, 51, 55, 60],
    "drop_b":         [35, 42, 47, 52, 56, 61],
    "drop_a":         [33, 40, 45, 50, 54, 59],
    "seven_b":        [35, 40, 45, 50, 55, 59, 64],
    "seven_drop_a":   [33, 40, 45, 50, 55, 59, 64],
    "eight_f#":       [30, 35, 40, 45, 50, 55, 59, 64],
    "eight_drop_e":   [28, 35, 40, 45, 50, 55, 59, 64],
    "double_drop_c":  [24, 31, 36, 41, 46, 50, 55, 60],
}


def resolve_tuning(tuning) -> Optional[List[int]]:
    """'drop_c' / 'Drop C' / {'strings': [...]} / [36, 43, ...] → list of open-string MIDI pitches."""
    if not tuning:
        return None
    if isinstance(tuning, dict):
        return list(tuning.get("strings") or []) or resolve_tuning(tuning.get("name"))
    if isinstance(tuning, (list, tuple)):
        return [int(p) for p in tuning]
    key = str(tuning).strip().lower().replace(" ", "_").replace("-", "_")
    return TUNINGS.get(key)


def lowest_string(tuning) -> Optional[int]:
    strings = resolve_tuning(tuning)
    return min(strings) if strings else None


def pedal_note(root_pc: int, low: int) -> int:
    """Lowest pitch with pitch class `root_pc` at or above the open string `low`."""
    return low + ((root_pc - low) % 12)


# -------- Drum name → GM MIDI note mapping (channel 9) --------

GM_DRUM = {
    "kick": 36, "bd": 36,
    "snare": 38, "sd": 38,
    "snare_rimshot": 40, "snare_sidestick": 37, "snare_ghost": 38,
    "clap": 39,
    "rim": 37, "rimshot": 37,
    "hh": 42, "hat": 42, "closed_hat": 42, "closedhat": 42,
    "hh_tight": 42, "hh_pedal": 44,
    "oh": 46, "open_hat": 46, "openhat": 46,
    "ride": 51, "ride_bell": 53, "ride_edge": 59,
    "crash": 49, "crash_2": 57, "crash_choke": 49,
    "china": 52, "splash": 55,
    "tom_high": 50, "tom_mid": 47, "tom_low": 45, "floor_tom": 43, "floor_tom_low": 41,
    "perc": 56,  # cowbell as default percussive accent
    "cowbell": 56, "tambourine": 54, "shaker": 70,
}


def drum_pitch(name: str) -> int:
    return GM_DRUM.get(name.lower(), 36)


def midi_to_pitch_str(midi: int) -> str:
    """37 → 'C#2', 60 → 'C4'."""
    octave = midi // 12 - 1
    pc = midi % 12
    return f"{NOTE_NAMES[pc]}{octave}"


def midi_in_register(pitch: int, register: Tuple[int, int]) -> int:
    """Shift pitch by octaves until it sits inside [low, high]."""
    low, high = register
    while pitch < low:
        pitch += 12
    while pitch > high:
        pitch -= 12
    return pitch
