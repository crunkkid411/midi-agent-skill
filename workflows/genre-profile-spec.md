# Genre Profile Specification

> Quick reference for what goes into a `genres/<slug>.json`. Full schema
> in `genres/_schema.json`.

---

## Required Fields

| Field | Type | Notes |
|-------|------|-------|
| `name` | string | Lowercase-hyphenated. Must match filename. |
| `tempo_range` | `[min, max]` | BPM. Composers use the midpoint by default. |
| `default_key` | string | "D Dorian", "C minor", "F# major" |
| `scales` | array | Mode names. See "Scales" below. |
| `chord_progressions` | array | Roman numerals. See "Chord Notation" below. |
| `drum_patterns` | array | 16-step grid strings. See "Drum Grid" below. |
| `song_sections` | array | With `micro` sub-blocks. The nuance layer. |
| `instruments` | array | Per-role instrument config. |
| `principles` | array | 2-4 plain-English rules. |

## Scales

Recognized scale names (case-insensitive, underscore for spaces):

`major`, `ionian`, `minor`, `aeolian`, `harmonic_minor`, `melodic_minor`,
`dorian`, `phrygian`, `lydian`, `mixolydian`, `locrian`,
`minor_pentatonic`, `major_pentatonic`, `blues`.

## Chord Notation (Roman Numerals)

- **Case** = quality: `I` major, `i` minor.
- **Suffix** = quality marker: `°` or `dim` = diminished, `ø` = half-diminished, `+` or `aug` = augmented.
- **Numbers** = extensions: `7`, `9`, `11`, `13`. Implicit lower extensions are added (a 9 chord includes the 7).
- **Accidentals** = chromatic alteration: `b9`, `#11`, `b5`. Prefix `b`/`#` on the Roman numeral itself shifts the root chromatically: `bVI` = chord on flat-6.
- **Examples**: `i9`, `iv9`, `iiø7`, `V7b9`, `bVI`, `#iv°7`.

## Drum Grid

Each lane is a string. Length = resolution:
- 16 chars → 16th-note resolution (one bar).
- 8 chars → 8th-note resolution.
- 32 chars → 32nd-note resolution.

Characters:
- `'x'` — hit
- `'.'` — rest
- `'g'` — ghost note (low velocity)

Recognized lanes: `kick`, `snare`, `clap`, `hh` (closed hi-hat), `oh` (open hi-hat), `perc`.

```json
{
  "name": "boom-bap-lazy",
  "kick":  "x.....x...x.....",
  "snare": "....x.......x...",
  "hh":    "x.x.x.x.x.x.x.x.",
  "swing": 0.58,
  "ghost_density": 0.3
}
```

`swing`: 0.5 = straight, 0.66 = hard swing. Applies to offbeat 16ths.
`ghost_density`: 0.0-1.0 — probability of inserting ghost notes on rests.

## Song Sections + Micro-Sections

```json
{
  "name": "verse",
  "bars": 8,
  "density": 0.7,
  "note": "Optional human description",
  "micro": [
    { "name": "A", "bars": 4, "density": 0.6, "note": "Sparse" },
    { "name": "B", "bars": 4, "density": 0.85, "note": "Build" }
  ]
}
```

- `name` should be one of: `intro`, `verse`, `pre-chorus`, `chorus`, `drop`, `bridge`, `break`, `outro`, `loop`.
- `density` (0.0-1.0) drives composer behavior (see lego-pipeline.md).
- `micro` sub-blocks get dotted addressing: `verse.A`, `chorus.drop`. When the structure planner repeats sections (e.g. for a 32-bar song), they get numbered: `verse1.A`, `verse2.B`.

## Instrument Roles

Each role: one entry. Composers look up their role to get GM instrument, register, style, velocity range.

```json
{
  "role": "bass",
  "gm": "synth-bass-1",
  "register": [28, 50],
  "style": "sub_808_distorted",
  "velocity_range": [0.7, 0.9]
}
```

Recognized roles: `drums`, `bass`, `chords`, `melody`, `lead`, `pad`, `fx`.

`gm` should match a key in `midi_types/gm_instruments.py`. Drums omit `gm` (channel 9 is implicit).

`style` is a free-form descriptor read by the relevant composer. Recognized values today:
- bass: `sustained_root`, `walking`, `sub_808_distorted`, `arpeggio`
- chords: `stab`, `sustained`, `sparse_held`
- melody: `pentatonic_sparse`, `fast_arpeggio`, `stab`

Adding a new style means adding a branch in the corresponding composer.

## Velocity Ranges

Per-role overrides (optional). If absent, composers use the role's `velocity_range`.

```json
"velocity_ranges": {
  "drums": [0.3, 0.9],
  "chords": [0.3, 0.5],
  "melody": [0.5, 0.8]
}
```

## Principles

2-4 plain-English rules that define the genre. The composers don't read
these directly, but the agent uses them in:
- The composition brief shown to the user.
- Self-evaluation after rendering.
- Decisions about style/voicing when the profile is ambiguous.

Bad principle: `"Use minor key chords."` (too vague)
Good principle: `"Hyperpop chord stabs — minor key, simple i-VI loop, sharp staccato attacks with short release."`

## References & Research Sources

```json
"references": [
  { "artist": "Nujabes", "song": "Feather", "url": "..." }
],
"research_sources": [
  "https://hooktheory.com/...",
  "Music Production Wiki - Lo-fi"
]
```

These are for posterity, not consumed by the pipeline. Future agents
loading the profile will know what they're working from.

## Sound Design (Forward-Compat)

```json
"sound_design": {
  "synth_presets": { "lead": "supersaw_otf" },
  "drum_samples": { "kick": "/path/to/808.wav" },
  "fx_chain": ["lpf", "saturator", "limiter"]
}
```

Composers ignore this today. It exists so research notes about preset
choices and sample paths can be captured now and consumed when sound-design
support lands.
