# Genre Profile Specification

> Quick reference for what goes into a `genres/<slug>.json`. Full schema
> in `genres/_schema.json`. `genres/metalcore.json` uses every field and is
> the most complete example.

---

## Required Fields

| Field | Type | Notes |
|-------|------|-------|
| `name` | string | Lowercase-hyphenated. Must match filename. |
| `tempo_range` | `[min, max]` | BPM. Composers use the midpoint by default. |
| `default_key` | string | "D Dorian", "C minor", "F# major" |
| `scales` | array | Mode names. See "Scales" below. |
| `chord_progressions` | array | Named Roman-numeral progressions. |
| `drum_patterns` | array | Grid strings, any lanes. See "Drum Grid". |
| `song_sections` | array | Sections + micro-sections + bindings. The nuance layer. |
| `instruments` | array | Per-role instrument config (+ optional `vst`). |
| `principles` | array | 2-4 plain-English rules. |

## Granularity Fields (every profile should have these)

| Field | What it controls |
|-------|------------------|
| `form` | Song form template, rendered with `--form` |
| `fills` | Phrase-end drum fills |
| `motif` | Repetition with variation instead of re-rolling every bar |
| section bindings | Per-section drum pattern, progression, scale, styles, mutes, stops |
| `layer_order` | Which layer drives which (riff-first vs chord-first) |
| `tuning` | Guitar/bass open strings (riff and bass pedal notes) |
| `humanize` | Timing / velocity jitter |
| section `automation` | Filter sweeps, sidechain pumps, wobbles, pitch risers/dives |
| `instruments[].double_track` + section `double_track` | How double/quad-tracked takes relate per section |
| `instruments[].play_in` | Which sections a part plays in |
| section `tempo` / `tempo_scale` | Tempo changes |

## Scales

Recognized scale names (case-insensitive, underscore for spaces):

`major`, `ionian`, `minor`, `aeolian`, `harmonic_minor`, `melodic_minor`,
`dorian`, `phrygian`, `lydian`, `mixolydian`, `locrian`,
`minor_pentatonic`, `major_pentatonic`, `blues`.

A section's `scale` binding overrides the key's scale for that section.
Roman numerals are resolved against the section's scale. In a Phrygian
section, `II` is the flat-2 major chord.

## Chord Notation (Roman Numerals)

- **Case** = quality: `I` major, `i` minor.
- **Suffix** = quality marker: `°` or `dim` = diminished, `ø` = half-diminished, `+` or `aug` = augmented.
- **Numbers** = extensions: `7`, `9`, `11`, `13`. Implicit lower extensions are added (a 9 chord includes the 7).
- **Accidentals** = chromatic alteration: `b9`, `#11`, `b5`. Prefix `b`/`#` on the Roman numeral itself shifts the root chromatically: `bVI` = chord on flat-6.
- **Examples**: `i9`, `iv9`, `iiø7`, `V7b9`, `bVI`, `#iv°7`.

Give every progression a `name`; sections bind to it.

```json
{ "name": "chorus-hook", "roman": ["i", "VI", "III", "VII"], "bars_per_chord": 1.0, "weight": 1.0 }
```

## Voicing

`voicing_style` (profile), `instruments[].voicing` (role) or the section
`voicing` binding: `triads`, `extended_7th_9th`, `power_chords` (root–5th–
octave, no 3rd, which keeps distorted guitars clear), `open_voicings` (spread,
3rd up an octave), `stab`, `sustained`. Chords are always voiced in root
position.

## Drum Grid

Each lane is a string. Length = resolution: 8 (8ths), 12 (8th triplets),
16 (16ths), 24 (16th triplets), 32 (32nds — hat rolls).

Characters: `x` hit, `.` rest, `g` ghost (low velocity), `a` accent.

`"layer": "electronic"` marks a pattern for the `electronic_drums` role (a
drum machine such as UJAM Beatmaker layered with the kit, or carrying a
dance break). That layer plays only in sections that bind
`electronic_pattern`, and it gets no automatic crashes or fills.

Lanes: `kick`, `snare`, `snare_rimshot`, `snare_sidestick`, `clap`, `hh`,
`hh_tight`, `hh_pedal`, `oh`, `crash`, `crash_2`, `china`, `ride`,
`ride_bell`, `ride_edge`, `splash`, `tom_high`, `tom_mid`, `tom_low`,
`floor_tom`, `cowbell`, `tambourine`, `shaker`, `perc`. Lanes are
instrument-agnostic. The instrument map turns them into kit notes.

```json
{
  "name": "halftime-breakdown",
  "kick_follow": true,
  "kick":  "x..x..x...x..x..",
  "snare": "........x.......",
  "china": "x...x...x...x...",
  "swing": 0.5,
  "ghost_density": 0.0,
  "feel": "halftime"
}
```

