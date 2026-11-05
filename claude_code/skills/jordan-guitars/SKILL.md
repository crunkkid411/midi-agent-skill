---
name: jordan-guitars
description: How Jordan actually programs guitars in Cubase - Odin III tracks, his "ODIN 3 HJ 2" keyswitch map, bus routing, articulation and velocity habits, pitch-bend use. Read before writing guitar MIDI for him, building a guitar template, or reading guitar parts out of one of his projects.
---

# How Jordan uses the guitars

Source: his TEST-FILE.cpr (a working copy only, never his real project), read on 2026-11-05.
I read the expression-map files, the Odin settings saved in the project, a MIDI export of
every guitar track, and spot-checked notes in the Key Editor. Anything not checked is marked
**UNVERIFIED**.

## 1. The instrument and the keyswitch map

Every guitar track is **Solemn Tones Odin III** (8-string baritone). The keyswitches come from
a **Cubase Expression Map named "ODIN 3 HJ 2"**. They do not come from Odin's own keyswitch
list: Odin's "Keyswitch List" is 0 and "Keyswitch Latching" is off on every instance.

The map file is `C:\Users\only1\Documents\Cubase Maps and being extra\Cubase Expression Maps\ODIN 3 HJ 2 3.expressionmap`.
`ODIN 3 HJ.expressionmap` and `ODIN 3.expressionmap` in the same folder are byte-for-byte the
same map under the same name.

Every keyswitch is sent at velocity 120. Cubase names notes with C3 = 60.

| MIDI | Cubase | Articulation | | MIDI | Cubase | Articulation |
|---|---|---|---|---|---|---|
| 24 | C0 | Alt Pick | | 12 | C-1 | **Dwn Op PM** (down, open palm mute) |
| 25 | C#0 | **Dwn Pick** | | 13 | C#-1 | Hmr On |
| 26 | D0 | Up Pick | | 14 | D-1 | Pull Off |
| 27 | D#0 | Alt Mute Semi | | 15 | D#-1 | **Up Op PM** |
| 28 | E0 | **Dwn Mute Semi** | | 16 | E-1 | Bnd Up Fst |
| 29 | F0 | Up Mute Semi | | 17 | F-1 | Bnd Up Slw |
| 30 | F#0 | Alt Mute Cls | | 19 | G-1 | Scrapes |
| 31 | G0 | Dwn Mute Cls | | 20 | G#-1 | Trem |
| 32 | G#0 | Up Mute Cls | | 21 | A-1 | Nat Harm |
| 33 | A0 | Alt Mute Dead | | 22 | A#-1 | Pinch |
| 34 | A#0 | Dwn Mute Dead | | 23 | B-1 | **NSTY Pinch Hrm** |
| 35 | B0 | Up Mute Dead | | 0-7 | C-2..G-2 | Slides: Slw From up / From Dwn / To Dwn / To Up, then Fst ×4 in the same order |
| 101 | F6 | String 8 | | 9 / 10 / 11 | A-2 / A#-2 / B-2 | Fingr Pluck Open / Fingr Pluck Dead / Thumb Slap |

Strings 7 to 1 are slots with no action ("Open"). Bold = the ones he actually uses (section 3).
Machine-readable copy: `instruments/odin-iii.json`.

**Playable notes stay at MIDI 36 and above**, because everything below 36 is a keyswitch.

## 2. Tracks and routing (Guitars folder)

- **Busses**: Guitars All, GTR Rythm Bus, GTR Leads Bus, GTRs D+R FX, Bass Bus.
- **guitar 1 / guitar 2**: the main double-tracked rhythm guitars.
  - He keeps them **disabled** to save CPU and plays their rendered audio tracks, "guitar 1 (R)" and "guitar 2 (R)".
  - To see their MIDI: right-click the track, then "Enable Track".
  - guitar 1 has three ToneForge-family amp inserts and its fader at -20 dB.
- **Gtr Clean / Gtr_Clean_2**: clean layers. "Gtr Clean (R)" is the render.
- **Gtr Lead**: lead guitar. **UNVERIFIED**: nothing from it showed up in the MIDI export.
- **ODIN Ambient Lead** (plus its (R) render) and **GTR Synth** (disabled).
- **bass guitar** (+ (R)): this also uses an expression map; its text marks match the guitars.

