# Rhythm Patterns Reference (default)

Built from the reference albums (`reference-albums.md`). The grids are the
most common one-bar drum pattern in each section of the Songsterr
transcriptions, copied as measured. A genre profile in `genres/` overrides
anything here. The textbook version is in
`classical-reference/rhythm-patterns.md`.

## Contents
- Time signatures and odd bars
- Felt vs. written tempo
- Tempo by record
- The eight groove families (measured grids)
- Section-by-section drum moves
- Syncopation that defines these records
- Note duration reference (MIDI)
- Groove tips and common mistakes

**How to read a grid:** 16 steps = one 4/4 bar of 16th notes.
```
Step:  1 e + a 2 e + a 3 e + a 4 e + a
       x . . . x . . . x . . . x . . .   = hit on every beat
```
`kick` / `snare` / `hh` (closed hat) / `oh` (open hat) / `ride` / `china`
match the drum lanes in `genres/_schema.json`. Claps sit in the snare lane
on the electronic records.

---

## Time Signatures and Odd Bars

| What | How often | Where (measured) |
|---|---|---|
| 4/4 | Almost everything | 53 of 65 transcriptions stay in 4/4 the whole way; 7 more are 4/4 except for a single odd bar |
| Whole song in 3/4 | Rare, for a waltz-feel ballad | Sueco "Today" (74 bars of 3/4); Underoath "Some Will Seek Forgiveness" (128 bars) |
| A section switches 4/4 → 3/4 | One heavy section | Underoath "Down, Set, Go" (68 bars 4/4, 74 bars 3/4), "The Impact of Reason" (57 bars 3/4) |
| One short bar (2/4 or 1/8) | A pickup or a stop before a section hits | ISMFOF "Brief Interviews", "Crank That"; Lil Lotus "Over and Over Again"; Underoath "I'm Content With Losing" |
| One long bar (5/4) | A held stop before a drop | Attack Attack! "A for Andrew" |
| A run of 1-beat bars | A stutter / tape-chop effect | Attack Attack! "AC-130": eight 1/4 bars plus two 2/4 bars |

**Default:** write 4/4. Add one 2/4 bar before a big entrance, or a 1/4
stutter run going into a breakdown. Switch a whole section to 3/4 only for
a deliberate heavy or waltz moment. Use the section `meter` binding.

---

## Felt vs. Written Tempo

The same song can be "97 BPM" or "190 BPM". It depends on whether you count
the half-note pulse a listener nods to or the grid the drummer reads.
Measured felt-vs-written pairs:

| Song | Felt (audio) | Written (tab) | Ratio |
|---|---|---|---|
| Sueco "Loser" | 97 | 190 | 2× |
| ISMFOF "Brief Interviews with Hideous Men" | 65 | 130 | 2× |
| Lady Gaga "Eh, Eh" | 188 | 93 | ½× |
| Breathe Carolina "Lauren's Song" | 148 | 74 | ½× |
| ISMFOF "Things That Rhyme with Orange" | 112 | 168 | 1.5× |
| Attack Attack! "Sexual Man Chocolate" | 83 | 124 | 1.5× |
| Sueco "SOS" | 100 | 150 | 1.5× |
| Attack Attack! "A for Andrew" | 160 | 120 | 4/3 |

**Rule:** pick the tempo for the grid you'll write on, then write the drum
grid that gives the felt pulse (snare on beat 3 = half time; 16th hats =
double time). The drum pattern's `feel` field (`halftime`, `double_time`)
only labels it; the hits are what a listener hears. A pop-punk song at 190 with a snare on beat 3 *feels* like 95. A
dance song at 93 with 16th hats *feels* like 186. A 1.5× ratio is a song
built on a dotted or triplet grouping. Don't "correct" it.

---

## Tempo by Record

Written tempo where a tab exists, otherwise measured from the audio.

| Record | Lowest | Median | Highest | Character |
|---|---|---|---|---|
| Lady Gaga — The Fame | 60 (ballad) | 116 | 130 | Club tempo, very tight band (115–130) |
| Breathe Carolina — Hell Is What You Make It | 74 | 123 | 157 | Trance/club 123–140 |
| Ayesha Erotica — Precum | 109 | 130 | 171 | Club 116–130, plus fast tracks |
| Attack Attack! — self-titled | 113 | 130 | 190 | 124–165 plus pop-punk 190 |
| 6arelyhuman — Digital Dancer | 79 | 143 | 170 | Fast club: 130, 143–165 |
| 3OH!3 — Want | 74 | 145 | 160 | 130–160 crunk/electro |
| Sueco — It Was Fun While It Lasted | 94 | 144 | 190 | 94–100 felt (150–190 written) |
| ISMFOF — You Can't Spell Slaughter… | 85 | 148 | 215 | Everything from crunk to blast |
| Lil Lotus — ERRØR BØY | 78 | 117 | 190 | Pop-punk 150–190 / emo-rap 93–125 |
| Underoath — They're Only Chasing Safety | 85 | 170 | 185 | Fast: 156–185 |

