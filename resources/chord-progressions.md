# Chord Progressions Reference (default)

Built from the reference albums (`reference-albums.md`). Sources: `HT` =
Hooktheory chart, `UG` = Ultimate Guitar chart, `tab` = bass roots read
from the Songsterr transcription. A genre profile in `genres/` overrides
anything here. The textbook version (jazz ii–V–I, the blues, the 50s
progression) is in `classical-reference/chord-progressions.md`.

## Contents
- How harmony works on these records
- The minor four-chord loop (the default)
- Two-chord loops
- Pedal + chromatic neighbour (riff harmony)
- Relative-major choruses (the "modulation" these records use)
- Major-key progressions (pop, pop-punk, ballads)
- Dorian (the major IV in a minor key)
- Bridges and chromatic moves
- Progression templates by section and mood
- Bass line patterns (measured)
- Substitutions

Numerals: lowercase = minor chord, uppercase = major. In a minor key, `VI`,
`III` and `VII` are the major chords built on the 6th, 3rd and 7th of the
natural minor scale. In A minor that's F, C and G.

---

## How Harmony Works on These Records

1. **Minor key, looped.** A four-chord (or two-chord) loop repeats through
   the verse and the chorus. Sections differ by drums, density and
   register more than by new chords.
2. **No major V → i in minor keys.** None of the minor-key charts here uses
   the classical dominant resolution. When a `v` appears, it's minor
   ("Starstrukk", "LoveGame", "Money Honey", "Paper Gangsta"). Loops
   "resolve" by coming back around to `i`. V → I shows up only in the
   major-key Gaga songs ("Summerboy" chorus IV7–V–I, ballad "Brown Eyes"
   I–vi–IV–V7).
3. **The chorus can keep the same chords and still sound brighter**
   by treating them as the relative major (see "Relative-major choruses").
4. **Heavy sections drop the harmony to one note.** The breakdown is a
   pedal on the open low string plus a half-step neighbour, not a
   progression.

---

## The Minor Four-Chord Loop (the Default)

**i – VI – III – VII** and its rotations. It's the most common progression
across these albums: rock bands, crunk and club pop alike.

```
A minor:  Am – F  – C  – G
C minor:  Cm – Ab – Eb – Bb
G minor:  Gm – Eb – Bb – F
B minor:  Bm – G  – D  – A
```

| Song | Progression | Source |
|---|---|---|
| 3OH!3 "Don't Trust Me" (chorus, G minor) | i–VI–III–VII (Gm Eb Bb F) | HT + tab (bass G Eb Bb F) |
| Sueco "Loser" | i–VI–III–VII (Cm Ab Eb Bb) | tab |
| Sueco "Paralyzed" (C minor) | i–VI–III–VII with the VII over its 3rd (bass walks C–A♭–E♭–D) | UG + tab |
| Attack Attack! "Smokahontas" (Dub Step Break) | i–VI–III–VII (Bm G D A) | tab |
| Underoath "Reinventing Your Exit" | i–III–VI–VII (Am C F G) | tab |
| Underoath "It's Dangerous Business…" | i–VI–iv–VII intro, i–VI–VII chorus | tab |
| Underoath "Some Will Seek Forgiveness…" (3/4) | i–VII–VI–VII (Bm A G A) | tab |
| 3OH!3 "Starstrukk" (D minor) | chorus i–VII–VI–VII; verse i–v–VII–IV | HT |
| Lady Gaga "Paper Gangsta" (B♭ minor) | i–VI–VII–v7 | HT |
| Lady Gaga "LoveGame" (B minor) | i–VI–III–iv–v | HT |
| Lady Gaga "Just Dance" (C♯ minor) | i–III–VII–iv | HT |
| Breathe Carolina "Blackout" (D♯ minor) | i–VII–III–v–VI | HT |
| ISMFOF "Ravenous, Ravenous Rhinos" | i–VI–VII (C Ab Bb) | tab |
| ISMFOF "Brief Interviews…" | i–III–VI (Dm F Bb) | tab |

**Common rotations:**
```
i  – VI – III – VII     (the standard order)
i  – VII – VI – VII     (rocking: back and forth off the VII)
i  – VI – VII           (three-chord version)
i  – III – VI – VII     (bass climbs, then steps)
VI – III                (just the two bright chords: ISMFOF "Beauty…" chorus over C minor)
```
Add `iv` or `v` (minor) as a fifth chord for a darker turn (LoveGame,
Blackout, Paper Gangsta). Keep `v` **minor**. A major V would bring back the
classical cadence these records avoid.