### Odin settings

**Same on every instance**:
- Channel Choice 2, Manual String Choice 7, String Mode 1.
- Guitar 01 / 02 pan -50 / +50, pan linked.
- Slide Speed 77, Sustain 1, Sustain Mutes 1, Output Gain 0.86.
- Pitch Bend Range 2.
- Auto Sustain off, Human Left Hand off.

**Per-track settings** (copy them exactly when building a template):

| Setting | guitar 1 | guitar 2 | Gtr Clean / Clean_2 | Gtr Lead | Ambient Lead |
|---|---|---|---|---|---|
| Enforce Pick Attack | on | on | off | off | off |
| Pick Attack Impact | 2.01 | 1.85 | 1.78 | 1.78 | 1.03 |
| Human Mute | 87 | 87 | 10 | 0 | 87 |
| Humanize | 2 | 3 | 4 | 3 | 4 |
| Auto Chord Delay | 0.006 | 0.018 | 0.16 | 0.03 | 0.10 |
| Force Vibrato | off | off | on | on | on |
| Vibrato amp / speed | 0 | 0 | 11.6 / 1.19 | 28.3 / 4.05 | 13.3 / 1.43 |
| Soften Attack | 0 | 0 | 79 | 0 | 0 |
| Pickup Choice | 0 | 1 | 0 / 2 | 0 | 2 |
| Auto String Choice | 5 | 0 | 6 | 3 | 5 |
| Tuning Scale Choice | 4 | 0 | 11 | 11 | 4 |

In plain terms: the rhythm guitars are tight and hard-picked with no vibrato. The clean and
lead tracks have soft attacks, strum their chords slowly, and use vibrato.
**UNVERIFIED**: which tuning the numbers 0, 4 and 11 mean inside Odin.

## 3. How he plays them

### Chug riffs (guitar 1 and guitar 2 together)

Evidence: bars 11-18 and 79-94.
- 8th-note palm mutes on the low strings (MIDI 44-51), mostly 2-note power-chord shapes.
- **Dwn Op PM** on almost every note.
- **Up Op PM** on the two off-beat pickups of each bar (about 2 per bar).
- **Velocity is flat at 98**. He does not vary velocity inside a chug.
- **One NSTY Pinch Hrm** at the peak of the riff (bar 89, velocity 117). It is rare: one per section.
- **Pitch-bend wobble** on the last beat of a 4-bar phrase (bars 14, 18, 82, 90).
  - It is a whammy-style dip and rise, about +2 to -2 semitones, made of 40-50 messages.
  - It ends back at 0 or on a slight dip, and the next bar resets it.
- guitar 2 copies guitar 1's articulations exactly but sits on slightly different low notes.
  That makes it a real double, not a copy-paste.

### Melodic and lead sections (guitar 1 high, guitar 2 low)

