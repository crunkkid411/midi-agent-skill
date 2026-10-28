# Counterpoint Reference (default): How Parts Play Against Each Other

On the reference albums (`reference-albums.md`) "counterpoint" isn't two
independent melodies following species rules. It's **layers doing different
rhythmic jobs**: a locked riff, a pulsing bass, held pads, a vocal on top,
and whole sections that switch genre. This file maps each classical species
to its equivalent on these records. A genre profile in `genres/` overrides
anything here. The species rules themselves are in
`classical-reference/counterpoint.md` (only for classical writing).

## Contents
- Consonance on these records
- 1:1 — locked parts (first species)
- 2:1 — pulse under held chords (second species)
- 4:1 — fast layer over a slow one (third species)
- Held note against moving parts (fourth species / suspensions)
- Free line over a loop (fifth species / florid)
- Interplay between sections
- Vocal interplay
- Practical application for MIDI
- Checklist

---

## Consonance on These Records

| Classical label | Here |
|---|---|
| Perfect (unison, 5th, octave) | The backbone: power chords, bass doubling, vocal stacks |
| Imperfect (3rds, 6ths) | Synth chords, pads, harmony vocals |
| "Dissonant" minor 2nd | A riff tool (A–B♭, G–A♭, C–B, see `chord-progressions.md`) |
| "Dissonant" added 6th / 2nd | Pad and synth colour ("Paparazzi" add6, "Brown Eyes" sus2) |

---

## 1:1 — Locked Parts (First Species)

Note against note, in rhythm *and* pitch: the guitar riff, the bass an octave
below, and the kick on the same hits.

- Measured: bass roots identical to guitar roots on Attack Attack! "Renob,
  Nevada", "A for Andrew", the "Sexual Man Chocolate" break, and ISMFOF
  "Things That Rhyme with Orange".
- Breakdown kick plays the chug rhythm (`rhythm-patterns.md`, family 6).
- Joey Sturgis: everything to a click, every note on the grid.

**Motion:** parallel (all three move together). That's the point; it makes
one huge instrument. Use bass `doubled` + drum `kick_follow`, riff first in
`layer_order`.

---

## 2:1 — Pulse Under Held Chords (Second Species)

A moving layer at twice (or more) the rate of a held layer.

- Gaga "Just Dance": bass root 8ths (8 notes per bar) and a two-note synth
  ostinato (E–A, 8ths) under half-note pads (Songsterr).
- Pop-punk: root-8th bass under power chords held or strummed (Sueco "SOS").

Use bass `root_eighths` or `octave` under chords `sparse_held` / pad
`sustained`. The moving layer stays on the chord root or its octave, the same
job passing tones did in classical writing, but with repetition instead of
passing motion.

---

## 4:1 — Fast Layer over a Slow One (Third Species)

16th notes over a quarter-note or half-note frame.

- 16th hats over a four-on-the-floor kick ("Blackout", `rhythm-patterns.md`).
- 16th double kick under 8th snare (ISMFOF "Beauty…" intro).
- Gated synth chords (`trance_gate`) chopping a held chord into 16ths over the
  kick.
- Tremolo-picked guitar (`tremolo`) over a half-time drum bar.

The fast layer repeats one pitch or one chord. It adds motion without new
harmony.

---

## Held Note Against Moving Parts (Fourth Species / Suspensions)

Classical suspensions hold a note over a chord change, then resolve it. Here
the held note **doesn't** resolve; that's the tension.

- **Pedal bass under moving chords:** Attack Attack! "Sexual Man Chocolate"
  verse (bass B–B♭–A–B♭ under a moving guitar), 6arelyhuman "CR4CK HOUSE"
  (one D all song).
- **sus2 instead of a triad:** "Brown Eyes" chorus `I – vi(sus2) – IV – V7`.
- **Pad holding a common tone** across a loop change. In Am–F–C–G, the note
  C sits in Am, F and C. Hold it for three chords and step to B (or D) for
  the G.

---

## Free Line over a Loop (Fifth Species / Florid)