**Default ranges by job:**
| Job | BPM |
|---|---|
| Club / electropop / electroclash (four-on-the-floor) | 115–130 |
| Crunk / emo-rap / trap-leaning verse | 70–100 felt (140–200 written in half time) |
| Electronicore / crunkcore band songs | 125–165 |
| Pop-punk drive | 150–190 |
| Post-hardcore / screamo | 155–185 |
| Ballad | 60–80 |

### Tempo changes inside a song (measured)
- Ritardando into the last hit: "Smokahontas" 135 → 120 over the final bar.
- Slower section: ISMFOF "Crank That" 138 → 145 → 103. "But the Nuns Are
  Watching" 215 → 115. Sueco "SOS" 150 → 120 → 150.
- Speed-up: Sueco "Motel 666" 144 → 180. ISMFOF "Reese's Pieces" 98 →
  138 → 140.
- Drift down: Sueco "Today" 105 → 95 → 90.

Use the section `tempo` / `tempo_scale` bindings.

---

## The Eight Groove Families (Measured Grids)

### 1. Four-on-the-floor dance
Kick on every beat, clap/snare on 2 and 4. The hat sets the flavour.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x . . . x . . . x . . . x . . .
snare/clap       . . . . x . . . . . . . x . . .
16th hats        x x x x x x x x x x x x x x x x   Breathe Carolina "Blackout" (106 bars)
off-beat open    . . x . . . x . . . x . . . x .   Gaga "The Fame", "Summerboy", "Disco Heaven" hats
8th open + ride  x . x . x . x . x . x . x . x .   Gaga "Poker Face" chorus (open hat 8ths, ride on top)
```
Where: The Fame (every club track), "Blackout", Attack Attack! "Sexual Man
Chocolate" chorus, ISMFOF "Reese's Pieces" verses (79 bars), 6arelyhuman
"CR4CK HOUSE", Ayesha Erotica "MENLO PARK".

### 2. Half-time / crunk
Snare on beat 3 only, or kick on 1 and 3 with a 2-and-4 backbeat over a
slow pulse.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x . . . . . . . . . . . x . . .   ISMFOF "Crank That" chorus (48 bars)
snare            . . . . . . . . x . . . . . . .
hh               x x x x x x x x . . x . x . x .   3OH!3 "Chokechain": kick on 1 only, snare on 3
```
```
kick             x . . . . . . . . . x . . . . .   3OH!3 "Don't Trust Me" verse
snare            . . . . x . . . . . . . x . . .
hh               x . . . x . . . x . . . x . . .   (quarter-note hats: sparse verse)
```
```
kick             x . . . . . . . x . . . . . . .   Gaga "Just Dance" verse, "Money Honey";
snare            . . . . x . . . . . . . x . . .   3OH!3 "Starstrukk" (16th hats on top)
```

### 3. Pop-punk 8th-note drive
The kick fills the gaps around a 2-and-4 snare. This is the most common
fast-rock bar in the set.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x . x . . . x . x . x . . . x .
snare            . . . . x . . . . . . . x . . .
```
Where: Sueco "It's Going Good!", Attack Attack! "Renob, Nevada" (148
bars), "A for Andrew", Underoath "Moving for the Sake of Motion" (148 bars),
"It's Dangerous Business" chorus. Hats or ride play straight 8ths on top.

### 4. Skank / thrash beat (fastest songs)
Snare on every beat, kick on every "and". This is the punk polka at 165–190.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             . . x . . . x . . . x . . . x .
snare            x . . . x . . . x . . . x . . .
```
Where: Underoath "Young and Aspiring" (128 verse bars), Lil Lotus "Over
and Over Again", Underoath "Writing on the Walls" pre-chorus (half-speed
variant: snare on 1 and 3 only).

### 5. Double-kick drive (heavy choruses)
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x . x . x . x . x . x . x . x .   Attack Attack! "Turbo Swag" chorus (49 bars),
snare            . . . . x . . . . . . . x . . .   "Fumbles O'Brian" chorus (35 bars)
```
```
kick             x x x x x x x x x x x x x x x x   ISMFOF "Beauty Is in the Eyes…" intro:
snare            x . x . x . x . x . x . x . x .   16th kicks, 8th snare (blast-style)
```

### 6. Breakdown (kick locked to the guitar chugs, snare on 3)
The kick plays the chug rhythm. The snare drops to beat 3 (half time). The
crash or china marks the downbeats.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x x x x x . . . . . . . . . . .   ISMFOF "Things That Rhyme with Orange"
snare            . . . . . . . . x . . . . . . .
kick             x . x . x . x . . . x . x . x .   ISMFOF "But the Nuns Are Watching"
snare            . . . . . . . . x . . . . . . .
kick             . . x . . . x . x . x . . . x .   Attack Attack! "Smokahontas" breakdown
snare            . . . . x . . . . . . . x . . .   (+ open hat on every beat)
kick             . . . . . . . . . . . . . . x .   Underoath "Writing on the Walls": almost
snare            . . . . . . . . x . . . . . . .   empty bar, the space IS the breakdown
```
The `riff` composer's chug styles plus drum `kick_follow` produce this.
Write the riff first (`layer_order`).