- `swing`: 0.5 = straight, 0.66 = hard swing. Applies to offbeat 16ths.
- `ghost_density`: 0.0-1.0 probability of ghost notes on rests (snare, hats).
- `kick_follow`: the kick doubles the rhythm guitar's low attacks (chugs) instead of its grid. Needs `rhythm_guitar` before `drums` in `layer_order`.

Automatic drum behaviour: crash + kick on every top-level section downbeat
and whenever the groove changes inside a section; per-lane thinning at low
density.

## Fills

```json
"fills": {
  "phrase_bars": 4,
  "probability": 0.5,
  "types": ["tom_run", "kick_snare_burst", "triplet_toms", "snare_roll"],
  "beats": [1.0, 2.0],
  "section_end_probability": 0.9
}
```

Types: `snare_roll`, `tom_run`, `triplet_toms`, `kick_snare_burst`,
`snare_build` (8ths → 16ths), `open_hat`, `drop_out` (drums drop out),
`stop_hit`. A fill replaces the last `beats` of a phrase. Section ends use
the longest option.

## Song Sections, Micro-Sections and Bindings

Names: `intro`, `verse`, `pre-chorus`, `chorus`, `post-chorus`, `drop`,
`build`, `bridge`, `break`, `breakdown`, `pre-breakdown`, `stop`, `solo`,
`interlude`, `dance-break`, `outro`, `loop`.

```json
{
  "name": "breakdown", "bars": 8, "density": 0.35, "energy": 1.0,
  "scale": "phrygian", "riff_style": "chug", "kick_follow": true,
  "progression": ["breakdown-pedal", "breakdown-b2"],
  "drum_pattern": "halftime-breakdown",
  "mute": ["lead", "pad"],
  "micro": [
    { "name": "chug", "bars": 4, "density": 0.35 },
    { "name": "drop", "bars": 4, "density": 0.4, "drum_pattern": "breakdown-heavy" }
  ]
}
```

| Binding | Meaning |
|---|---|
| `density` | 0-1 how busy (thins hats, gates melody, chug rests) |
| `energy` | 0-1 how hard (velocity). Defaults to density. |
| `drum_pattern` | name, or a list played one per phrase (`fills.phrase_bars`) |
| `progression` | name/index, or a list to choose from |
| `scale` | per-section scale |
| `riff_style`, `bass_style`, `melody_style`, `voicing` | role style override here |
| `fill` | false disables fills here; true forces a section-end fill |
| `stop` | one hit on beat 1, then silence |
| `mute` | roles silent here |
| `crash` | false suppresses the downbeat crash; true forces one on a micro downbeat |
| `kick_follow` | force kick-follows-riff |
| `electronic_pattern` | electronic drum layer pattern(s) here (silent where unset) |
| `chords_style` | chords/pad style here; `riff_style`, `bass_style`, `melody_style` and `chords_style` may also be a `{role: style}` dict |
| `double_track` | `identical`, `variation`, `octave` or `harmony` (how the takes relate here) |
| `automation` | controller lanes over the section (see "Automation") |
| `tempo` / `tempo_scale` | absolute BPM, or a multiple of the song tempo; the song tempo returns after |

Micro-sections inherit the parent's bindings and override them. They are
addressable by dotted name (`verse2.B`, `breakdown.drop`). Drums, riffs and
melodies render per micro-section.

## Automation

```json
"automation": [
  { "param": "filter", "shape": "ramp_up", "roles": ["pad", "chords"], "from": 0.2, "to": 1.0 },
  { "param": "volume", "shape": "pump",    "roles": ["pad"], "depth": 0.4, "rate": "4" },
  { "param": "filter", "shape": "wobble",  "roles": ["synth_bass"], "rate": "8", "from": 0.15, "to": 0.9 },
  { "param": "pitch",  "shape": "dive",    "roles": ["chords"], "semitones": 12, "beats": 3 }
]
```

Params: `filter`, `resonance`, `volume`, `expression`, `mod_wheel`,
`pitch`, `macro1`–`macro8`. Shapes:
- `ramp_up` / `ramp_down`: builds, filtered intros and outros
- `hold`: a constant `value`, optional `reset`
- `pump`: sidechain ducking on every `rate` hit
- `wobble`: LFO at `rate` (`4`, `8`, `8t`, `16`)
- `dive` / `rise`: pitch tape-stop and riser; `semitones` is converted with the synth's bend range

The instrument map turns params into CC numbers. General MIDI defaults:
filter 74, volume 7, expression 11, mod wheel 1.

## Form

```json
"form": [
  { "section": "intro" },
  { "section": "verse" },
  { "section": "pre-chorus", "probability": 0.8 },
  { "section": "chorus" },
  { "section": "verse", "bars": 8 },
  { "section": "stop" },
  { "section": "breakdown" },
  { "section": "chorus", "energy": 1.0, "drum_pattern": "chorus-double-kick" },
  { "section": "outro" }
]
```