---

## Two-Chord Loops

The club and crunk tracks often use only two chords for a whole section.

| Loop | Songs |
|---|---|
| i – VI | Gaga "Starstruck" (G♯m–E, whole song), "Paparazzi" verse (Cm(add6)–A♭), Sueco "Toxic Therapy" (Em–C) |
| i – iv | Gaga "Just Dance" bass (C♯–F♯ alternating under the four-chord loop) |
| i – v | Gaga "Money Honey" (i–v–VII), 6arelyhuman "Millionaire" (bass A–E) |
| i – VII | Gaga "The Fame" guitar riff (G–F) |

**Default for electroclash / club / crunk verses:** two chords, a bar each,
for 8 bars.

---

## Pedal + Chromatic Neighbour (Riff Harmony)

In breakdowns and heavy verses the "progression" is one low note plus the
note a half step above or below it.

| Song | Bass/guitar roots | What it is |
|---|---|---|
| Attack Attack! "A for Andrew" (drop A) | A – B♭ – A – B♭ … | i – ♭II |
| Attack Attack! "AC-130" (drop G) | G – A♭ – G – A♭ | i – ♭II |
| Attack Attack! "Sexual Man Chocolate" verse (drop B) | B – B♭ – A – B♭ | pedal with a chromatic wobble below |
| Attack Attack! "Sexual Man Chocolate" break | B – D – B – D – C | i – III – i – ♭II |
| ISMFOF "Things That Rhyme with Orange" breakdown | C – B – C – B | i with its leading tone |
| ISMFOF "Brief Interviews…" breakdown | E♭ – E – F | chromatic climb |
| 6arelyhuman "CR4CK HOUSE" | D (one note all song) | pure pedal |

Use the `riff` composer (`pedal_tone`, `chug`, `gallop`) with a Phrygian or
minor section `scale`. The `♭II` comes from the scale, so you don't need
chord symbols.

---

## Relative-Major Choruses (the Modulation These Records Use)

There's no key change in the classical sense. The chorus re-centres on the
**relative major** and often keeps the same chords.

| Song | Verse | Chorus |
|---|---|---|
| Lady Gaga "Poker Face" | G♯ minor: i – VI – VII (G♯m E F♯) | B major: vi – IV – I – V (G♯m E B F♯). Same chords plus B, heard as major |
| Lady Gaga "Paparazzi" | C minor: i(add6) – VI (Cm A♭) | A♭ major: I – V – vi – IV (A♭ E♭ Fm D♭) |

**Rule:** to lift a chorus, move the loop so it starts on the old `III`
(the relative-major tonic). In Poker Face terms, `i–VI–VII` becomes
`vi–IV–I–V`. The melody range rises with it: "Poker Face" verse F♯3–G♯4,
chorus B2–B4; "Paparazzi" verse C4–A♯4, chorus E♭4–E♭5 (HT).

---

## Major-Key Progressions (Pop, Pop-Punk, Ballads)

| Progression | Song | Source |
|---|---|---|
| I – V – vi – IV | Gaga "Paparazzi" chorus (A♭ E♭ Fm D♭) | HT |
| vi – IV – I – V | Gaga "Poker Face" chorus | HT |
| IV – I – V (verse), I – V6 – ii7 (chorus) | Gaga "Eh, Eh" (E major, 94) | HT |
| ii7 – V(add6) – I – IV(add6) / IV7 – V – I – V6 – vi | Gaga "Summerboy" (E major, 105) | HT |
| vi – I – IV (verse), I – II – IV (chorus) | ISMFOF "Things That Rhyme with Orange" (bass A C F / C D F) | tab |
| Descending step bass: I – V/7 – vi – V – IV | Sueco "It's Going Good!" (bass E D♯ C♯ B A) | tab |
| Climbing bass in 3/4 | Sueco "Today" (bass E F♯ G G♯) | tab |
| Ballad: I – iii⁶₄ – IV, then I – vi(sus2) – IV – V7 | Gaga "Brown Eyes" (B♭ major, 62) | HT |

Pop-punk tip: the same four chords serve as `vi–IV–I–V` (major, bright)
and `i–VI–III–VII` (minor, sad). Pick the one that matches the mood.
These two pop-punk albums lean major in the audio key estimates, but those
estimates often mix up relative major and minor (`reference-albums.md`).

---

## Dorian (the Major IV in a Minor Key)

