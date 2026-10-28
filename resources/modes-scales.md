# Modes and Scales Reference (default)

Built from the reference albums (`reference-albums.md`). A genre profile in
`genres/` overrides anything here. The textbook survey (blues, whole-tone,
jazz melodic minor) is in `classical-reference/modes-scales.md`.

## Contents
- What the reference albums actually use
- The key follows the tuning
- The seven modes (with how much each is used here)
- Scale per section
- Melody inside the scale (measured)
- Relative major / minor
- Quick reference: notes in each mode (key of C)

---

## What the Reference Albums Actually Use

| Scale | How common | Where | Profile `scale` |
|---|---|---|---|
| **Natural minor (Aeolian)** | The default. Most chord-checked songs are minor | "Don't Trust Me", "Starstrukk", "Blackout", "Paralyzed", "Just Dance", "LoveGame", "Poker Face" verse, "Money Honey", "Paper Gangsta", "Starstruck" (all chart-checked); Underoath's loops are minor by their bass roots ("Reinventing" A C F G) | `minor` |
| **Phrygian (♭2)** | Breakdowns and heavy riffs only | Attack Attack! "A for Andrew" (A–B♭), "AC-130" (G–A♭), "Sexual Man Chocolate" break (B + C) | `phrygian` |
| **Major (Ionian)** | Pop-punk choruses, the brighter Gaga songs, ballads | "Paparazzi" and "Poker Face" choruses, "Eh, Eh", "Summerboy", "Brown Eyes", "Things That Rhyme with Orange", Sueco "It's Going Good!" | `major` |
| **Dorian (natural 6)** | Glam / funk minor | Gaga "I Like It Rough" (C Dorian), "The Fame" (G Dorian); the `add6` colour in "Paparazzi" and "Beautiful, Dirty, Rich" | `dorian` |
| Harmonic minor | **Not found.** No major V in any minor-key chart | — | use only if a genre profile asks |
| Lydian, Mixolydian, Locrian, whole-tone, blues | **Not found** on these records | — | not a default |

**Default:** minor for verses and dark choruses. Phrygian for breakdowns.
Major (the relative major) for a lifted chorus. Dorian for sleaze, glam and
funk.

---

## The Key Follows the Tuning

On the heaviest band records (Attack Attack!, ISMFOF) the tonic is the
guitar's open low string, so the heaviest riff note rings open. Underoath
is the exception: drop D under A-minor loops.

| Record | Lowest string (tabs) | Song roots (tabs) |
|---|---|---|
| Attack Attack! | B1 (drop B); A1 on "A for Andrew"; G1 on "AC-130" | B pedal ("Sexual Man Chocolate", "Smokahontas"), A ("A for Andrew"), G ("AC-130") |
| ISMFOF | C2 (drop C) on every tab | C roots on "Things That Rhyme…", "Beauty…", "Ravenous…", "Crank That" |
| Breathe Carolina | C2 ("Wooly", "Lauren's Song"), D2 ("Edge of Heaven") | — |
| Underoath | D2 (drop D) on every tab | A-minor loops over the drop-D string ("Reinventing", "It's Dangerous"); D under "Down, Set, Go" |
| Sueco / Lil Lotus | E2 (standard) mostly; C2 on "Paralyzed"; E♭ on "Loser" | "Paralyzed" C minor on the drop-C string |
| 3OH!3 / Lady Gaga | E2 (standard), guitar is a support part | Key set by the synths and vocal |

**Rule:** with a `tuning` set, choose a key whose tonic is the open low
string (drop B → B minor / B Phrygian; drop C → C minor). The riff and
bass composers already pedal on that string.

---

## The Seven Modes (and How Much Each Is Used Here)

