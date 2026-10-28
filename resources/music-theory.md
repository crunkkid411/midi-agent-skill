# Music Theory Reference (default)

The basics, set to the sound of the reference albums
(`reference-albums.md`): crunkcore, electronicore, electropop, pop-punk,
post-hardcore, electroclash and hyperpop. A genre profile in `genres/`
overrides anything here. The classical version is in
`classical-reference/music-theory.md`.

## Contents
- Keys these records use
- Chord construction (including power chords and add6)
- Diatonic chords in a minor key
- How sections end (instead of cadences)
- Dissonance: what's a feature here, what's a mistake
- Chord voicings for guitar, synth and pad

---

## Keys These Records Use

Chart-checked keys (Hooktheory / Ultimate Guitar):

| Minor | Songs |
|---|---|
| G minor | "Don't Trust Me", "The Fame" (Dorian) |
| D minor | "Starstrukk" |
| D♯ / E♭ minor | "Blackout" |
| C minor | "Paralyzed", "Paparazzi" verse, "I Like It Rough" (Dorian) |
| C♯ minor | "Just Dance" |
| B minor | "LoveGame", "Beautiful, Dirty, Rich" |
| G♯ minor | "Poker Face" verse, "Money Honey", "Starstruck" |
| B♭ minor | "Paper Gangsta" |

| Major | Songs |
|---|---|
| E major | "Eh, Eh", "Summerboy" |
| A♭ major | "Paparazzi" chorus |
| B major | "Poker Face" chorus |
| B♭ major | "Brown Eyes" (ballad) |

The tabs add drop-B songs centred on B, A and G (Attack Attack!) and
drop-C songs on C (ISMFOF). **Default:** a minor key. On a drop-tuned
band song, the tonic is the open low string (`modes-scales.md`).

### Natural minor scale
```
W - H - W - W - H - W - W

A minor:  A B C D E F G
C minor:  C D Eb F G Ab Bb
G minor:  G A Bb C D Eb F
B minor:  B C# D E F# G A
```

### Major scale
```
W - W - H - W - W - W - H

E major:  E F# G# A B C# D#
Ab major: Ab Bb C Db Eb F G
```

---

## Chord Construction

| Type | Formula | Example (C) | Used here |
|---|---|---|---|
| **Power chord (5)** | 1 - 5 (- 8) | C - G - C | Every distorted guitar part; Gaga "I Like It Rough" verse |
| Major | 1 - 3 - 5 | C - E - G | Synths, pads, clean guitar |
| Minor | 1 - ♭3 - 5 | C - E♭ - G | Synths, pads, clean guitar |
| **Minor add6** | 1 - ♭3 - 5 - 6 | C - E♭ - G - A | "Paparazzi" verse, "Beautiful, Dirty, Rich" |
| **sus2** | 1 - 2 - 5 | C - D - G | "Brown Eyes" chorus |
| Minor 7 | 1 - ♭3 - 5 - ♭7 | C - E♭ - G - B♭ | "Eh, Eh", "Summerboy" ii7; "Paper Gangsta" v7 |
| Major 7 | 1 - 3 - 5 - 7 | C - E - G - B | Underoath ("AM7", see `docs/post-hardcore-findings.md`) |
| Dominant 7 | 1 - 3 - 5 - ♭7 | C - E - G - B♭ | Major-key pop only ("Brown Eyes" V7) |
| Diminished | 1 - ♭3 - ♭5 | C - E♭ - G♭ | Passing only ("Just Dance" bridge ii°) |

A **no-3rd** chord (`no3` in Hooktheory) is a power chord played by the
whole band. "Beautiful, Dirty, Rich" opens `i(no3) – VII(no3)`, and "I
Like It Rough" plays the whole verse that way.

---

## Diatonic Chords in a Minor Key (the Default)

| Degree | Chord | In A minor | In C minor | Role on these records |
|---|---|---|---|---|
| i | minor | Am | Cm | Home |
| ii° | diminished | B° | D° | Passing only |
| III | major | C | E♭ | Relative-major home for the chorus lift |
| iv | minor | Dm | Fm | Darker turn ("LoveGame", "Just Dance") |
| v | **minor** | Em | Gm | Keep it minor; no major V |
| VI | major | F | A♭ | The most used chord after i |
| VII | major | G | B♭ | Loop turnaround |
| ♭II | major (Phrygian) | B♭ | D♭ | Breakdown neighbour only |

