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

Granularity fields every profile should carry: named progressions, ≥4 drum
patterns (any lanes: crash, china, ride, toms, …), section/micro bindings
(`drum_pattern`, `progression`, `scale`, `*_style`, `mute`, `stop`, `energy`),
`form`, `fills`, `motif`, `humanize`, section `automation`, `play_in` for
sectional parts; plus `tuning`, `layer_order` and `double_track` modes for
guitar-driven genres. See `workflows/genre-profile-spec.md`.

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

## Sound design and VSTs

`sound_design` records synth presets, sample paths, FX chains and the kits
or VSTs the profile was tuned for. Which instrument a role renders on is
`instruments[].vst` (a map in `instruments/`). Any VST can be used with any
genre via `--vst role=slug`.

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
- `lo-fi-hip-hop.json` — Nujabes/J Dilla-style boom-bap chillhop. Swung rim/shaker variants, hats-only drops under a filter dip, half-time bridge, motif-based vibraphone, low-passed intro/outro. Beatmaker drums, Vital pad.
- `post-hardcore.json` — 2000s post-hardcore / screamo, modelled on Underoath's *They're Only Chasing Safety*. Aliases: screamo, underoath, theyre-only-chasing-safety.
  - Vocals: screamed verses vs a clean, drummer-sung chorus (`vocal_line`).
  - Guitars: guitar 1 plays palm-muted 8ths and big open chords in drop D; guitar 2 plays a high ostinato with feedback bends.
  - Harmony and arrangement: emo-pop minor loops from the album's songs, a maj7 bridge with a choir tone, a 3/4 breakdown, an acoustic break, electronica beats and keys.
- `crabcore.json` — Attack Attack!/I See Stars-style electronicore in drop C#.
  - Guitars: double-tracked, with section-dependent take modes.
  - Synths: trance-gated, pumping synths that follow the breakdown chugs; a wobble-bass breakdown; a pitch-riser build; a tape-stop stop.
  - Arrangement: a euro-dance break on a Beatmaker drum machine at a lower tempo, and a slower second breakdown.
  - VSTs: SD3, Odin III, Beatmaker, Serum 2, Vital.
- `metalcore.json` — Killswitch Engage / August Burns Red / Parkway Drive-style metalcore. Drop C, riff-first layer order, pedal-tone/gallop/chug riffs. Double-kick, skank, gallop and blast verses, a stop before a kick-follows-riff breakdown, and a final chorus. Targets Superior Drummer 3 and Odin III, double-tracked: identical breakdowns, varied verses, octave-split choruses.