Evidence: bars 47-61 and 70-77.
- Single notes or 2-3 note shapes in 8ths.
- Base velocity 84, with accents at 117.
- Key Editor checks:
  - a lead note (F4) = **Dwn Mute Semi**;
  - a chord note (A#3) = **Dwn Pick**.
- guitar 1 plays the melody (MIDI 58-83) while guitar 2 plays a harmony underneath (47-66).
- Gtr Clean doubles the same bars with 4-note chords at velocity 67-117.

### Big chord sections

Evidence: bars 129-136.
- guitar 1 and Gtr Clean play 5-note chords in 8ths.
- They are palm-muted (Dwn Op PM) in alternate bars, at velocity 81-117.

### Held chords and intros

Evidence: bars 1-7 and 62-69.
- Whole-note or longer chords, 4 notes, at velocity 117 or 81.

### Ghost/dynamic passage

Evidence: guitar 1, bars 218-229.
- 16th-note single notes on the low strings, velocity 15-100 (some notes sit at 15-23).
- Velocity 15-24 is Odin's factory closed-mute band (section 6), so the quiet notes are almost certainly velocity-driven closed mutes. Not checked by ear in Cubase.

### Gtr_Clean_2

- Sparse: long held notes (6.5 beats) at velocity 85.
- Then an 8th-note 2-note figure at a flat 97.
- Then long notes at 117 in the big sections.

### Summary rules for a generator

| Musical intent | Keyswitch | Velocity |
|---|---|---|
| chug / palm mute | Dwn Op PM (12), Up Op PM (15) on off-beat pickups | flat ~98 |
| accented or ringing chord | Dwn Pick (25) | 117 (softer 81-84) |
| lead / melody line | Dwn Mute Semi (28) or Dwn Pick | 84 base, 117 accents |
| riff climax | NSTY Pinch Hrm (23), once | 117 |
| phrase ending | pitch-bend wobble ±2 st on the last beat | - |

## 4. Why each keyswitch is chosen (pick direction and genre)

### 4a. Measured in Jordan's TEST file (strongest evidence)

I counted where every Dwn/Up Op PM mark falls in the bar, on guitar 1 and guitar 2:

| Track | Down-strokes | Up-strokes |
|---|---|---|
| guitar 1 | 207: 108 on the beat, 99 on the "&" | 30: 16 on the "e", 14 on the "a" |
| guitar 2 | 157 on the beat or "&" | 46 on the "e" or "a", 2 exceptions |

Every up-stroke came 1/16 after the previous note.

**Rule: the picking hand moves on a 16th-note grid.**
- A note on the 8th grid (the beat, or the "&") is a **down-stroke**.
- A note on the 16th in between (the "e" or the "a") is an **up-stroke**.
- A note on the beat is never an up-stroke, so the first note of a section, a chorus or a bar is always down.
- Plain 8th chugs are therefore all down-strokes.

**Why** (players' and teachers' reasons, plus my inference where marked):
- **All-down 8ths sound heavier and tighter.** Every hit has the same attack.
  - Metal and thrash players down-pick 8ths for a tighter, heavier result ([Wikipedia: Downpicking](https://en.wikipedia.org/wiki/Downpicking)).
  - It is credited for Hetfield's "Master of Puppets" ([Roadie](https://www.roadiemusic.com/blog/how-to-master-downpicking/)), for Johnny Ramone's whole sound ([Premier Guitar](https://www.premierguitar.com/lessons/how-to-jam-out-like-the-ramones)), and for thrash's uniform attack ([Guitar World / Dave Davidson](https://www.guitarworld.com/lessons/thrash-course-revocations-dave-davidson-using-fast-down-picking-execute-powerful-riffs-and-signature-parts)).
- **Up-strokes only where speed forces them.** A hand cannot down-pick 16ths at metal tempos.
  - So it swings down-up, and the in-between 16th lands on the way up.
  - Down-picking costs about twice the hand motion of alternate picking ([Wikipedia](https://en.wikipedia.org/wiki/Downpicking)).
  - Keeping the down-stroke on the strong 8th positions keeps the accent on the beat. (This last point is my inference from the data.)
- **Section starts are down** because they fall on the beat. A down-stroke is the natural accent.
  - Every section start in the file (bars 11, 47, 79 and 129) is a down articulation: Dwn Op PM or Dwn Pick.

The engine now does this automatically for Odin III:
- `instruments/odin-iii.json` sets `"pick_grid": 0.5` (beats) and an `upstrokes` table (down_pick → up_pick, down_open_palm_mute → up_open_palm_mute, ...).
- Each note is snapped to the nearest 16th first, because humanize moves notes a few milliseconds.
- The same rule is in Python (`InstrumentMap.directed`) and in the plug-in (`InstrumentMap::directed`). Both are tested.

### 4b. Where the generic teacher rule is WRONG for his music

The beginner strumming rule is "down on the beat, up on the &" ([premierguitar strumming](https://www.premierguitar.com/guitar-strumming-patterns), [guitarlessonworld](https://www.guitarlessonworld.com/?p=134)). It treats the **quarter note** as the grid.
- Applied to metalcore 8th chugs, it would put half the chugs on weak up-strokes.
- That is the kind of generic advice that broke metalcore before.
- His grid is the **8th note** (`pick_grid` 0.5), not the quarter (1.0).
- Use the quarter-note grid only for strummed pop/acoustic/funk 8th strumming, and only after checking a real source for the genre.

### 4c. Other genres (outside sources, weaker until checked on a real project)

| Genre / part | Pick direction | Why | Confidence |
|---|---|---|---|
| Metal, metalcore, thrash 8th chugs | all down | uniform, heavy attack | high (his file + Hetfield + thrash lessons) |
| Metal 16th runs and gallops | down on the 8th grid, up on the 16ths between | speed; accent stays on the beat | high (his file) |
| Punk (Ramones line) | all down 8ths, even at fast tempos | "a timing mechanism" (Johnny Ramone); aggression | medium ([Guitar World](https://www.guitarworld.com/features/the-100-greatest-guitarists-of-all-time/9), [Premier Guitar](https://www.premierguitar.com/lessons/how-to-jam-out-like-the-ramones)) |
| Pop / acoustic / funk strumming | down on the beat, up on the "&" (16ths: constant down-up) | the hand keeps a steady down-up swing like a metronome | medium (lesson sites) |
| Ska / reggae skank | chop on the off-beat (2 and 4 in reggae, every "&" in ska), muted and staccato, usually described as an up-stroke | backbeat | **disputed**: Guitar Techniques calls "skank = up-stroke" a myth ([Wikipedia: Ska stroke](https://en.wikipedia.org/wiki/Ska_stroke), [Guitar World](https://guitarworld.com/lessons/bob-marley-reggae-rhythm-guitar)) |
| Breakdowns / groove | a run of muted chugs opened by an accented open chord; off-beat stabs over muted strings | contrast between muted and open strings makes the stab punch | medium ([Riffhard](https://www.riffhard.com/how-to-play-metalcore-breakdowns/), [Guitarchalk](https://www.guitarchalk.com/metal-rhythm-guitar-patterns/)) |
| Pinch harmonics | on chosen accent notes, often a phrase's last note, with vibrato; death metal uses them inside riffs | cut through the mix | medium ([Riffhard](https://www.riffhard.com/?p=36284), [Guitar World](https://www.guitarworld.com/lessons/techniques/pinch-harmonics)). Jordan: about one per section, at the riff's peak |

Solemn Tones' own demonstrations are in section 6.

## 5. Amp sims

The full list of Jordan's installed amp sims is in `resources/amp-sims.json`.
- The TEST file uses Toneforge Jeff Loomis, Toneforge Menace, Toneforge Ben Bruce, Archetype Tim Henson X, and JST BG-Guitars on the bus.
- The presets used were "Loomy song_main rhythm", "Attack Attack! - All My Life - Rhythm", "Bogren Dual Gains" (lead), and "Breakup All In" / "Sparkle All In" (clean). All of them were tweaked.

## 6. What Solemn Tones (the makers of Odin III) show

Jordan picked these videos as the trusted source (2026-11-05). For new genres and techniques,
search the Solemn Tones YouTube channel first. Full notes, with timestamps and SAW/HEARD tags,
are in `docs/research/odin-iii-videos.md`.

Videos:
- [eVAyRiLNFcg](https://youtu.be/eVAyRiLNFcg): Solemn Tones analysing Nik Nocturnal's deathcore MIDI.
- [bu30PxxV3kk](https://youtu.be/bu30PxxV3kk): Xander Raymond Charles, a metalcore breakdown.
- [klAbrHifvnY](https://youtu.be/klAbrHifvnY): official walkthrough.
- [gIMUmzriLGA](https://youtu.be/gIMUmzriLGA): song playthrough.
- [qcjFhbPp5wc](https://youtu.be/qcjFhbPp5wc): a full Motionless In White-style metalcore cover. Its MIDI is in the description; I measured it.
- [THly6-rXk-U](https://youtu.be/THly6-rXk-U): realism tips.

### Odin's factory velocity map

This applies whatever the keyswitches are (seen on screen in bu30PxxV3kk at 5:25 and qcjFhbPp5wc at 2:06):

| Velocity | Sound |
|---|---|
| 127 | pinch harmonic |
| 126-35 | normal pick |
| 34-25 | round, "bloomy" palm mute |
| 24-15 | closed (tight) mute |
| 14-0 | dead mute |

**Engine rule (built):**
- `velocity_limits` in `odin-iii.json` keeps every normal note between 35 and 126.
- An accent can never turn into an accidental pinch harmonic, and a quiet note can never turn into a mute.
- Only pinch articulations may reach 127. Dead-mute keyswitch notes are held at 8-14.

### Measured in the official full song (`ODIN 3 MIDI.mid`, 1,390 notes, 101 BPM metalcore)

- **Normal notes at a flat velocity of 125**: 827 notes. Their dynamics come from mute bands, not from loudness.
- **Closed mutes at velocity 20**: 151 notes, mostly on the "&" and "a". Pinch harmonics at 127: 24 notes. Bloomy palm mutes at 25-34: 57 notes.
- **Velocity-1 notes**: 44, mostly on the "&". They fill the gaps between chugs, so the part "sounds less edited" (2:32).
  - A velocity-1 note right on the beat fakes a hard hand mute (11:18).
- **Barely any keyswitches**: only 4 different keyswitch notes.
  - The pinch keyswitch (22) is used 8 times.
  - The factory **B0 dead-mute "rake"** is used 4 times, on the last beat before a new section. "Pretty common in metal" (2:05, 8:03, 10:01).
  - String-choice keys (102 / 103) force melody notes onto thicker strings for warmth. They are placed slightly before the note (7:15).
- **Odin alternates the pick direction by itself**: no down- or up-pick keyswitches are used anywhere.
- **No timing humanize**: 1,381 of 1,390 notes sit on the 16th grid. No pitch bend.
- Chord top notes are kept low (about 55) to darken the guitar and make room for the orchestra. Chorus top notes rise to about 80 (3:13, 14:25).
- The end of a chorus fades through velocities 30, 30, 25, 20, so "the energy [is] sucked out of the room" before the verse (9:22).
- The regular pinch was chosen over the nasty pinch because it matched the original's pitch (3:53-5:09).

### From the deathcore and breakdown videos

- **"Chugging and palm mutes can be fully done with velocities"** (eVAyRiLNFcg 1:13).
  - About 28-30 = bloomy palm mute on slower power-chord chugs.
  - About 10-14 = dead mute on fast 16ths. Mix both inside one riff.
- **Release mute**: a tiny dead note (velocity about 14) at the end of a ringing note, right before silence. It imitates the right hand slamming into the bridge (5:00).
- **Fast breakdowns**: down, up on every 16th, and **each burst starts on a down-stroke** (bu30PxxV3kk 5:57, 7:26).
  - This is the closest the videos come to "down pick at the start of a section". It matches Jordan's 8th-grid rule.
- **Bends**: long notes start pre-bent and fall back; roots stay unbent. Tremolo notes bend slowly up. Bend intensity 2 is the default, 3-4 is nastier.
- **Low tremolo lines**: force them onto the lowest string for warmth.
- **Pull-offs inside picked runs**: they remove some pick attacks.
- **Double tracks**: two copies panned hard left and right, one tight (humanize about 13%) and one sloppy (about 55%).
- **For deathcore, "disgusting" beats "realistic"** (6:44). Treat realism as a choice, not a rule.

### From the official walkthrough and the Paramore pop-punk video (klAbrHifvnY, THly6-rXk-U)

- **Pick-direction keyswitches are the exception.** Down/Up keys are only used "to ensure the alternative picking hits exactly when we want it to", at specific notes (Architects playthrough). They are not a section-start rule.
  - "Down at the start of a chorus" is true in Jordan's files, because his section starts fall on the 8th grid.
  - It is not a Solemn Tones rule.
- **Pop punk** (173 BPM): "your work on your guitar MIDI will be far far less" (9:32).
  - Choruses are open chords at a flat 125 with no humanize (7:23).
  - Verse chugs sit all at 34, the bloomy palm mute (4:11).
  - Leads mix closed mutes at 15 with normal picks at 70-80. That is deliberately "messy", because "it's not trying to be... perfection metalcore style" (5:25).
- **Build into a chorus**: a velocity crescendo 34 → 65 → 70 → 85 → … → 125 on the chords, walking out of the palm-mute zone into open chords (6:34).
- **Scrapes & Noises keyswitch** (factory G-1):
  - put it in some rhythm rests, on beat 2 or 4;
  - not every rest, never twice the same, never on leads;
  - "too much... not gonna sound very professional" (2:01-5:05).
- **Slide down**: the slide keyswitch goes on the last note of a lead phrase, on beat 4 of the bar before the next section (3:21, 6:22).
- **Cleans**: velocity under about 60 (55-59 is "very gentle"); notes overlap and ring (8:23).
- **Positions**: each Odin instance is locked to a fretboard position.
  - Leads go on position 4 so they stay "warm... not too shrill" (3:47).
  - The String Mod keyswitch is held over a build so it doesn't get brighter (6:52).
- **Short fast pickups inside an 8th chug** become palm mutes; the main pulse stays open (walkthrough 15:00).
- **Fast runs**: use hammer-on/pull-off keyswitches on the notes a human couldn't pick (17:01).
- **No vibrato** on rhythm chords; vibrato on slow lead notes starts after a delay (4:01).
- **Timing**: 100% on the 16th grid in their files. The feel comes from Odin's Humanize knob, Human Noises (80-100) and a -20 ms track delay, not from moving notes.

Solemn Tones channel: https://www.youtube.com/channel/UCwmWmniB4EZGOAwoF-Y4jjQ (136 videos).
- Genre tutorials that come with MIDI files: Architects, Motionless In White, Spiritbox, The Black Dahlia Murder, Dance Gavin Dance, Sleep Token, Three Days Grace, Slipknot, Lorna Shore, Slaughter To Prevail, THALL, Plini.
- Technique videos: "7 Tips For Midi Guitar Realism", Gojira scrapes, slides, delay compensation.
- The full list with links is in `docs/research/odin-iii-videos.md`. Ingest the matching one before building a new genre.

### Jordan's way vs the factory way (both work; his is the default for his songs)

| | Jordan (TEST file, "ODIN 3 HJ 2" map) | Solemn Tones factory |
|---|---|---|
| Pick direction | explicit Dwn/Up keyswitches on the 8th/16th grid | Odin alternates by itself |
| Palm mute | Dwn Op PM keyswitch at velocity 98 | velocity 25-34 / 15-24 / 0-14 |
| Normal velocity | 84-117 | 125 flat |
| Keyswitch notes | his own map (B0 = Up Mute Dead) | factory map (B0 = dead-mute rake) |

**The two maps do not share note numbers.** Never copy keyswitch notes out of a Solemn Tones
MIDI into Jordan's project, or the other way round, without translating them.

**Not built yet** (documented, so a composer can add them):
- the dead-mute rake before a section;
- release mutes before rests;
- velocity-1 gap fills;
- the chorus-end velocity fade;
- pre-bent long notes.

## 7. Reading one of his projects (gotchas)

1. **Enable disabled tracks before exporting MIDI.** Cubase leaves disabled tracks out of
   File > Export > MIDI File. Enable them on the copy only.
2. **Keyswitches never appear in a MIDI export.** Cubase sends them from the Expression Map at
   playback. Read the `.expressionmap` file for the note numbers.
3. Articulations with a text symbol export as **MIDI lyric events**:
   - "." = Dwn Op PM
   - ".." = Up Op PM
   - "..." = NSTY Pinch Hrm
   Articulations shown only as a symbol (the "–" mark = Dwn Pick) export nothing. Check those
   in the Key Editor's Articulations field.
4. In the Key Editor's Articulations dropdown, **clicking the ticked item switches it OFF** and
   leaves the note with no articulation. If that happens, use Edit > "Undo Set Articulation".
5. Instrument tracks cannot send MIDI out to a port. Clicking the output name opens the
   instrument swap list. Use a MIDI track if you need to capture keyswitches live.
6. On his PC the Key Editor opens on the lower monitor.

## 8. Still unknown

- What is on Gtr Lead.
- The articulations on unmarked notes beyond the two spot checks.
- The Serum presets on the Synths folder tracks (Saw Bass, synth 1/2, Lead synth 1/2, dirty
  synth, strings, supersaw, Pluck). Jordan thinks they are usable presets, but they were not
  inspected.
