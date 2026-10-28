# Orchestration Reference (default): The Band + Synth + Club Kit

Which instruments the reference albums (`reference-albums.md`) use, what
each one does, and how they stack. A genre profile in `genres/` overrides
anything here. The orchestra, jazz combo and film-score version is in
`classical-reference/orchestration.md`.

## Contents
- Instrument families (on these records)
- Register and roles
- Combining instruments (doubling, layering, blend)
- Dynamic layering (building intensity)
- Arrangements by record type
- Frequency considerations
- Track assignment tips for MIDI
- Quick checklist

---

## Instrument Families (on These Records)

### Guitars
| Part | Job | Evidence |
|---|---|---|
| Distorted rhythm guitar, drop-tuned | Power-chord riffs, chugs, breakdowns | Attack Attack! drop B/A/G, ISMFOF drop C, Underoath drop D, Breathe Carolina drop C/D |
| Doubled L/R | Width. Two takes, never four | Joey Sturgis (Audiofanzine) |
| Clean / open-chord guitar | Intros, clean verses, ballads | Underoath open Am / AM7 (`docs/post-hardcore-findings.md`) |
| Glam / Bowie-style riff | Guitar riff in a dance song | Gaga "The Fame" (Wikipedia), "I Like It Rough" power chords |
| Standard-tuning pop-punk guitar | Strummed 8th power chords | Lil Lotus, Sueco, 3OH!3 tabs (E2) |

Profile: role `rhythm_guitar`, `tuning`, riff styles `pedal_tone`, `chug`,
`gallop`, `power_strum`, `palm_eighths`, `tremolo`, `open_chords`,
`clean_arp`. Measured: palm-muted notes are 19–36% of guitar notes; 57–70%
of guitar events are chords (power chords), the rest single notes.

### Bass
| Part | Job | Evidence |
|---|---|---|
| Bass guitar doubling the riff | Root an octave down | Attack Attack!, ISMFOF, Underoath tabs |
| Synth bass, root 8ths | Club pulse | Gaga "Just Dance" |
| 808 / sub | Long held low note | 3OH!3 "Richman" (whole notes, E♭1–B♭1) |
| Pedal / octave pump | One note, octave jumps | 6arelyhuman "CR4CK HOUSE" (D1–D2) |

Profile: roles `bass`, `synth_bass`; styles `doubled`, `root_eighths`,
`octave`, `808`/`sub`, `wobble`, `arpeggio`, `walking`.

### Synths
| Part | Job | Evidence |
|---|---|---|
| Pad | Held chord with its 3rd | "Just Dance" half notes; Underoath "Reinventing" |
| Ostinato / pluck | 8th-note two-note figure | "Just Dance" (E–A) |
| Lead / saw stack | Hook, breaks | ISMFOF, Breathe Carolina "Wooly" saw stack A♭2–E♭5 |
| Bell / sparkle | Top-octave colour | "Wooly" B♭4–B♭5 |
| Trance-gated chords | 16th-chopped chords | Breathe Carolina "house-techno sequencers" (Alternative Press) |
| Wobble / dubstep bass | Dubstep break | Attack Attack! "Smokahontas" "Dub Step Break" |

Character words from the sources: "stainless steel synths" (AllMusic on *The
Fame*), "sawing electronics", "blown-out synths" (crunkcore). Profile: roles
`chords`, `pad`, `lead`, `fx`; chord styles `trance_gate`, `stab`, `strum8`,
`pulse4`, `sparse_held`. Instrument maps: `serum-2`, `vital`, `odin-iii`.

### Drums
| Kit | Job | Evidence |
|---|---|---|
| Acoustic kit, sample-replaced, on the grid | Band sections | Sturgis: programmed drums first, drummer last, sample replacement |
| Club kit (909/808-style kick, clap, open hat) | Four-on-the-floor sections, dance breaks | *The Fame*, "Blackout", "Smokahontas" Techno Break |
| Crunk / hip-hop kit | Half-time verses | 3OH!3 "pre-programmed beats"; ISMFOF "Crank That" |
| Distorted, bit-reduced beats | Electroclash / "krushclub" | 6arelyhuman (Nina, 2024) |

Profile: roles `drums`, `electronic_drums`; `drum_patterns[].layer`
`acoustic` / `electronic`. Instrument maps: `superior-drummer-3`,
`ujam-beatmaker`, `ujam-beatmaker-style`, `gm-drums`.

### Vocals
Lead (sung or rapped), stacked doubles, scream stack, auto-tuned
("modulated vocals" in crunkcore; auto-tuned singing in electronicore; auto-
tune on "Starstruck" and "Paper Gangsta"). Vocals aren't rendered as audio
here. The `melody` role writes the vocal line as MIDI.

### Not used
Orchestral strings, woodwinds, brass, choir, jazz piano. None appear in the
sources for these records except the piano ballad "Brown Eyes". Use them only
when a genre profile or the user asks.

---

## Register and Roles

| Band | Range | Who lives there |
|---|---|---|
| Sub | below about E1 | 808 / sub, drop-B bass (B0) |
| Low | E1–B2 | Bass, the open low string of the guitar |
| Low-mid | B1–E4 | Distorted power chords |
| Mid | B2–F♯4 | Pads, plucks |
| Upper-mid | E♭3–B4 (male), up to E♭5 (Gaga) | Lead vocal |
| High | C4–B♭5 | Lead synth, bells |

Full measured table: `voice-leading.md` → registers.

---

