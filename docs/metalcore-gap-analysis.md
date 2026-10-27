# Metalcore: midi-agent-skill vs. metalerator

`genres/metalcore.json` was added the standard way: web research, the
schema, and a test compose. This doc compares the result with
[metalerator](https://github.com/crunkkid411/metalerator), a hand-coded
metalcore generator, and lists what the profile + Lego Pipeline approach
cannot express yet.

**Short version:** the pipeline assumes a song is a **chord progression
with parts layered over it**. Metalcore is built on a **rhythm-guitar riff
that everything else locks to**. Most of the gaps below come from that one
difference.

---

## Status: implemented

All nine gaps below are now implemented. The analysis is kept as the record of why.

| # | Gap | Fix |
|---|---|---|
| 1 | No rhythm guitar / riff | `composers/riff.py` (pedal_tone, gallop, chug, open_chords, tremolo, power_strum, clean_arp). `voicing_style: power_chords` works. Chords are voiced in root position. |
| 2 | Layer order backwards | profile `layer_order`; drum `kick_follow` (pattern or section); bass `doubled` / `kick_locked`; `bass.py` precedence bug fixed |
| 3 | Breakdowns not first-class | `breakdown`, `stop`, `pre-breakdown`, … section names; per-section/micro `drum_pattern`, `progression`, `energy` (separate from density), `mute`, `stop` |
| 4 | Drum vocabulary | any lane (crash, china, ride, ride_bell, splash, toms, …); pattern sequences per phrase; crash on section and groove changes; `fills` (8 fill types) |
| 5 | No riff identity | `motif` (repeat with exact / last_bar / last_half / octave_drop / last_root); verse 2 replays verse 1's riff; melody now really reads the drum track |
| 6 | Rigid form | profile `form` with optional slots, per-slot overrides and numbering; `--form`; micro-sections no longer squeezed into 1-bar slivers |
| 7 | Tuning / register | `tuning` (drop_c, drop_b, 7/8-string, …): riff pedal on the open string, bass an octave below |
| 8 | Per-section scale | section `scale` binding, resolved into the shared harmony timeline |
| 9 | Articulation / output | `instruments/` maps (Superior Drummer 3, Odin III, GM, template); note `articulation` + `drum` lanes; held/latching keyswitches, velocity bands; `--vst role=slug` for any genre; `--split-tracks` |

Side bugs fixed as well: lo-fi's `fingered-bass` (rendered as piano) is now
`electric-bass-finger`, and notes longer than a whole note were truncated
to 4 beats. Covered by `tests/test_pipeline.py`.

---

## What the test render actually produced (before the fixes)

`compose_from_profile.py --genres metalcore --bars 40 --seed 1`:

| Layer | What came out | What metalcore needs |
|---|---|---|
| Chords (distortion guitar) | Full minor triads (C–Eb–G) on random 8th stabs, sometimes inverted (C4 above Eb3) | Root–5th–octave power chords, palm-muted 8th/16th chugs on an open low string |
| Drums | One 16-step pattern per section, picked at random by weight. The intro happened to get the breakdown pattern. | Pattern changes inside a section, fills at phrase ends, crash on section starts, china/ride as the main cymbal |
| Bass | Whole-note roots under a 16th-note double kick | Bass doubles the guitar riff |
| Melody | Random walk that starts over on each chord, with no repeated motif | Lead derived from the riff, or harmonized 3rds. Usually absent in verses |
| Form | intro → verse → pre → chorus → break → verse2 (cut to 4 bars) → outro | …chorus → pre-breakdown → **breakdown** → **final chorus** → outro |

---

## Gaps, ranked by impact

### 1. No rhythm guitar or riff concept (critical)
- metalerator: `rhythm_guitar/verse/pedal_tone_riff.py` builds pedal-tone
  riffs. The open root plays on 8ths, with scale "high notes" mixed in and
  never more than 2 high notes in a row. The chorus uses root+5+8 power
  chords (`chorus.py`).
- midi-agent-skill: the only role for harmony is `chords`. `chords.py`
  **ignores `voicing_style`**, so the schema's `power_chords` does nothing.
  It always emits full triads, and with distortion the 3rds turn to mud.
- **Fix:** add a `rhythm_guitar` role (or a `riff` style) that generates
  pedal-tone, gallop and chug riffs, and make `voicing_style:
  power_chords` emit `[root, +7, +12]`.

### 2. The layer order is backwards for this genre (critical)
- metalerator: **guitar → drums → bass**. In
  `breakdown/default_melodic.py`, `create_kick_pattern()` puts a kick on
  every low guitar note, and `Bass.copy_guitar()` doubles the riff.
- midi-agent-skill: chords → drums → bass. Drums never read the chord or
  riff rhythm, and bass only locks to the kick for `*808*` styles.
- **Fix:** let a profile set `layer_order`. Also add a drum mode where
  the kick follows the riff, and a `doubled` bass style that copies the
  riff down an octave.

### 3. Breakdowns are not first-class (critical)
- The section enum has no `breakdown`. The profile has to use `break`
  plus `density < 0.4` to trigger the low-energy drum pattern. That ties
  "sparse" to "heavy", and the two mean opposite things in this genre.
- Drum patterns are picked only for **top-level** sections. Micro-sections
  like `break.drop` can never get different drums.
- Chord progressions are picked at random for every section, so the
  breakdown's `i` pedal can land in the chorus and the reverse.
- metalerator has a dedicated breakdown generator. It mixes syncopated
  16th, 8th, quarter and half durations, rules out lone 16ths, and makes
  rests more likely the longer the root repeats. Its chug rhythm *is* the
  kick pattern.
- **Fix:** add `breakdown` to the enum. Let sections bind to patterns and
  progressions (`"drum_pattern": "halftime-breakdown"`,
  `"progression": 3`). Add separate `energy` and `density` fields.

### 4. Drum vocabulary is too small
| metalerator has | midi-agent-skill |
|---|---|
| Kick modes: double bass, 8ths with bursts before the snare, "trr-trr", 4- and 2-on-the-floor | Fixed grid per pattern |
| Snare modes: step, half-step, double-time, blast1, blast2 (triplet blast) | Fixed grid |
| Main cymbal chosen per groove (china, ride, ride bell, open hat), crash on every pattern change, random splashes | `DRUM_LANES` = kick/snare/clap/hh/oh/perc. **No crash, china, ride or toms**, even though `GM_DRUM` maps them |
| Fills (fast, triplet, snare roll, tss) at phrase ends, ghosts before and after the snare | No fills. Ghost notes are random |
| Groove changes every 4 bars inside a section | One pattern per section |
- **Fix:** add `crash`, `china`, `ride` and `tom_*` lanes, let a section
  list several patterns (`"pattern_sequence"`), and add a fill generator
  that runs on the last bar of each phrase.

### 5. No repetition with variation, so no riff identity
- metalerator writes a 4-bar idea and repeats it 4 times. It changes bar
  4 of the 2nd repeat and bars 3–4 of the 4th, drops the 3rd repeat an
  octave, and sometimes changes the last root. That matches how real riffs
  are arranged.
- midi-agent-skill rolls every note again on every pass of the loop. The
  `melody.py` docstring says it "rests over busy drum moments", but the
  `drum_track` argument is never used.
- **Fix:** add a `motif` layer: generate N bars once, then repeat them
  with variation rules set in the profile (`"repeat_variation": ["last_bar",
  "octave_drop"]`).

### 6. Song form is too rigid
- metalerator's form includes optional sections: a pre-chorus that may be
  normal, a "rest" stop or skipped, a post-chorus, a shorter verse 2, a
  pre-breakdown, and a **final chorus after the breakdown**. It also has
  stops: one hit, then silence.
- `structure.py` loops through the middle sections in order and cuts the
  last one short (`verse2` came out 4 bars, with its micro-sections
  squeezed to 2+2). It cannot express optional, conditional or one-off
  sections, or silence.
- **Fix:** let a profile give an explicit `form` template with optional
  and probability flags, and add a `stop` section type.

### 7. Tuning and register
- metalerator picks a random riff root between MIDI 29 and 38 (F1 to D2,
  the range of drop tunings) and folds every part around it.
- midi-agent-skill has no idea of tuning or an open low string. The key
  is just a pitch class, so the riff root can land above the lowest
  string. `midi_in_register` folds each chord tone separately, which
  produces inversions.
- **Fix:** add a `tuning` field (`"drop_c"` → lowest string 36) that the
  riff and bass composers use as their pedal note.

### 8. Per-section scale
- metalerator chooses a scale per section (verse may be Phrygian, chorus
  natural minor) on a shared root. midi-agent-skill uses one scale for the
  whole song.

### 9. Articulation and output target
- metalerator writes palm-mute keyswitches (notes 12/14 for Ample Metal
  Hellrazer), slides (101/102) and dead notes (94), maps drums to
  Superior Drummer 3, and writes **one MIDI file per instrument** to drop
  into an FL Studio template.
- midi-agent-skill writes one GM file. Played through a GM SoundFont,
  "distortion-guitar" never sounds like metalcore, and `sound_design` is
  ignored.
- **Fix:** add an `articulations` lane for keyswitches, a per-track
  export option, and drum maps keyed by `sound_design.drum_map`
  (`"gm"`, `"sd3"`, `"ggd"`).

---

## What midi-agent-skill already does better

- **Data, not code.** Genres are JSON files, so adding a genre doesn't
  mean writing 2,000 lines of Python the way metalerator does.
- **Reproducible.** It uses `--seed`. metalerator uses the global
  `random` with no seed, so you can't recreate a song you liked.
- **Addressable and editable.** You can regenerate `verse2.B` drums on
  their own or override a layer. metalerator is all-or-nothing.
- **Blending, tempo and key control.** metalerator doesn't set a tempo
  (the DAW decides) and has no CLI options.

## Suggested order of work
1. Make `voicing_style: power_chords` work, plus a `riff`/`rhythm_guitar` style with palm-mute chugs.
2. Let a profile set `layer_order` and add riff-locked kick and bass styles.
3. Add `breakdown` to the section enum and bind sections to drum patterns and progressions.
4. Add crash, china and ride lanes and phrase-end fills.
5. Add a motif layer that repeats with variation.
6. Add form templates, `tuning`, per-track export and keyswitch articulations.

Items 1–3 alone would make the metalcore profile render as metalcore.

## Unrelated bugs found along the way
- `bass.py`: `if drum_track and "808" in style or "kick" in style` is
  missing parentheses. The two branches are also inconsistent: a style
  containing "kick" collects kick times, but only `808`/`sub` styles
  read them.
- `lo-fi-hip-hop.json` uses `"gm": "fingered-bass"`, which isn't a GM key
  or alias. `resolve_instrument` falls back to **piano** (program 0).
  The slug should be `electric-bass-finger`.