| Mode | Formula | Character | Use on these records |
|---|---|---|---|
| Ionian (major) | W-W-H-W-W-W-H | Bright, resolved | Pop-punk and pop choruses, ballads |
| Dorian | W-H-W-W-W-H-W | Minor with a major 6th | Glam/funk minor (Gaga) |
| Phrygian | H-W-W-W-H-W-W | Dark; ♭2 sits a half step over the root | Breakdowns, chug riffs |
| Lydian | W-W-W-H-W-W-H | Floating | Not used |
| Mixolydian | W-W-H-W-W-H-W | Rock-major with ♭7 | Not used |
| Aeolian (natural minor) | W-H-W-W-H-W-W | Dark, emotional | **The default** |
| Locrian | H-W-W-H-W-W-W | Unstable | Not used |

```
A Aeolian:   A B C D E F G        (the default minor)
A Phrygian:  A Bb C D E F G       (A for Andrew: the riff lives on A and Bb)
A Dorian:    A B C D E F# G       (major 6th, F#)
C Ionian:    C D E F G A B
```

---

## Scale per Section

Genre profiles can set a `scale` per section (it shares the root). The
records do this:

| Section | Scale | Why |
|---|---|---|
| Verse (heavy) | Phrygian or minor | Chug riff with a half-step neighbour |
| Verse (club) | Minor or Dorian | Two-chord loop; add6 colour |
| Pre-chorus | Minor | Build |
| Chorus (dark) | Minor | Four-chord loop |
| Chorus (lift) | Relative major | "Poker Face", "Paparazzi" |
| Breakdown | Phrygian | `i – ♭II` pedal |
| Ballad / acoustic | Major | "Brown Eyes" (B♭ major, HT) |

---

## Melody Inside the Scale (Measured)

- "Poker Face" melody: **96% diatonic, 75% chord tones**, mostly
  stepwise (HT).
- Sung vocal lines in the tabs move **by step 53–78% of the time**
  (album averages) and leap a 4th or more only **7–26%** of the time.
  The pop-punk and screamo records sit at the smooth end (Sueco 61% step /
  7% leap, Lil Lotus 74% / 10%, Underoath 65% / 14%). The crunk and
  electronicore records leap more (3OH!3 55% / 26%, Breathe Carolina
  53% / 24%).
- Rapped lines stay inside **a 3rd or 4th**. Sueco "Hate You Too": E3–A♭3
  at 10 notes per bar. "Toxic Therapy": 7 notes per bar.
- Ranges (measured): male sung lead roughly **E♭3–B4** (Sueco
  "Paralyzed" E♭3–B♭4, Attack Attack! clean vocals B3–B4). Gaga choruses
  climb to E♭5 ("Paparazzi") and B4 ("Poker Face"). Breathe Carolina
  "Blackout" D♯4–G♯5.

So keep melodies in the scale, on chord tones at the strong beats, and
moving mostly by step. The chromatic notes live in the riff, not in the
vocal line.

---

## Relative Major / Minor

The four-chord loop and the relative-major chorus both depend on these
pairs:

| Minor | Relative major | Loop i–VI–III–VII = vi–IV–I–V |
|---|---|---|
| A minor | C | Am F C G |
| B minor | D | Bm G D A |
| C minor | E♭ | Cm A♭ E♭ B♭ |
| C♯ minor | E | C♯m A E B |
| D minor | F | Dm B♭ F C |
| E minor | G | Em C G D |
| G minor | B♭ | Gm E♭ B♭ F |
| G♯ minor | B | G♯m E B F♯ |

---

## Quick Reference: Notes in Each Mode (Key of C)

| Mode | Notes |
|---|---|
| C Ionian | C D E F G A B |
| C Dorian | C D E♭ F G A B♭ |
| C Phrygian | C D♭ E♭ F G A♭ B♭ |
| C Lydian | C D E F♯ G A B |
| C Mixolydian | C D E F G A B♭ |
| C Aeolian | C D E♭ F G A♭ B♭ |
| C Locrian | C D♭ E♭ F G♭ A♭ B♭ |