`--form` renders it whole. `--bars N` (N > 8) walks it and truncates.
`probability < 1` makes a slot optional, rolled with the seed. Any binding
on a form slot applies to that occurrence only. Repeats are numbered:
`verse`, `verse2`, `chorus3`.

## Motif

```json
"motif": { "bars": 4, "variations": ["exact", "last_bar", "octave_drop", "last_half"],
           "roles": ["rhythm_guitar", "lead"] }
```

The first `bars` are the idea. Later repetitions cycle through the
variations: `exact`, `last_bar` (new final bar), `last_half`,
`octave_drop`, `last_root` (rhythm guitar: move the last root). Repeated
sections reuse the same motif.

## Layer Order and Tuning

```json
"layer_order": ["rhythm_guitar", "drums", "bass", "pad", "melody", "lead", "chords"],
"tuning": "drop_c"
```

Default order: chords, pad, rhythm_guitar, drums, bass, melody, lead.
Tunings: `standard_e`, `drop_d`, `standard_d`, `drop_c`, `standard_c`,
`drop_b`, `drop_a`, `seven_b`, `seven_drop_a`, `eight_f#`, `eight_drop_e`,
`double_drop_c`, a list of open-string MIDI pitches, or `{name, strings}`.
The riff's pedal note is the chord root on the lowest string. The bass
sits an octave below.

## Instrument Roles

```json
{
  "role": "rhythm_guitar",
  "gm": "distortion-guitar",
  "vst": "odin-iii",
  "register": [36, 79],
  "style": "pedal_tone",
  "velocity_range": [0.55, 0.95],
  "double_track": true,
  "articulations": { "pinch_harmonic_rate": 0.04 }
}
```

Roles: `drums`, `electronic_drums`, `rhythm_guitar`, `bass`, `synth_bass`, `chords`, `pad`, `melody`, `lead`, `fx`.

| Role | Styles |
|---|---|
| rhythm_guitar | `pedal_tone`, `gallop`, `chug`, `open_chords`, `tremolo`, `power_strum`, `clean_arp` |
| bass / synth_bass | `sustained_root`, `walking`, `kick_locked` (aliases `sub_808*`), `doubled`, `root_eighths`, `octave`, `arpeggio`, `wobble` |
| chords / pad | `stab`, `sustained`, `sparse_held`, `strum8`, `pulse4`, `trance_gate`, `follow_riff` |
| melody / lead | `pentatonic_sparse`, `fast_arpeggio`, `stab`, `sustained_hook`, `harmonized_thirds`, `riff_octave`, `tremolo_lead` |

- `gm` must match a key in `midi_types/gm_instruments.py` (General MIDI fallback / preview).
- `vst` names an instrument map in `instruments/`. The CLI's `--vst role=slug` overrides it for any genre.
- `double_track`: multi-take recording. `true`, a mode, or `{"takes": 2|4, "mode": "identical|variation|octave|harmony", "variation_rate": 0.2}`.
  - **identical**: same notes (tight breakdowns).
  - **variation**: the right take changes some melodic notes and pinch harmonics.
  - **octave**: the right take plays its open notes an octave up (choruses).
  - **harmony**: the right take plays a diatonic 3rd up.

  Palm-muted chugs stay identical in every mode. The section binding `double_track` overrides the mode. `takes: 4` adds an inner L2/R2 pair. Each take is humanized on its own.
- `play_in`: section names the role plays in. It is muted everywhere else (a trance lead only in choruses, a clean vocal only in the chorus).
- `articulations`:
  - `pinch_harmonic_rate` (rhythm guitar)
  - `glide_rate` (808 / synth-bass octave slides; glides overlap on mono synths)
  - `gate` (16-step string for `trance_gate`)

## Velocity Ranges and Humanize

```json
"velocity_ranges": { "drums": [0.62, 0.98], "rhythm_guitar": [0.55, 0.95] },
"humanize": { "timing": 0.006, "velocity": 0.04 }
```

`humanize.timing` is in beats (0.006 ≈ 2.4 ms at 150 BPM). Lo-fi uses
~0.03 for a lazy feel. Drums get half the timing jitter.

## Principles

2-4 plain-English rules that define the genre. The composers don't read
these directly, but the agent uses them in the composition brief, in
self-evaluation, and when the profile is ambiguous.

Bad principle: `"Use minor key chords."` (too vague)
Good principle: `"Half-time breakdown as the climax, set up by a stop — kick doubles the chugs, snare on 3, china on quarters."`

## References & Research Sources

```json
"references": [ { "artist": "Parkway Drive", "song": "Carrion" } ],
"research_sources": [ "https://..." ]
```

## Sound Design

`sound_design` records preset, sample and FX-chain notes (and which VSTs
and kits the profile was tuned for). Note mapping and articulations live
in the instrument maps, not here.

## Instrument Maps (instruments/*.json)

Genre-independent. See `instruments/README.md`. Composers write drum lanes
and articulations. The map converts them to kit notes, keyswitches and
velocity bands at render time.
