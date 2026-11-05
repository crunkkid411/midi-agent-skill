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
- **UNVERIFIED**: which articulation those quiet notes use.

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

## 4. Reading one of his projects (gotchas)

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

## 5. Still unknown

- What is on Gtr Lead.
- The articulations on unmarked notes beyond the two spot checks.
- The quiet notes in the ghost passage.
- The Serum presets on the Synths folder tracks (Saw Bass, synth 1/2, Lead synth 1/2, dirty
  synth, strings, supersaw, Pluck). Jordan thinks they are usable presets, but they were not
  inspected.
