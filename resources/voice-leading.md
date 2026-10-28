# Voice Leading Reference (default)

How the parts move and stack on the reference albums (`reference-albums.md`).
A genre profile in `genres/` overrides anything here. The classical rules
(avoid parallel fifths, resolve the leading tone, and so on) are in
`classical-reference/voice-leading.md`. Use them only for classical or choral
writing.

## Contents
- Core principles
- Interval rules (by part)
- Common mistakes
- Part writing: spacing and registers
- Multi-track arrangement tips
- Quick checklist before generating MIDI

---

## Core Principles

### 1. Smooth motion is for the vocal and the pads, not the riff
Measured from the transcriptions:

| Part | Moves by step | Leaps of a 4th or more | Source |
|---|---|---|---|
| Sung vocal (pop-punk / screamo) | 61–78% | 7–14% | Sueco, Lil Lotus, Underoath tabs |
| Sung vocal (crunk / electronicore) | 53–55% | 24–26% | 3OH!3, Breathe Carolina tabs |
| Distorted guitar riff | about half | about half | Every band tab: riffs jump between the open string and fretted notes |
| Pad / held synth | Whole notes, common tones held | — | "Just Dance", Underoath "Reinventing" |

**Rule:** write the vocal (`melody` role, `vocal_line`) mostly by step, landing
on chord tones on strong beats. Let pads hold common tones between chords.
Riffs are free to leap. A riff that keeps returning to the open low string is
*supposed* to jump.

### 2. Parallel motion is the sound
Classical writing wants contrary motion. These records do the opposite:
- **Power chords** (root + 5th + octave) move in parallel fifths and octaves
  on every distorted riff.
- **The bass doubles the riff** an octave down, note for note, on Attack
  Attack!, ISMFOF and Underoath tabs (`chord-progressions.md` → bass patterns).
- **Kick follows the chugs** in breakdowns (drum `kick_follow`; the bass
  style `kick_locked` makes the bass follow the kick too).
- **Vocals stack in unison and octaves.** Joey Sturgis: "I double everything,
  triple some things and quadruple others", with "four low screams that
  combine to support one centre scream" (Sound On Sound).

Contrary motion is still useful *between layers that do different jobs*: a
vocal line rising while the riff pedals on the low string, or a synth lead
falling over a climbing bass.

### 3. Lock the parts together rhythmically
Joey Sturgis: "Everything's to a click", every note checked "on the grid, on
time and in tune". In heavy sections, guitar, bass and kick hit together.
Separation comes from register and stereo side, not from rhythmic
independence (`counterpoint.md` covers when parts should split apart).

---

## Interval Rules (by Part)

### Use freely
| Interval | Where |
|---|---|
| Unison / octave | Doubled guitars, bass under riff, stacked vocals |
| Perfect 5th | Power chords, the whole distorted guitar layer |
| Minor 2nd (half step) | Riffs and breakdowns: A–B♭ ("A for Andrew"), G–A♭ ("AC-130"), C–B (ISMFOF) |
| 3rds and 6ths | Synth chords, pads, harmony vocals |
| Added 6th / 2nd | Synth chords and pads ("Paparazzi" add6, "Brown Eyes" sus2) |

### Use carefully
| Interval | Why |
|---|---|
| 3rd inside a low distorted chord | Turns to mud. Leave the 3rd to synth or vocal |
| Big vocal leap (6th or more) | Rare (under 10% on most records). Use for a hook moment, then step back |
| Tritone in the vocal | Not found as a melodic move in the charts |
| Half step in the vocal against the chord on a strong beat | Sounds wrong, not edgy. Keep it in the riff |

---

## Common Mistakes

### 1. Clusters in the wrong register
A half-step clash is a feature in a low riff (two notes in time, one after the
other) and in a pad's added 6th up high. Don't put it as a *held* chord in the
low-mid register (C3–C4) under distortion. That's mud.

### 2. Treating "dissonance must resolve" as a rule
The `♭II` in a breakdown never resolves; it alternates with the root for the
whole section. Leave it.

### 3. Parts crossing in register
Crossing *is* a problem here, for a different reason than in classical
writing: a synth lead dropping into the guitar's range (below about C4) gets
buried. Keep each role in its own band (registers below).