| Song | Progression |
|---|---|
| Lady Gaga "I Like It Rough" (C Dorian, 120) | i – VII – IV – III (power chords, no 3rd, in the verse) |
| Lady Gaga "The Fame" (G Dorian, 127) | HT lists the verse/chorus as "I II III" in G Dorian; the guitar tab's riff walks G–F–G–A. Bridge in D minor: VI – VII – i – VI – VII |

The major `IV` (F in C minor) is the Dorian colour. Use a section `scale` of
`dorian` for a funkier, glam-rock minor (Bowie-style guitar, per the
reviews of "The Fame").

---

## Bridges and Chromatic Moves

- **Rising root line:** "Just Dance" bridge `i⁶ – ii° – III – ♭IV – ♭V`,
  with no 5th on the middle chords and root only on the last two. The bass
  climbs D♯–E–F–G out of C♯ minor (HT).
- **Relative-key bridge:** "The Fame" bridge moves from G Dorian to D minor.
- **Drop to the pedal:** most band songs replace the bridge with a
  breakdown (see above).

---

## Progression Templates by Section and Mood

| Section / mood | Default | Alternatives |
|---|---|---|
| Verse (club/crunk) | `i – VI` (2 chords) | `i – iv`, `i – v` |
| Verse (band) | `i – VI – III – VII` | `i – III – VI – VII` |
| Pre-chorus | `VI – v` (Just Dance) or `VI – VII` | hold the `VI` |
| Chorus (dark) | `i – VI – III – VII` | `i – VII – VI – VII` |
| Chorus (lift) | Relative major: `vi – IV – I – V` | `I – V – vi – IV` |
| Breakdown | Pedal `i` + `♭II` (Phrygian) | `i` + leading tone |
| Dance / techno break | `i` – `VI` – `III` – `VII` on a gated synth (`trance_gate`) | One-chord pedal |
| Ballad / acoustic | `I – vi – IV – V7` | `I – iii – IV` |
| Glam / funk minor | Dorian `i – VII – IV – III` | `i – VII` |

### By mood
| Mood | Use |
|---|---|
| Dark / sad / emo | `i – VI – III – VII` in minor |
| Bright / pop-punk sugar | `I – V – vi – IV` or `vi – IV – I – V` |
| Sleazy / club | Two-chord `i – VI` or `i – v`, added 6ths |
| Heavy | One-chord pedal + `♭II` |
| Swagger / glam | Dorian, power chords |

---

## Bass Line Patterns (Measured)

| Pattern | What it does | Where |
|---|---|---|
| **Doubles the guitar riff** an octave down | Bass = riff, note for note | Attack Attack! ("Renob, Nevada", "A for Andrew", "Sexual Man Chocolate" break: bass roots identical to the guitar roots), Underoath "Reinventing", ISMFOF "Things That Rhyme…". (In the "Sexual Man Chocolate" verse the bass holds a B–B♭–A–B♭ pedal under a moving guitar instead.) |
| **Root 8ths** | Root on every 8th note | Gaga "Just Dance" (8.0 notes/bar), "Money Honey", Sueco "SOS" |
| **Pedal + octave pump** | One root, octave jumps | 6arelyhuman "CR4CK HOUSE" (D1–D2, 8ths) |
| **Two-note alternation** | i–iv or i–v, a bar each | "Just Dance" C♯–F♯, "Millionaire" A–E |
| **Walk-down** | Step down the scale under the loop | Sueco "It's Going Good!" E D♯ C♯ B A |
| **Inversion bass** | Bass on the 3rd of the VII chord | Sueco "Paralyzed" (B♭/D) |
| **Sub / 808 hold** | One long note per chord | 3OH!3 "Richman" (whole notes, E♭1–B♭1) |

Registers (measured): drop-B bass B0–B1, drop-C bass C1–A♭2, standard-tuning
pop-punk bass E1–B2, club synth bass E1–A1.

---

## Substitutions

| Original | Substitute | Effect | Where |
|---|---|---|---|
| `i` | `i(add6)` | Sleazier, Dorian tint | "Paparazzi", "Beautiful, Dirty, Rich" |
| triad | power chord (no 3rd) | Heavier, fits distortion | "I Like It Rough", "Beautiful, Dirty, Rich" |
| `VII` | `VII/3` (3rd in the bass) | Smoother walk-down | "Paralyzed" |
| minor loop | relative-major loop | Lift a chorus | "Poker Face", "Paparazzi" |
| progression | pedal + `♭II` | Breakdown | Attack Attack!, ISMFOF |
| `IV` | `IV7` / `IV(add6)` | Pop polish | "Summerboy" |