## Combining Instruments

### Doubling (same notes)
| Doubling | Effect | Where |
|---|---|---|
| Guitar L + guitar R | Wide wall | Every heavy record (2 takes) |
| Bass an octave under the riff | One giant riff | Attack Attack!, ISMFOF, Underoath |
| Kick on the chugs | Breakdown punch | Every breakdown |
| Vocal ×2–×4 | Big chorus | Sturgis |
| Synth lead an octave up | Hook lift | Profile `double_track` `octave` |

### Layering (different notes)
| Layer | Over | Effect |
|---|---|---|
| Pad with the 3rd | Power chords | Tells the ear major/minor without muddying guitars |
| 16th gate / hats | Four-on-the-floor | Drive |
| Vocal | Loop | The only free line (`counterpoint.md`) |

### Timbral blend
Distorted guitars and saw synths blend into one wall. Bell and pluck sounds
cut through on top. A clean guitar or piano is a contrast colour for an intro
or a quiet bridge.

---

## Dynamic Layering (Building Intensity)

Measured contrast comes from **which layers play and how dense the drums
are**, not from slow crescendos:

| Step | Layers | Evidence |
|---|---|---|
| Verse | Fewer layers, sparse kick (1.8 kicks per bar on "Don't Trust Me"), quarter hats | `rhythm-patterns.md` |
| Pre-chorus | Straight quarter kick, hats filling up | "Turbo Swag", "Poker Face" |
| Chorus | Full band + synths, densest kick (5.0 per bar on "Don't Trust Me"), vocal stacks | Same |
| Break | Switch kit and genre (techno / dubstep / hip-hop) | "Smokahontas", "Fumbles O'Brian" |
| Stop | One hit, silence or a short bar | "AC-130" stutter bars |
| Breakdown | Guitars + bass + kick locked, half time | Every heavy record |

Profile: section `energy`, `roles`, `drum_pattern`, `fills` (`snare_build`,
`drop_out`, `stop_hit`), and `automation` shapes (`rise`, `ramp_up`, `pump`)
for filter sweeps and sidechain pumping into a drop.

---

## Arrangements by Record Type

### Crunkcore / electro-hop (3OH!3, ISMFOF dance songs)
Crunk/hip-hop drums, 808 or synth bass, blown-out synth lead, chanted or
rapped vocal; guitars as a chorus support. 130–160 BPM.

### Electronicore (Attack Attack!, Breathe Carolina, ISMFOF heavy songs)
Drop-tuned guitars L/R, bass doubling, sample-replaced acoustic drums, synth
pads and gated leads, a club-kit dance break, a half-time breakdown;
clean auto-tuned chorus vs. screams. 124–165 BPM.

### Electropop / dance-pop (*The Fame*)
Four-on-the-floor club kit, synth bass, pads and ostinato, a glam guitar riff
on some songs, a big stepwise vocal hook. 115–130 BPM.

### Electroclash / hyperpop (6arelyhuman, Ayesha Erotica)
Distorted, bit-reduced club drums (four-on-the-floor on "CR4CK HOUSE" and
"MENLO PARK"), a one-note or two-note bass, 2000s club synths, a pop-rap
vocal. 116–165 BPM.

### Pop-punk / emo-rap (Lil Lotus, Sueco)
Standard-tuning guitars, 8th-note power strums, pop-punk drive or skank
drums, rap verse into a sung guitar chorus; half-time emo-rap verses. 150–190
written (often felt at half).

### Post-hardcore (Underoath)
Drop D, open-chord clean parts against heavy parts, syncopated drums, synth
pads, screamed and sung vocals; 3/4 sections. 155–185 BPM. See
`genres/post-hardcore.json`.

---

## Frequency Considerations

### Avoid frequency clashing
- Only bass and kick under about 100 Hz. Tune the kick to the song's key
  (Sturgis).
- Guitars and pads fight in the low-mids: move the pad up, or thin the pad in
  heavy sections.
- In a dance break, drop the guitars so the synth bass has the low end alone.

### Space by role
| Role | Space |
|---|---|
| Kick, 808, bass | Sub and low |
| Guitars | Low-mid, panned L/R |
| Pads | Mid, wide |
| Vocal | Centre, upper-mid |
| Lead / bells | High |

Mix note (for the future Cubase/Reaper session builder): Sturgis mixes
through his mastering limiter from the start and keeps the drums "crushed"
(Sound On Sound). That's a mix setting, not MIDI.

---

## Track Assignment Tips for MIDI

### Minimal arrangement (4 tracks)
1. `drums` (or `electronic_drums`)
2. `bass` / `synth_bass`
3. `rhythm_guitar` (band) or `chords` (club)
4. `melody`

### Full arrangement (7–9 tracks)
1. `drums` (acoustic layer)
2. `electronic_drums` (club layer for verses/breaks)
3. `bass`
4. `synth_bass` (dance break, 808 hits)
5. `rhythm_guitar` (doubled L/R with `double_track`)
6. `pad`
7. `chords` (gated / stab synth)
8. `lead`
9. `melody` (vocal, with `double_track` on choruses)

---

## Quick Checklist

- [ ] Guitar tuning matches the key (open low string = tonic)
- [ ] Guitars doubled L/R, two takes
- [ ] Bass doubles the riff in heavy parts
- [ ] Pad/synth carries the 3rd
- [ ] A club-kit layer is ready for the verse or the break
- [ ] Each section changes which layers play
- [ ] No orchestral instruments unless asked