### 4. Doubling the 3rd in heavy chords
Classical writing warns about doubling the leading tone. Here the warning is
about the 3rd: don't double it in distorted guitars. One clean source (synth
or vocal) carries it.

---

## Part Writing: Spacing and Registers

Measured ranges from the transcriptions:

| Role | Range | Notes |
|---|---|---|
| Bass, drop B | B0–B1 | Attack Attack! |
| Bass, drop C | C1–A♭2 | ISMFOF, Breathe Carolina |
| Bass, standard | E1–B2 | Pop-punk |
| Club synth bass | E1–A1 | Gaga "Just Dance" |
| Distorted guitar | Open low string (B1 / C2 / D2 / E2) up to about E4 | Power chords sit in the bottom octave and a half |
| Pad | B2–F♯4 (narrow) up to A2–B5 (wide) | "Just Dance"; Underoath "Reinventing" |
| Synth ostinato / pluck | E4–A4 | "Just Dance" |
| Lead synth | C3–G5 | ISMFOF, Underoath |
| Bell / sparkle | B♭4–B♭5 | Breathe Carolina "Wooly" |
| Male sung lead | E♭3–B4 | Sueco, Attack Attack! clean vocals |
| Gaga chorus top | up to E♭5 | "Paparazzi" |
| Rapped vocal | a 3rd–4th wide, around E3–A♭3 | Sueco "Hate You Too" |

**Spacing:** keep the bass and guitar root an octave apart (the doubling).
Keep pads and synth chords above about B2 so they sit over the power chords.
Put the lead synth or vocal at least an octave above the guitar root.

---

## Multi-Track Arrangement Tips

### Assigning notes to tracks
| Track (role) | Carries |
|---|---|
| `rhythm_guitar` | Root + 5th (+ octave). The riff rhythm |
| `bass` / `synth_bass` | The riff root an octave down, or an 808/sub held note |
| `chords` / `pad` | The full chord with its 3rd (and add6 / sus2 colour) |
| `melody` | Vocal line: steps and chord tones |
| `lead` | Synth hook or ostinato above the pad (intros, breaks, chorus doubles) |
| `drums` / `electronic_drums` | Kick locked to the riff in heavy parts |

Write the riff first so bass and kick can follow it (`layer_order`).

### Double-tracking (how the records stack parts)
- **Guitars:** two takes, one left, one right, never four (Joey Sturgis,
  Audiofanzine). Profile: `double_track` with `takes: 2`, mode `identical`
  (tight) or `variation` (wider).
- **Vocals:** doubled, tripled or quadrupled. Low screams under a centre
  scream. Profile: `double_track` mode `octave` or `harmony` with `takes: 4`
  for the scream stack; `identical` with `takes: 2` for a clean chorus double.
- **Synth leads:** an octave double (`octave`) for the chorus hook.

### Avoid frequency masking
- Only the bass and the kick live under about 100 Hz. Tune the kick to the key
  (Sturgis tunes kick samples to the song's key).
- The guitar and the synth pad fight in the low-mids. Thin one of them out in
  heavy sections, or move the pad up an octave.
- When the dance/techno break arrives, the guitars drop out and the synth bass
  takes the low end alone (`rhythm-patterns.md`, family 7).

### Bass + chord relationship
| Pattern | Use |
|---|---|
| Bass = riff root | Heavy sections, every band record |
| Bass on the chord's 3rd | Smoother walk-down (Sueco "Paralyzed": B♭ over D) |
| Bass pedal under moving chords | Tension ("Sexual Man Chocolate" verse) |
| Bass alternating two roots | Club verses ("Just Dance" C♯–F♯) |

---

## Quick Checklist Before Generating MIDI

- [ ] Riff written first; bass and kick follow it in heavy sections
- [ ] Distorted guitar uses power chords (no 3rds low down)
- [ ] The 3rd lives in the synth/pad or vocal
- [ ] Vocal moves mostly by step and lands on chord tones on strong beats
- [ ] Each role stays in its register band (no lead buried under C4)
- [ ] Guitars doubled L/R (2 takes), vocals stacked where the section is big
- [ ] Everything on the grid (`humanize.timing` near 0 for electronicore/club)
- [ ] Half-step clashes only in the riff or as pad colour, never as a held low chord