### 7. Techno / dance break inside a band song
A band song drops into a club bar for 8–36 bars, then slams back into
guitars.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x . . . . . . . x . . . . . . .   Attack Attack! "Smokahontas" Techno Break
snare            . . . . x . . . . . . . x . . .   (36 bars across the song)
hh               x x . . x x . . x x . . x x . .
oh               . . x . . . x . . . x . . . x .
```
Section names in the tabs say it outright: "Techno Break", "Dub Step
Break", "Techno Pop Break", "Hip Hop Intro". Use an `electronic_drums`
layer (`"layer": "electronic"` patterns) or a `dance-break` section.

### 8. Syncopated club clap
The clap sits on the "and" of 2 and 4 instead of on the beat, and the kick
pushes the end of the bar.
```
                 1 e + a 2 e + a 3 e + a 4 e + a
kick             x . . . . . . . x . x . x . . .   Lady Gaga "Paparazzi" (whole song)
snare/clap       . . . . . . x . . . . . . . x .
```

---

## Section-by-Section Drum Moves (Measured)

| Section | Default move | Evidence |
|---|---|---|
| Intro | Either half the groove (kick 1 & 3, snare 3) or the full chorus groove as a teaser | "Fumbles O'Brian", "Brief Interviews" (half); "The Fame" (full four-on-floor) |
| Verse | Thinner than the chorus. Sparse kick, quarter or 8th hats, or kick only on 1 | "Don't Trust Me" verse 1.8 kicks/bar vs chorus 5.0; "Just Dance" verse kick on 1 & 3 |
| Pre-chorus | A build: straight quarter kick, hats filling up | "Turbo Swag" pre-chorus kick x...x...x...x... with 16th hats; "Poker Face" pre-chorus four-on-floor |
| Chorus | The densest bar: four-on-floor, double kick, or 16th hats | "Don't Trust Me" chorus kick x...x...x...x.x. + 16th hats; "Turbo Swag" 8th double kick |
| Breakdown | Half time, kick on the chugs, crash/china on the downbeats | See family 6 |
| Break / interlude | Switch genre: techno, dubstep or hip-hop bar | See family 7 |
| Outro | Half-time with a china on beat 3, or end on one kick | ISMFOF "Ravenous", "Brief Interviews" (china on 3); Underoath "Reinventing" (single kick) |

**Kick density rises verse → chorus.** Measured kicks per bar: "Don't Trust
Me" 1.8 → 5.0; "Just Dance" 2.4 → 4.0 (bridge four-on-floor); ISMFOF "Crank
That" 0.8 → 2.2. Encode this with section `energy` plus per-section
`drum_pattern`.

---

## Syncopation That Defines These Records

| Type | Pattern | Where |
|---|---|---|
| Off-beat open hat | `oh: ..x...x...x...x.` | Every Fame club track, "Smokahontas" break, "Sexual Man Chocolate" chorus |
| Pushed kick before beat 1 | kick on the last 16th (`..............x.`) | "Don't Trust Me" chorus (`x...x...x...x.x.`), Underoath breakdowns |
| Off-beat clap | clap on the "and" of 2 and 4 | "Paparazzi" |
| Kick on "and of 1" + 3 | `x.x.....x.x.....` | Underoath "Reinventing Your Exit" verse, ISMFOF "Beauty…" break |
| Chug-locked kick | kick = guitar rhythm | Every breakdown |

---

## Note Duration Reference (MIDI)

| Name | Beats (4/4) | MIDI Duration |
|------|-------------|---------------|
| Whole | 4 | "1" |
| Half | 2 | "2" |
| Quarter | 1 | "4" |
| Eighth | 0.5 | "8" |
| Sixteenth | 0.25 | "16" |
| Dotted Half | 3 | "d2" |
| Dotted Quarter | 1.5 | "d4" |
| Dotted Eighth | 0.75 | "d8" |
| Triplet Quarter | 0.67 | "4t" |
| Triplet Eighth | 0.33 | "8t" |

Measured note lengths: guitar and bass parts are mostly 8ths and 16ths
(`1/2` and `1/4` beat). Pads and held synth chords are whole notes. Sung
vocals are mostly 8ths and quarters. Rapped vocals are 16ths (7–10 notes
per bar).

---

## Groove Tips

1. **Lock to the grid.** Joey Sturgis (Attack Attack!): "Everything's to a
   click", and every note is checked "on the grid, on time". The drums are
   sample-replaced. Keep `humanize.timing` at or near 0 for electronicore,
   crunkcore and club tracks. Vary velocity, not timing.
2. **Contrast comes from switching grooves, not from drifting tempo.** A
   song moves between families: dance verse → double-kick chorus →
   techno break → half-time breakdown.
3. **Leave space before the drop:** one empty or short bar, a 1/4 stutter,
   or a single hit and silence (section type `stop`).
4. Hats carry the energy level: quarters (verse) → 8ths → 16ths (chorus).

### Common mistakes
- ✗ One drum pattern for the whole song.
- ✗ Classical "rubato" or loose timing on a club or electronicore track.
- ✗ Snare on 2 and 4 in a breakdown. It belongs on 3.
- ✗ Reading a 190 BPM half-time song as "too fast" and halving it.