The vocal (or the lead synth) is the only part that moves freely, over a loop
that repeats.

Measured vocal behaviour (`modes-scales.md`):
- mostly stepwise (53–78% steps), few big leaps;
- on chord tones at strong beats ("Poker Face": 75% chord tones, 96% in the
  key);
- range rises from verse to chorus ("Poker Face" verse F♯3–G♯4, chorus to B4;
  "Paparazzi" verse to A♯4, chorus to E♭5);
- rap verses sit in a 3rd–4th and move in 16ths (Sueco "Hate You Too").

RedOne, producer of *The Fame*: "I am a melody person." The vocal hook is
the focus; the loop stays simple so the hook stands out.

---

## Interplay Between Sections

On these records the biggest contrast is **between sections**, not between
simultaneous lines. The tabs' own section names show it:

| Song | Section order (Songsterr markers) |
|---|---|
| Attack Attack! "Smokahontas" | Intro > Verse > Chorus > Techno Break > Dub Step Break > Post-Dub Step > Pre-Breakdown > Breakdown > Outro |
| Attack Attack! "Fumbles O'Brian" | "Hip Hop Intro" … "Techno Pop Break" |
| Lady Gaga "Poker Face" | Includes a post-chorus |

Electronicore is described as "dynamic transitions from soft electronica
ballads to intense metalcore passages" (Wikipedia). Sueco pairs "rapped
verses with guitar-heavy choruses" (Prelude Press).

**Rule:** plan contrast as a sequence of layer sets. For example: synth +
electronic drums (verse) → full band (chorus) → club drums alone (techno
break) → guitars + kick only (breakdown). Use section `roles`, `energy` and
per-section `drum_pattern`, and the `dance-break` / `breakdown` section
types.

---

## Vocal Interplay

| Technique | Evidence | Profile |
|---|---|---|
| Stacked unison doubles on the chorus | Sturgis: "I double everything, triple some things and quadruple others" | `double_track` `identical`, `takes: 2` |
| Low screams under a centre scream | Sturgis: "four low screams that combine to support one centre scream" | `double_track` `octave`, `takes: 4` |
| Clean vs. scream sections | Breathe Carolina's reissue: "less on scream vocals and more emphasis on clean vocals"; electronicore = "auto-tuned singing, and screamed vocals" | Separate vocal sections; scream sections have no pitch line to compose |
| Rap verse → sung chorus | Sueco (Prelude Press); Lil Lotus "emo-rap" verses (BrooklynVegan) | `vocal_line` with dense 16ths in the verse, wider sung range in the chorus |
| Harmony vocal a 3rd above | Not measured on these records | `double_track` `harmony` — use only if asked |

**Not measured:** a synth lead answering the vocal in its gaps (call and
response) is common in pop arranging, but these transcriptions don't show it
clearly. Treat it as an option, not a default.

---

## Practical Application for MIDI

### Two-layer texture (minimum)
```
Riff (rhythm_guitar):  B1 B1 B1 C2 | B1 B1 B1 Bb1   (8ths, power chords)
Bass:                  B0 B0 B0 C1 | B0 B0 B0 Bb0   (same rhythm, octave down)
```
Locked 1:1. Add a kick on every riff hit for a breakdown.

### Club texture
```
Kick:        x . . . x . . . x . . . x . . .
Synth bass:  root 8ths
Pad:         half or whole notes, full chord with its 3rd
Vocal:       stepwise, chord tones on 1 and 3
```

### Band + synth texture (electronicore)
```
Guitars L/R: power chords (no 3rd)
Bass:        doubles the riff
Pad / synth: full chord with its 3rd, above B2
Vocal:       top line, at least an octave above the guitar root
```

---

## Checklist

- [ ] Heavy parts locked 1:1 (riff + bass + kick)
- [ ] One fast layer (hats, gate, tremolo) over slow chords, not three
- [ ] Pedal or held note under moving parts where you want tension
- [ ] The vocal is the only free-moving melody in the section
- [ ] Sections differ by which layers play, not just by new chords
- [ ] Scream sections stacked; clean chorus doubled
