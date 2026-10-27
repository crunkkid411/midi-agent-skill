# Genres — Cached Genre Profiles

This directory holds the cacheable artifacts produced by the music-theory
research workflow. Each `*.json` file captures everything the deterministic
composers in `composers/` need to render a song in that style.

## Why cache?

A musician doesn't re-research the same genre every time they sit down to
write. When the agent researches a genre once and writes it here, every
subsequent composition in that style is deterministic and instant — no AI
calls, no rate limits.

## How profiles are produced

1. Agent reads `workflows/music-theory-research.md` (the 5-phase research process).
2. Agent does 3-6 web searches for the genre (chord progressions, drum
   programming, song structure, references, BPM ranges).
3. Agent fills out the schema defined in `_schema.json`.
4. Agent saves the result here as `<slug>.json`.
5. Composers in `composers/` consume the profile to build deterministic MIDI.

## Naming

- File name = profile `name` field. Lowercase-hyphenated.
- Use `aliases` for variants ("lofi", "lo-fi", "chillhop").
- Resolution by alias is handled by `midi_types.genre_profile.find_profile()`.

## Schema

See `_schema.json` for the full JSON schema. Required fields:

- `name`, `tempo_range`, `default_key`, `scales`
- `chord_progressions` (with Roman numerals like `i9`, `V7b9`, `bVI`)
- `drum_patterns` (16-step grid strings: `x` hit, `.` rest, `g` ghost)
- `song_sections` (with optional `micro` sub-blocks for nuance)
- `instruments` (per-role: drums, bass, chords, melody, lead, pad, fx)
- `principles` (2-4 distilled human-readable rules — drives composer behavior
  AND the agent's self-evaluation step)

## Micro-sections — the nuance layer

The `micro` array under each section is what makes addressing things like
"add a drum break in the B section of verse 2" tractable. A micro-section
is a sub-block with its own density and note, addressable by dotted name:
`verse.B`, `chorus.drop`, `verse2.A`.

```json
{
  "name": "verse",
  "bars": 8,
  "micro": [
    { "name": "A", "bars": 4, "density": 0.6, "note": "Sparse, build" },
    { "name": "B", "bars": 4, "density": 0.85, "note": "Full kit kicks in" }
  ]
}
```

When the user asks for "a drum break in verse 2 B", the composer can target
exactly those 4 bars without disturbing anything else.

## Sound design (forward-compat)

The `sound_design` field is reserved for future use: synth presets, paths to
user-provided drum samples, FX chains. Composers ignore it today but writers
should fill it in when known so the data is captured.

```json
"sound_design": {
  "synth_presets": { "lead": "supersaw_otf_compressed" },
  "drum_samples": { "kick": "/path/to/808_kick.wav" },
  "fx_chain": ["lpf", "saturator", "limiter"]
}
```

## Adding a new profile

1. Run the research workflow in `workflows/music-theory-research.md`.
2. Copy `lo-fi-hip-hop.json` as a starting template.
3. Fill in fields. Verify against `_schema.json`.
4. Sanity-check by running:
   ```
   python skills/compose_from_profile.py --genres <slug> --bars 8 --title test
   ```
5. Listen to the output, refine the profile, repeat.

## Seeded profiles

- `scenecore.json` — ported from the Ace-Step DAW reference plan (140 BPM, A minor, trap/breakcore drums, hyperpop stabs).
- `lo-fi-hip-hop.json` — Nujabes/J Dilla-style boom-bap chillhop default.
- `metalcore.json` — Killswitch Engage / August Burns Red-style metalcore (130-170 BPM, C minor / Phrygian, double-kick verses, clean chorus, half-time breakdown).
