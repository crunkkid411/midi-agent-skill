# Lego Pipeline — Context-Aware Sequential Generation

> Adapted from Ace-Step DAW's "Lego Pipeline" concept: each layer of a
> composition is built knowing the cumulative result of all previous
> layers. In Ace-Step this used the 1.5B-parameter ACE-Step audio model;
> here we achieve the same coherence with deterministic Python rules.
> Proper music theory + a well-researched genre profile gets us most of
> the way there.

---

## The Order

```
structure → harmony timeline → [layer_order]

default layer_order:  chords → pad → rhythm_guitar → drums → electronic_drums → bass → synth_bass → melody → lead
metalcore:            rhythm_guitar → drums → bass → pad → melody → lead → chords
crabcore:             rhythm_guitar → drums → electronic_drums → bass → synth_bass → chords → pad → lead → melody
```

A profile sets `layer_order` when the genre's real dependency differs.
Chord-led genres (lo-fi, pop, scenecore) keep chords first. Riff-led genres
(metalcore, djent, thrash, punk) put `rhythm_guitar` first, because the riff
is the rhythm section there and the kick and bass lock to it.

| Layer | Module | Reads |
|-------|--------|-------|
| Structure | `structure.py` | profile `form` / `song_sections`, bars, seed |
| Harmony timeline | `harmony.py` | profile progressions + section `progression` / `scale` bindings |
| Chords / Pad | `chords.py` | timeline, voicing (section → role → profile), **riff attacks** (follow_riff) |
| Rhythm guitar | `riff.py` | timeline, `tuning`, riff style, motif |
| Drums / Electronic drums | `drums.py` | sections, patterns (by layer), fills, **riff attacks** (kick_follow) |
| Bass / Synth bass | `bass.py` | timeline, **drum track** (kick_locked), **rhythm guitar** (doubled) |
| Melody / Lead | `melody.py` | timeline, **drum track** (rests where busy), **rhythm guitar** (riff_octave), motif |

The orchestrator in `composers/pipeline.py` runs them, then post-processes
every track:
1. tags it with its role and instrument map (`vst`)
2. writes the section automation lanes onto it (`automation.py`)
3. splits double/quad-tracked roles into takes (`doubling.py`)
4. humanizes each take independently

Section `tempo` and `tempo_scale` bindings become the composition's tempo map.

## The Harmony Timeline

Every pitched layer reads the same `ChordSpan(start, end, roman, scale,
section)` list. Progressions are chosen per top-level section. A
micro-section that binds its own `progression` restarts the harmony there.
Otherwise micro-sections continue the parent's progression, so `verse.B`
picks up where `verse.A` left off. The span's `scale` is the section's
`scale` binding (a Phrygian verse and a minor chorus in one song) or the
key's scale.

## Section Bindings — Telling Each Section What To Do

The structure planner resolves song section → form slot → micro-section
bindings into `Section.bindings`, and every composer reads only that:

| Binding | Effect |
|---|---|
| `energy` | how hard (scales velocity), independent of `density` (how busy) |
| `drum_pattern` | pattern name, or a list played one per phrase |
| `progression` | progression name/index, or a list to choose from |
| `scale` | per-section scale |
| `riff_style` / `bass_style` / `melody_style` / `voicing` | role style override |
| `fill` | false = no fills here, true = force a section-end fill |
| `stop` | one hit on beat 1, then silence |
| `mute` | roles silent here |
| `crash` | false = no downbeat crash, true = crash on a micro downbeat |
| `kick_follow` | the kick doubles the rhythm guitar |
| `electronic_pattern` | the drum-machine layer's groove here (silent where unset) |
| `chords_style` | chords/pad style here (`trance_gate`, `follow_riff`, …) |
| `double_track` | how the takes relate here: identical / variation / octave / harmony |
| `automation` | controller lanes: ramp_up/down, hold, pump, wobble, dive, rise |
| `tempo` / `tempo_scale` | tempo change for the section |

Instruments can also set `play_in`, which mutes the role in every section not
listed. The structure planner writes those mutes into the sections.

## What Each Layer Knows About the Others

- **Rhythm guitar** writes pedal-tone, gallop, chug, open-chord, tremolo, power-strum or clean-arpeggio riffs on the tuning's lowest string. Notes carry articulations (`palm_mute`, `open`, `pinch_harmonic`). It returns its *attacks*, meaning where the low notes and accents land.
- **Drums** render per leaf section (micro-sections where present). They use the bound pattern or pattern sequence and any lanes (crash, china, ride, toms, …). They crash on section downbeats and groove changes, put fills at phrase ends, and replace the kick grid with the riff attacks under `kick_follow`.
- **Bass** follows chord roots on the bass's open string. It can sustain, walk, lock to the kick (`kick_locked`) or double the riff (`doubled`).
- **Melody/Lead** target chord tones on strong beats and back off where the drum track is busy (fills, crashes, snare stacks). They repeat as a motif and can harmonize in 3rds or double the riff an octave up.