---

## How Sections End (Instead of Cadences)

The classical cadences (V→I, IV→I) mostly don't appear. Sections end like
this:

| Ending | How | Where |
|---|---|---|
| **Loop return** | The loop's last chord (VII or VI) leads back to i at the next section | Every four-chord loop song |
| **Relative-major lift** | Verse ends on VI or VII, the chorus starts on the old III | "Poker Face", "Paparazzi" |
| **Stop** | One hit, then silence (or a short 1/4 or 2/4 bar), then the drop | "AC-130" stutter bars, the 2/4 pickup bars (`rhythm-patterns.md`) |
| **Pre-chorus hang** | Pre-chorus ends on `v` or `VII` without resolving | "Just Dance" pre-chorus `VI – v` |
| **Ritardando to a last hit** | Tempo eases down into the last chord | "Smokahontas" 135 → 120 |
| V → I | Only in major-key pop and ballads | "Summerboy" chorus, "Brown Eyes" |

---

## Dissonance: What's a Feature Here, What's a Mistake

The textbook rules ("avoid notes a half step apart", "avoid parallel
fifths") don't fit these records.

### Features, not mistakes
- **Half-step riff motion**: root against ♭II, or root against the note
  below. "A for Andrew" A–B♭, "AC-130" G–A♭, ISMFOF breakdown C–B.
- **Parallel fifths and octaves**: every power-chord riff, and every bass
  doubling the riff an octave down.
- **Added 6ths and 2nds** on synth chords and pads ("Paparazzi" add6,
  "Brown Eyes" sus2, "Summerboy" add6).
- **Pedal against moving chords**: one bass note held while the chords
  change above it ("Sexual Man Chocolate" verse).

### Real mistakes on these records
- **Full triads with 3rds in a low, distorted guitar.** The 3rd turns to
  mud under distortion (see `docs/metalcore-gap-analysis.md`). Use power
  chords and put the 3rd in the synth or vocal.
- **A major V in a minor-key loop.** It sounds classical or "musical
  theatre". Keep `v` minor.
- **A vocal off the chord on a strong beat.** "Poker Face" lands 75% of
  its notes on chord tones and is 96% diatonic (HT). Put the colour notes
  in the synth, not the lead vocal.
- **Two parts fighting in one register.** Two distorted guitars and a
  synth all at C3–C4. Split them by octave or by side (`orchestration.md`).

---

## Chord Voicings for Guitar, Synth and Pad

**Distorted guitar (power chords, root position):**
```
Drop B:   B1 - F#2 - B2      (root - 5th - octave on the open string)
Drop C:   C2 - G2  - C3
Standard: E2 - B2  - E3
```
Profile: `voicing_style: power_chords` (the `riff` composer uses it).

**Clean / acoustic guitar:** open-position chords (`guitar_open`),
for intros, clean verses and ballads (Underoath's open Am / AM7 chords,
see `docs/post-hardcore-findings.md`).

**Synth chords and pads (measured registers):**
| Part | Range on the records | Where |
|---|---|---|
| Synth ostinato / pluck | E4–A4 | "Just Dance" (8th-note two-note figure) |
| Synth pad | B2–F♯4 | "Just Dance" (half notes) |
| Synth pad (wide) | A2–B5 | Underoath "Reinventing" |
| Lead synth | C3–G5 | ISMFOF "Things That Rhyme…"; Underoath synth lead E4–G5 |
| Bell / sparkle | B♭4–B♭5 | Breathe Carolina "Wooly" |
| Saw stack | A♭2–E♭5 | Breathe Carolina "Wooly" |

Pads hold whole notes. Leads and plucks run 8ths and 16ths. Voice synth
chords as full triads or add6 chords **above** the guitar's power chords.
The synth carries the 3rd that the guitar leaves out.

---

For progressions see `chord-progressions.md`. For how parts stack see
`voice-leading.md`. For instrument roles see `orchestration.md`.