## Meter

Every section carries `beats_per_bar` and an absolute `beat_offset` from
the planner, using the section's `meter` binding or the song `meter`.
Composers place bars with `section.bar_start(bar)` and loop over
`section.beats_per_bar`, never over a fixed 4. That is why a 3/4 breakdown
works and everything after it stays on the grid. The renderer writes
time-signature events.

## Tones

A clean intro, an acoustic break or a choir bridge is a different
instrument, not the same patch played softly. A role's `tones` and a
section's `tone` binding move those notes to their own track
(`Rhythm Guitar (clean)`, `Pad (choir)`), each with its own GM program or
VST and its own double-tracking setting.

## Double / Quad Tracking

Metal and post-hardcore rhythm guitars are recorded at least twice and
panned hard left and right. `doubling.py` turns one composed part into
takes. The section's `double_track` mode decides how they relate:
- **identical** for tight breakdowns
- **variation** for verses, where some high notes and pinch harmonics move
- **octave** for choruses, where one side plays its open chords an octave up
- **harmony** for leads, a 3rd up

Palm-muted chugs stay identical in every mode. Each take is humanized
separately, because a copy-pasted double is not a double.

## Automation

Notes alone can't express a filtered intro, a build, sidechain pumping or a
tape stop. Sections bind automation lanes and `automation.py` writes events
onto the targeted roles. The renderer sends them as CCs or pitch bend through
the instrument map's `cc_map` (General MIDI defaults otherwise).

## Motif Repetition

With `profile.motif`, bars at the same position in the motif share a seed.
The rhythm and intervallic shape repeat while the pitches follow the chord,
and later repetitions apply `last_bar`, `last_half`, `octave_drop` or
`last_root` variations. The motif key is the section's base name, so verse 2
replays verse 1's riff. That repetition gives a song an identity instead of
8 bars of new random notes.

## Layer Override Hooks

Any layer can be replaced with user-supplied content:

```python
from composers.pipeline import compose
from midi_types.music import Track, Note

user_bass = Track(
    name="Bass", role="bass",
    instrument="electric-bass-finger",
    notes=[Note("D2", "2", velocity=0.7, start=0), Note("A2", "2", velocity=0.6, start=2)],
)

composition = compose(profile=profile, bars=8, overrides={"bass": user_bass})
```

Later layers still hear an overridden layer. An overridden `rhythm_guitar`
still drives `kick_follow` and the bass's `doubled` style.

## Section-Level Regeneration

`regenerate_section(composition, profile, name, layers, seed)`:

1. Locates the named section (top-level or micro).
2. Re-runs the requested layers for just that bar range, **with the section's stored bindings**.
3. Splices the new notes into the existing composition, time-shifted.
4. Returns a new Composition. Other sections and layers are untouched.

Each layer has its own random stream (`seed:role`), so regenerating the
drums never reshuffles the riff.

## Density vs Energy

- **Density** (busy-ness): drums thin hats and cymbals below per-lane thresholds and pick the lowest-energy pattern below 0.4 when nothing is bound. Melody sits out below 0.5. Chug riffs rest more at low density.
- **Energy** (intensity): scales drum and riff velocities. Crashes need energy ≥ 0.3. Defaults to density when unset.

A breakdown is the canonical case: density 0.35 and energy 1.0 give sparse,
heavy hits.

## Rendering Through Instruments

Composers write intent: drum lanes and articulations. `skills/generate_midi.py`
maps that intent onto each track's instrument map (`instruments/*.json`):

- drum lane → kit note (Superior Drummer 3, GM, …), with fallbacks
- articulation → keyswitch held across the note (non-latching instruments) or a single keyswitch press per change (latching), and/or a velocity band (Odin III Swift Mode)
- melodic notes fold into the instrument's range, plus `octave_offset`

Any track can target any map in any genre, via profile `instruments[].vst`
or `--vst role=slug`. `--split-tracks` writes one file per track.

## Extending the Pipeline

To add a new layer:

1. Add the role to `genres/_schema.json` (instrument `role` enum).
2. Add a composer module taking `(profile, sections, timeline, ..., rng)` and returning a `Track` with `role` set.
3. Add it to `DEFAULT_LAYER_ORDER` in `pipeline.py` and dispatch it in `compose()`.

To add a new riff/bass/melody style, add a branch in that composer and
document the style name in the schema's `style` description.
