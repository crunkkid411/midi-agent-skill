# Odin III video research (2026-11-05)

Ingested by helper agents from the videos Jordan picked; SAW = on screen, HEARD = spoken. Summary and engine rules: claude_code/skills/jordan-guitars/SKILL.md section 6.

# Odin III videos - batch A

Sources: the English auto-captions (original track; the watch skill picked a German auto-translation first, so I used `--sub-langs en-orig` instead), plus frames I pulled myself at 1280 px from the downloaded video and looked at.
Tags: **[HEARD]** = what the speaker said (caption text, lightly cleaned). **[SAW]** = what I read on screen. **UNVERIFIED** = I could not confirm it.

---

## Video 1: "Analyzing Nik Nocturnal's FAKE Song (Odin 3 Midi)"
- URL: https://youtu.be/eVAyRiLNFcg, channel **Solemn Tones** (the developer), 8:34
- Description chapters: 00:00 Intro, 00:21 Odin 3 Settings, 00:47 Analyzing Midi
- Context: a Solemn Tones presenter walks through Nik Nocturnal's MIDI for a deathcore / "thall"-style song (Odin III + Sanguine bass). The MIDI was programmed by Nik, not the presenter. What follows is his reading of a working deathcore MIDI file.

### Settings (0:21-0:47)
- [HEARD 0:23] Pickup **Bridge Modern**: "most modern sounding pickup pattern, it gets rid of some of that mid honk".
- [HEARD 0:33] "as for playing I think everything is default".
- [HEARD 0:35] Advanced tab: **attack impact turned down to 0.0 dB**. He does not give a reason. [SAW 0:36] The Advanced tab has: Realign phase, Note ring out, Auto Sustain, Keyswitch Latching, Midi chan. string select; Pick Attack "Enforce" switch plus the attack impact knob; Sliding "Autoslide" switch plus slidespeed (showed 77); Shaper "Bend Intensity" (showed 2); mutes D/S and others D/S knobs. I could not read the on/off state of each switch: UNVERIFIED.
- [HEARD 0:41] Tuning **8-string Double Drop C**, "so this way everything on the neck right here should also display properly".
- [SAW 1:08] Main tab: Amp = **DI**, **Humanize 13%**, Guitar 01/02 pan 50/50, preset "Panned Rhythm...", Active = "Alternate Picked".

### MIDI layout (0:48-1:19)
- [HEARD 0:50] Every Solemn Tones demo MIDI has three zones: "the notes that are actually being played in the middle, the articulation key switches at the bottom, and the string selection key switches at the top" (on the Odin keyboard, the String Mod keys sit at the top end).
- [HEARD 1:13] **"there's not many key switches going on for articulations because a lot of this is just chugging and palm mutes which can be fully done with velocities."** This is the main rule of the video: in deathcore, mute type comes from velocity, not from keyswitches.

### Pitch bend (1:39-2:23, 4:08-4:49)
- [HEARD 1:50] "the first notes, no bending obviously, we're just doing our low root note, but then these bends instantly here from the very first note, right at the top and gradually down back to our zero". He calls them "nasty bend up from downs".
- [SAW 1:46, 1:57] The pitch-bend lane on the long high notes starts high (about 43 in Logic's display) right at note-on, then falls in a curve back to 0 over about a beat. That is a pre-bent note released down. A second, shorter note starts around 21 and falls to 0 the same way. The low root notes under them have no bend.
- [HEARD 2:16] "when you have these notes like tremolo picked and then gradually moved up, that is also very very nasty sounding". So tremolo-picked notes get a slow bend UP.
- [HEARD 4:10-4:47] The bends in Nik's song sound bigger than his playback. He guesses Nik had a higher **Bend Intensity** ("maybe five"). Five is "maybe a bit too much"; 9 "bends into something the normal guitar wouldn't even be able to do... sounds ridiculous"; "maybe two, maybe three, maybe four. I have it on two, that's the default."

### Palm mutes by velocity (2:27-2:47, 3:41-3:55, 4:49-5:17, 6:12-6:24)
- [HEARD 2:30] "these first ones are around 30 velocity, so these are like the **bloomy open palm mutes**, and then these quicker ones here are at **velocity 10**, so these are going to be more the **dead mutes**. The slower ones open bloomy and the shorter ones more dead, more muted."
- [SAW 2:33, 2:39] The bloomy mutes (velocity 30) are 3-note power-chord stacks on longer values (about an 8th or more). The dead mutes (velocity 10) are short 16ths on the single low root right after them.
- [HEARD 3:43] Chugging section: "slower chugs on these, like bloomy, around 30, or in this case 28... while the shorter ones are on 14". [SAW 3:46, 3:52] A 16th-note power-chord chug run. The selected notes read velocity 14 and 100 in different places, so mixed velocities sit inside one run.
- [HEARD 4:53] End of riff: "the bloomier palm mutes, and then for the quicker ones we have 13, so that's closer to a dead mute".
- [HEARD 6:14] New riff: "some variety in palm mutes, like right here this is a bloomy palm mute and then here we have the dead ones, and even some of the shorter ones are also being bloomy, so again some variety". So the rule "short = dead" is not absolute. Nik mixes them.
- How this lines up with the factory velocity map (read off video 2): 34-25 = Palm Mutes, 24-15 = Palm Mutes Closed, 14-0 = Palm Mutes Dead. So 28-30 = "bloomy" palm mute, 13-14 = dead (13 and 14 sit at the top of the dead band), 10 = dead.

### Release mute at the end of a ring-out (4:59-5:17)
- [HEARD 5:00] "when silence happens we see a tiny little palm mute articulation at, I guess, velocity 14, to imitate those release notes. So a very very aggressive kind of muting. That's the kind of sound you will get when you have your right hand just slamming into that bridge to end the note from ringing out."
- [SAW 5:04] A long held low root, then one tiny dark-blue (low-velocity) note at the same pitch right where the long note ends, just before a rest.
- **Generator rule:** before every rest that follows a ringing note, add a very short dead-mute note (velocity about 14) on the same pitch at the end of the note.

### String selection (2:50-3:25)
- [HEARD 2:52] Tremolo-picked passage: "we have another string selection key switch, so this is being played on the eighth string on purpose". The first note is not available on string 8, so it falls back to string 7.
- [HEARD 3:07] When he deletes the keyswitch: "that's very very different... if this key switch is not there this is being played on like the six string, which is a fair bit brighter... that's just not something we're looking for here, you want that nice and warm tone."
- [SAW 2:53/3:08] The string keyswitch is a high note in the top zone (around C#6, velocity 107), with tremolo 16ths below it.
- **Generator rule:** for tremolo or lead lines in the low register, force the lowest string that can play the note, for warmth. The auto string choice goes brighter (string 6).

### Pull-offs (3:27-3:39)
- [HEARD 3:29] "we have pull-offs on some of the picking here to kill that pick attack, to make that run a little more realistic. I've shown this trick in past tutorials as well." The reason is realism: not every note in a run is picked.

### Left-hand mute dissonance (6:31-7:07)
- [HEARD 6:34] "I assume this is meant to be a kind of dissonant left-hand muting the strings, just to have a disgusting percussive kind of thing."
- [SAW 6:35] High dyads (around the C5 octave) as repeated 16ths (4 + 4), at velocity about 29, with notes in the bottom articulation-keyswitch zone at the same time. I could not read which keyswitch: UNVERIFIED.
- [HEARD 6:44] He tries swapping in dead notes at about velocity 10, made longer: "maybe more realistic sounding, but it's definitely less disgusting sounding... if you're going for that disgusting sound then it is really what you wanted." **So the realistic choice is not always the right one for deathcore.**

### Genre notes
- [HEARD 5:20] "in a song like this your guitar is spamming a lot of the same note, it's a lot of rhythmic stuff, so the song is really being made by the effects surrounding that... like Sleep Token or the Doom soundtrack, there's a lot of synth work and atmosphere... your guitar is almost being played like a percussive element."
- [HEARD 6:08] "thall kind of sound", and "super messed up dissonant deathcore vibe" (the latter is the German caption's wording).
- [HEARD 7:36] "definitely not the type of song I would be writing". He is reading Nik's MIDI, not his own.

---

## Video 2: "DON'T BUY ANOTHER GUITAR! TRY ODIN III"
- URL: https://youtu.be/bu30PxxV3kk, channel **Xander Raymond Charles** (a working touring guitarist; sponsored by Solemn Tones, disclosed at 1:04), 16:59
- Context: his own metalcore/breakdown demo in Logic, about 215 BPM in 4/4 [SAW the transport]. Two Odin instances, DI output into Fortin Nameless Suite X, bass is Loki Bass 2.

### Factory keyswitch map (SAW 5:25, 7:15, 7:26: the Keyswitch List panel, read in full)
| Key | Articulation | Key | Articulation | Key | Articulation |
|---|---|---|---|---|---|
| C0 | Alternate Picked | C-1 | Down Open Palm Mute | C-2 | Slide Slow From Up |
| C#0 | Down Picked | C#-1 | Hammer-On | C#-2 | Slide Slow From Down |
| D0 | Up Picked | D-1 | Pull-Off | D-2 | Slide Slow To Down |
| D#0 | Alternate Mute Semi Closed | D#-1 | Up Open Palm Mute | D#-2 | Slide Slow To Up |
| E0 | Down Mute Semi Closed | E-1 | Bend Up Fast | E-2 | Slide Fast From Up |
| F0 | Up Mute Semi Closed | F-1 | Bend Up Slow | F-2 | Slide Fast From Down |
| F#0 | Alternate Mute Closed | F#-1 | *Auto Slide | F#-2 | Slide Fast To Down |
| G0 | Down Mute Closed | G-1 | Scrapes | G-2 | Slide Fast To Up |
| G#0 | Up Mute Closed | G#-1 | Tremolo Picked | G#-2 | *Auto Power Chord |
| A0 | Alternate Mute Dead | A-1 | Natural Harmonic | A-2 | Finger Pluck Open |
| A#0 | Down Mute Dead | A#-1 | Pinch Harmonic | A#-2 | Finger Pluck Dead |
| B0 | Up Mute Dead | B-1 | Nasty Pinch Harmonic | B-2 | Thumb Slap |

- String selection: F6 = String 8, F#6 = 7, G6 = 6, G#6 = 5, A6 = 4, A#6 = 3, B6 = 2, C7 = String 1.
- Velocity triggers: **127 = Pinch Harmonics, 126-35 = Alt Picking, 34-25 = Palm Mutes, 24-15 = Palm Mutes Closed, 14-0 = Palm Mutes Dead.**
- This is the factory map in his session. Jordan's "ODIN 3 HJ 2" custom map may differ, so check it against instruments/odin-iii.json.
- The keyboard header groups the keys as Slides | Slap | Special | Pick & Mutes ... String Mod [SAW].

### Two guitars, double-tracked (3:26-4:35, 8:45-11:29)
- [HEARD 3:31] "two different instances of the plugin, one panned hard right and one panned hard left... guitar 1 solo DI for guitar right... guitar 2 solo DI for guitar left... two different samples... to emulate... two different takes panned hard left and hard right to create that giant stereo feel." [SAW 4:05, 5:25] Presets "Guitar 2 Solo'd" and "Guitar 1 Solo'd"; Pan knobs at 0 inside each instance; panning done on the track.
- [HEARD 4:04] "I have the human knob turned up a little bit, I do want this to be rather djenty and a little bit more stutty [likely 'stuttery'; caption reads 'stoty', UNVERIFIED]... guitar 1 I have a lot more human elements compared to guitar 2... guitar one to be kind of like the sloppy guitar, and then guitar left, guitar 2, more on the nose, more concrete... almost purposefully programmed and chopped up and edited sounding."
- [SAW] **Humanize 55%** on guitar 1 (right, the "sloppy" one) at 5:25 and 7:15. About **13%** on guitar 2 (left, the tight one) at 4:05; that knob label is small.
- [HEARD 9:28] Raising Humanize: "it's not as clean, it's not as fake sounding, which is a good thing to some degree. I would probably back this down a little bit."
- **Deliberate mistake (9:37-11:29)** [HEARD]: on one guitar only he programmed "essentially an F power chord, F C F, but it just doesn't sound clean, it sounds weird... got kind of a clank to it... sounds like 'oh that take could have been done better', but I did that on purpose... whenever I'm playing I don't necessarily want my takes to be perfect, I like happy accidents... if I make a mistake but it isn't that bad I'll leave it in a track more often than not... one guitar is chugging down cleanly and the other one is kind of a little sloppy." I could not read exactly how he made the clank (which notes or velocities): UNVERIFIED.

### Picking direction in fast breakdowns (5:28-7:35)
- [HEARD 5:33] "I've programmed a bunch of different key switches, meaning a bunch of different articulations with your right hand, as well as a little bit of the left like with some bends... it's not just like the same sample every single time, 'cause that wouldn't sound natural, that's not how the guitar sounds."
- [HEARD 5:57] "I'm doing a lot of fast crazy breakdown... and so I wanted to program the **individual up and down stroke, so down up down up**, to emulate that up and down maneuver with your hand."
- [HEARD 7:26] "it's super fast here, the alternate up and downs right here, **up mute close, down, down mute close**, like it's so fast."
- [SAW 6:25, zoomed piano roll] The 16th-note chug bursts on the low root each have a matching keyswitch note in the C0-B0 zone. The keyswitches zig-zag between two neighbouring keys, which by pixel position are about **G0 (Down Mute Closed) and G#0 (Up Mute Closed)**, one keyswitch per 16th, **with each burst starting on the lower key (down)**. [SAW 5:25 and 7:15] The Active display reads "Up Mute Closed" and "Down Mute Closed" during playback, which confirms those two. The exact key of each zig-zag note is a pixel estimate: treat it as likely, not certain.
- [SAW 6:33] In a slower syncopated section, the long chug notes have no keyswitch. Only the short note right after each long one gets a keyswitch (about G0, Down Mute Closed). So he places a keyswitch on individual notes rather than switching once per section. Whether Keyswitch Latching was on: UNVERIFIED.
- [SAW 6:25] One long keyswitch note sits lower in the zone (about E0, Down Mute Semi Closed?) under a held note: UNVERIFIED.

### Pinch harmonics (6:30-6:47)
- [HEARD 6:33] "notice these red notes here, that is to increase the velocity... so every time you have one of these [pinch] harmonic... that's very much what I do on a daily basis whenever I'm playing the real guitar."
- [SAW 6:33, 6:40, 6:46] The red notes are **velocity 127** (Logic's velocity readout showed 127), so they trigger pinch harmonics through the velocity map, not through a keyswitch. They sit higher than the chug root (about an octave and a half up) and are short. They repeat once per beat at the same point, between the chugs.
- [SAW 13:13-13:30] A run of four red (127) notes held as a sustained line, with a long note in the low keyswitch lane under it. I could not identify that keyswitch: UNVERIFIED.

### Slides (13:17-13:37)
- [HEARD 13:17] "I also did a slide down here as well... that's like something that I would do in real life." Which slide keyswitch he used: UNVERIFIED (not readable).

### String choice and pickup (7:38-8:43)
- [HEARD 7:41] "you also have the ability to choose which string you want to have the note played on, just for a different timbre... if you play the same note on a guitar in different spots... it will sound different."
- [HEARD 8:26] Pickup: he uses **Bridge Def** [SAW 4:05/5:25]. "Bridge Modern just has more fullness to it, they're very similar." Neck pickup: "obviously we're not going to have a neck pickup sound in a riff like this because this is high gain tones and metal tones."

### Keyswitch "Active" display (12:43-13:10)
- [HEARD 12:47] The "Active:" field shows "what key switch is currently triggering the sound... as it goes on it'll show you what key switch we're using next." Useful for checking a generated file by watching that field during playback.

### Genre / aesthetic
- [HEARD 14:18] "you have the ability to get a guitar sound unrealistically clean that a human can't replicate, and that is a flavor in and of itself... like if you take a real guitar track and cut it all up into bits and pieces and make it sound fake, make it sound very 2009 djenty, I think that's kind of cool."

---

## Cross-video rules for the generator (each one cited)
1. **Palm-mute type comes from velocity in metal, metalcore and deathcore**: about 28-30 = open/"bloomy" palm mute on longer chord chugs; about 10-14 = dead mute on fast short 16ths (V1 2:30, 3:43, 4:53; factory velocity bands from V2). Mix the two inside a riff; some short notes stay bloomy (V1 6:14).
2. **Velocity 127 = pinch harmonic** (V2 6:33, factory map). Never use 127 for an ordinary accent; the useful normal-picking ceiling is 126.
3. **Fast breakdown chugs: explicit Down/Up keyswitch per 16th, starting each burst with a down stroke** (V2 5:57, 7:26, seen 6:25). Mute-closed variants: G0 Down Mute Closed / G#0 Up Mute Closed.
4. **Release mute**: a tiny dead-mute note (velocity about 14) at the end of a ringing note before silence (V1 5:00).
5. **Bends**: pre-bent and released DOWN on long notes, starting at note-on; root notes unbent; slow bend UP on tremolo-picked notes. Bend Intensity default 2; 3-4 for nastier deathcore; 9 is unrealistic (V1 1:50-2:20, 4:10-4:47).
6. **Low tremolo/lead lines: force the lowest string** (string-select keyswitch) for a warm tone; auto choice lands on brighter string 6 (V1 2:52-3:25).
7. **Pull-offs inside picked runs** to remove some pick attacks, for realism (V1 3:29).
8. **Double-track two instances** with different sample sets (Guitar 1 / Guitar 2 presets), panned hard L/R: one tight (Humanize about 13%), one loose (about 55%). The occasional deliberate "clanky" chord goes on ONE side only (V2 3:31-4:35, 9:37-11:29).
9. **Advanced: attack impact 0.0 dB** in the Solemn Tones deathcore analysis (no reason given). Pickup Bridge Modern ("less mid honk") or Bridge Def; never the neck pickup for high-gain metal.

## Where this differs from generic guitar-teacher advice
- **"Short notes are dead mutes" is not a hard rule**: Nik keeps some short ones bloomy for variety (V1 6:14).
- **Realism loses to aesthetics in deathcore**: the "less realistic" left-hand-mute dyads sound more "disgusting" and that is the goal (V1 6:44). Xander also says the inhumanly clean, chopped "2009 djent" sound is a valid flavor of its own (V2 14:18).
- **Bends are pre-bent and released down** ("bend up from downs", V1 2:13), not the textbook pick-then-bend-up.
- **Deliberate imperfection on one side only** (V2 9:57). A textbook tight double-track would have both sides identical.
- **The guitar is a percussive element; the arrangement carries the song** (V1 5:20). Monotone root-note spam is correct for the genre.
- Neither video mentions "down pick at the start of a chorus/section" directly. The closest evidence is that each fast 16th burst starts on a down stroke (V2, seen 6:25). Anything beyond that: UNVERIFIED from these two videos.


---

# Odin III research - batch B (Solemn Tones, 3 videos + channel list)

Labels: **HEARD** = the speaker said it (quote/paraphrase with time). **SAW** = on screen in a frame I opened.
**MIDI** = read from the real MIDI file Solemn Tones published with the video. **UNVERIFIED** = my reading, not confirmed.

Sources:
| Video | Channel | channel_url | Length |
|---|---|---|---|
| "Odin 3 : Walkthrough" https://youtu.be/klAbrHifvnY | Solemn Tones | https://www.youtube.com/channel/UCwmWmniB4EZGOAwoF-Y4jjQ | 20:46 |
| "Architects - Odin 3.2 Playthrough" https://youtu.be/gIMUmzriLGA | Solemn Tones | same | 1:19 |
| "Guitar Programming: Paramore ( + Midi files)" https://youtu.be/THly6-rXk-U | Solemn Tones | same | 13:21 |

Descriptions: the walkthrough links an "Odin 3 Demo Midi File" (https://shorturl.at/LNEru, not downloaded).
The Architects description credits "Guitar midi humanization: Maxime @ Solemn Tones". The Paramore description links a Dropbox
folder with `PARAMORE GUITAR MIDI.mid` + backing track. **I downloaded and parsed that MIDI** (stdlib parser:
`C:\Users\only1\AppData\Local\Temp\claude\720250db6c6b595f\23d7b8f2-0dcb-4040-959d-8927adfa2f09\scratchpad\odin-videos\midi_dump.py`;
file: `...\odin-videos\dl-paramore\x\PARAMORE GUITAR MIDI.mid`). That gives exact note numbers, velocities and positions.

Transcripts: walkthrough = YouTube English captions (en-orig). Paramore had no captions: transcribed with
`becky-transcribe --forensic=false --single-pass --cleanup=false` (GPU had 7.7 GB free, RAM 14.9 GB free).
Architects has no speech at all: only on-screen text captions + the plug-in UI.

---

## 1. "Odin 3 : Walkthrough" (Odin 3.0, the official tour)

### Official keyswitch list (SAW, 0:59, Keyswitch List panel opened)
Note names are the plug-in's own; MIDI numbers use the Logic/Cubase convention where C-2 = 0, C-1 = 12, C0 = 24
(SAW: Logic's track shows "Key Limit C-2 G8"; matches the repo's `instruments/odin-iii.json`).

| Note | MIDI | Articulation | | Note | MIDI | Articulation |
|---|---|---|---|---|---|---|
| C0 | 24 | Alt Picked | | C#-1 | 13 | Hammer On |
| C#0 | 25 | Down Picked | | D-1 | 14 | Pull-Off |
| D0 | 26 | Up Picked | | E-1 | 16 | Bend Up Fast |
| D#0 | 27 | Alt Mute Semi Closed | | F-1 | 17 | Bend Up Slow |
| E0 | 28 | Down Mute Semi Closed | | G-1 | 19 | Scrapes (3.2: "Scrapes & Noises") |
| F0 | 29 | Up Mute Semi Closed | | G#-1 | 20 | Tremolo |
| F#0 | 30 | Alt Mute Closed | | A-1 | 21 | Natural Harmonic |
| G0 | 31 | Down Mute Closed | | A#-1 | 22 | Pinch Harmonic |
| G#0 | 32 | Up Mute Closed | | C-2 | 0 | Slide Slow From Up |
| A0 | 33 | Alt Mute Dead | | C#-2 | 1 | Slide Slow From Down |
| A#0 | 34 | Down Mute Dead | | D-2 | 2 | Slide Slow To Down |
| B0 | 35 | Up Mute Dead | | D#-2 | 3 | Slide Slow To Up |
| | | | | E-2 | 4 | Slide Fast From Up |
| | | | | F-2 | 5 | Slide Fast From Down |
| | | | | F#-2 | 6 | Slide Fast To Down |
| | | | | G-2 | 7 | Slide Fast To Up |
| | | | | A-2 | 9 | Finger Pluck Open |
| | | | | A#-2 | 10 | Finger Pluck Dead |
| | | | | B-2 | 11 | Thumb Slap |

**Velocity triggers (SAW, same panel) - while on the default Alt Picking:**
- 127 = Pinch Harmonics
- 126-35 = Alt Picking (open)
- 34-25 = Palm Mutes
- 24-15 = Palm Mutes Closed
- 14-0 = Palm Mutes Dead

FLAG for the repo: in this 3.0 panel, C-1 (12), D#-1 (15) and B-1 (23) are empty. Jordan's 3.2 map has them as
Down Open Palm Mute / Up Open Palm Mute / Nasty Pinch Harmonic. So those three were added after 3.0 (UNVERIFIED which
patch, see "Introducing: Odin Patch 3.1/3.2" below). Also the repo's `velocity_bands` ("dead 15-30", "palm_mute 90-100",
"pinch 110-120") do NOT match these official velocity zones. Jordan's numbers may be his own fit for his keyswitch-driven
style, but if the renderer ever relies on velocity alone, the official zones are 25-34 PM, 15-24 closed, 0-14 dead, 127 pinch.

Other UI facts (SAW): bottom keyboard labels the keyswitch zones "Slides / Slap / Special / Pick & Mutes" on the left and
"String Mod" (8 keys numbered 1-8) on the far right; "Active:" at top-right shows the current articulation (default
"Alternate Picked"). HEARD 2:15: the keyboard shows the name of the keyswitch you just played "so you don't have to open the
key switch list".

### Settings, what they do, and his advice (HEARD unless noted)
- **Humanize knob** (1:31): "will randomize the velocities of the notes being played to get a more human feel." SAW: 13% in
  the default "Panned Rhythm" preset.
- **Pickup voicing** (1:54): Bridge Def is default, "a very hot typically metal type of pickup", Bridge Modern "a bit more
  tamed".
- **Soften Attack** (3:01): "not a feature I would recommend using too often ... Odin 3's pick attack is simply great and
  usually when you go down in velocity the attack will already be changed". So: lower velocity = softer pick, no knob needed.
- **Force Vibrato** (3:25-5:08): replaces hand-drawn pitch-bend vibrato. "I don't want this to be on when I play like Rhythm
  guitars and slow chords, we don't want to vibrato our chords during a chorus" - only on leads with slower passages.
  Attack = how long into the note before vibrato starts: "when you hit a note usually you're not going to start bending it
  from the get-go. You want your listener to know oh this note is being played and then start bending it." Amplitude =
  depth, Speed = rate. For an album/film lead "it might still be useful to manually draw your vibratos so that they are
  different whenever they come up". SAW (8:45): defaults attack 0 ms, ampl 0 cents, speed 0 Hz, switch off.
- **String Select** (5:15-7:45): every fret position "will have its own unique tone". Three levels: Manual (one string the
  engine aims for, e.g. "the eighth string because you really want those round warm tones"), Auto (smart patterns / a
  fret "position"; out-of-position notes go as close as possible), and per-note "String Mod" keyswitches. Recommended
  hybrid (7:16): "for your Rhythm guitars your riffs are all on the E string ... but then you have a pinch harmonic here or
  there that you're going to be playing specifically on another string - that's when you use the string select modifier";
  "for your solos you're going to use auto mode in a specific position".
- **Human Noises** (7:53-8:36): replaces his old habit "I would often place little mutes behind certain notes to really
  emulate the guitar player muting the string with his or her right hand". "Always on by default in the Rhythm presets
  just at a very very little percentage." SAW (8:45): knobs string 0%, mute 32%, left hand 0%.
- **Auto Chord Delay** (8:43-10:12): "whenever a guitarist strikes a chord they will strike the strings one by one ... all
  the notes start at the exact same time [is] something that isn't actually possible". Low end of the dial = milliseconds
  (fast metal strums), high end = locks to project BPM, for "luscious slow chords" in clean low-velocity parts. SAW: 0 ms
  default.
- **Auto Slide** (10:15): overlapping notes slide automatically; but "I personally would always start with the pre-recorded
  ones because they sound incredible". Slide Speed (13:27) only affects Auto Slide; keyswitch slides have fixed recorded
  speeds ("slow slides ... fast slides").
- **Auto Sustain** (10:45-12:14): fills gaps; made because Guitar Pro MIDI "has very odd gaps between notes" giving
  "performances that humans could never play". OFF by default: "whenever I'm writing regularly I do want to have full
  control". => rule: a real guitarist's notes run up to the next note; gaps must be deliberate (mutes/rests).
- **Realign Phase** (12:14): stops the two (fully separate) sample sets phasing. SAW checked in Advanced tab.
- **Add Palm Mute Static** (12:41): "not something I would personally turn on" (raw/ugly). **Enforce Pick Attack**: forces
  attack (SAW checked). **Detune** for matching other recordings / 432 Hz. **Tuning** section is visual only ("no influence
  whatsoever on your sound").
- No Keyswitch Latching setting was shown in 3.0 (UNVERIFIED whether it exists in 3.0).

### The worked example: plain notes -> finished riff (14:21-18:49, SAW in Logic, 160 BPM, 4/4, 1/16 grid)
1. **Starting point** (14:49): Guitar-Pro-style notes, all velocity 120 = "pure alternate picking".
2. **Palm mutes first** (15:00): "the easiest one is going to be palm mutes ... let's say all the quick ones right here and
   we are going to put them at 30 velocity and that would turn them into mutes." SAW (15:20): in a low chug line of 8th
   notes, the short 16th-note doublets (6 notes, two per phrase) are set to velocity 30; the 8th notes stay at 120.
   => The fast pickup pairs get muted, the main 8th pulse rings open.
3. **String choice** (15:23): "these really fast runs are very hard to play for a real human"; set Manual -> String 7
   ("sounds way better"), String 8 also fine; or Auto + "the first five or so frets" position.
4. **Per-note string keyswitch** (16:32, SAW 17:55): String Mod note held for the whole region = string 8, with a short
   switch to string 7 under one note, then back. These sit at the very top of the keyboard (SAW ~C6-C7 in Logic).
5. **Hammer-ons / pull-offs on the fast notes** (16:59-18:12): "all of these notes are very very very fast and while that's
   technically possible it's probably going to be a bit more realistic that some of these are going to be hammer on and
   pull-offs"; "whenever we want a certain note to be played as a hammer-on we're going to go right under it and place
   the note C#-1"; "How do we go from one fast hammer on to another fast note? It's probably going to be a pull-off".
   Pattern he describes: hammer-on, regular picked note, pull-off, hammer-on, pull-off, then regular notes. SAW (18:15):
   keyswitch notes start exactly under their played notes (same start), short lengths; a second cluster near C0-D0
   (pick keyswitches) sits under other run notes. Result (18:33): "sounds a whole lot more realistic because these super
   fast notes now no longer have pick attack".
   => WHY: hammer/pull = removing pick attack from notes a human couldn't pick that fast.

---

## 2. "Architects - Odin 3.2 Playthrough" (no speech; 1:19)
SAW: the screen shows on-screen text captions, a small velocity-coloured piano roll (Logic colours) and Odin's UI with
"Active:" label. I sampled the "Active:" label every 0.5 s (so very short keyswitches can be missed).

| Time | On-screen caption (SAW, verbatim) | What the plug-in shows (SAW) |
|---|---|---|
| 0:00-0:10 | "NOTE THE NOISES KEYSWITCH BEING USED WHENEVER THE RIFF IS MUTED" | Active flips to "Scrapes & Noises" at ~0:08 |
| 0:10-0:22 | "CLOSED PALM MUTES FOR THAT MODERN 'SHOTGUN' EFFECT" | fast low chug + short higher notes; "Tremolo Picked" flashes at ~0:16.5 (UNVERIFIED why) |
| 0:22-0:34 | "BLOOMY OPEN PALM MUTES FOR THE SLOWER CHUGGING" | long held 3-note power-chord shapes (cyan in the roll = lower velocity than the yellow shotgun notes, exact values UNVERIFIED) |
| ~0:29-0:36 | "PICK SCRAPE" | "Scrapes & Noises" at ~0:29-0:29.5 |
| 0:36-0:42 | "BACK TO THE SHOTGUN STYLE RIFF" | |
| 0:42-0:46 | "NOTE THE DOWN/UP KEYSWITCHES TO ENSURE THE ALTERNATIVE PICKING HITS EXACTLY WHEN WE WANT IT TO" | Active: Down Picked 0:39.5-0:40, Up Picked 0:41, Down Picked 0:41.5 |
| 0:46-1:04 | "SIMPLE PROGRESSION STUFF" | long chords; "Scrapes & Noises" at ~0:54 |
| 1:04-1:19 | "BACK TO THE INTRO RIFF" | "Tremolo Picked" at ~1:16.5 |

Takeaways for metalcore/modern metal (Architects):
- Closed palm mutes = the tight "shotgun" fast chug; open palm mutes ("bloomy") = slower chugs. Choice depends on chug SPEED.
- Down/Up keyswitches are used to FORCE pick direction at chosen spots instead of leaving Odin's automatic alternation
  ("to ensure the alternative picking hits exactly when we want it to"). Which exact notes got down vs up: UNVERIFIED
  (the roll has no note names).
- Noises/scrapes go into the muted/silent spots of the riff.

---

## 3. "Guitar Programming: Paramore (+ Midi files)" (pop punk, 173 BPM, 4/4, Odin 3.2)

### Track setup (HEARD 0:28-1:12 + SAW + MIDI)
- Several Odin instances, each locked to an Auto **position**, named in the track: "ODIN RHYTHM MAIN 8STR DDROP C# POS 2",
  "RHYTHM VERSE ... POS 1", "LEAD 1 6STR DROP C# POS 2", "LEAD 2/3 ... POS 3", "CLEAN 1/2 ... POS 2". HEARD 0:31: "some of
  these riffs sound better in different positions". Rhythms on the 8-string tuning, leads/cleans on 6-string tuning.
- Lead position (HEARD 3:47-4:11): position four to keep the lead "nice and warm ... not too shrill. If we were to put this
  on something like position one ... it's going to sound very very bright, and it would not blend nearly as well with
  those lower, thicker chords."
- SAW (2:52, Logic track inspector): **Track Delay -20.0 ms** on the Odin tracks (latency/timing compensation; the channel
  has a video "Using DELAY COMPENSATION for midi guitars & violins"). Not discussed in this video.

### Velocities (HEARD + MIDI)
- Intro/chorus rhythm: "just all max velocity. Well, 125 because 127 would be pinch harmonics" (1:12). MIDI: 1,393 of
  1,403 rhythm notes are exactly velocity 125; no other variation. Chorus (7:23): "Just chords, max velocity, and no
  humanization, nothing going on." Lead 3 (chorus lead, "same chords but a fair bit higher up", 7:52): all velocity 120-123.
- Verse chug (4:11): "It's all just at 34 velocity, which means it's those bloomy, chunky palm mutes" (MIDI: 392 notes at 34).
- Lead 2 (5:25): "This is at 15, so that's gonna be a little bit more of a closed mute. Whereas this first one is at 30,
  it's gonna be a bloomy mute" - mixed with normal picking at 70-80. "Are they messing up? But then in context ... this is
  a pop punk song. It's not trying to be super in the face everything perfection metalcore style." MIDI: 15-24 x30,
  25-34 x60, 70-84 x24.
- Build-up into chorus (6:34): "palm muting at first, then you have them at 65, and then you really move up until you get
  your max power chords". MIDI (verse track): 34 ... then 65, 70, 85, 90, 100, 110, 125 (4 notes each) = a velocity
  crescendo that walks out of the palm-mute zone into open chords.
- Cleans (8:23): "everything played at very very low velocities ... like 55. It's really under 60 where I feel Odin starts to
  play very very gentle." MIDI: clean notes 55-59.
- Timing: MIDI is 100% on the 16th grid (0 of ~1,450 onsets off-grid), no pitch bend, no CC. Humanization comes from the
  plug-in (Humanize knob, human noises, -20 ms track delay), not from moved notes.

### Keyswitches actually used (MIDI, all start exactly with their note, lead = 0 ticks)
| Keyswitch | Where (MIDI) | Reason (HEARD) |
|---|---|---|
| **G-1 (19) Scrapes & Noises** + a carrier note at velocity 85, an 8th long | Always in a RHYTHM gap (rest) between chord hits, on **beat 2 or beat 4** (the backbeat), never on beat 1; every 2-4 bars, not every gap; two in a row once (bar 5 beat 4 and 4.5); carrier pitch alternates 62/63 | 2:01 "extra string noise we added in with the scrapes and noises key switch"; 2:21 "by sprinkling these whenever you have some silence, you're really making everything feel much more realistic"; 2:37 two in a row "because obviously you can't just be doing the exact same thing over and over ... you also don't want endless repetition of the same noises"; 4:39 "we don't put noises on every single silence because then you're once again training your listener to look for those exact same noises" |
| no noises on leads | Lead tracks have zero scrape keyswitches | 2:50 "we don't really want a jumbled mess"; "It's actually very normal for your editor or your engineer to cut out a lot of this noise ... if you're then adding maybe too much of it, it's not gonna sound very professional" |
| **F-2 (5) Slide Fast From Down** ("slide down") | Last note of a lead phrase, **beat 4 of the last bar before the next section** (bars 9, 25, 63 of the leads; bar 113 beat 3.5 on a 2-note chord), followed by silence | 3:21 "we also have a slide down key switch"; 6:22 "a slide down, which then leads into the chorus". Note length matters: "if you made this even longer, you would have the slide also being fully played" |
| **String Mod keyswitch** (MIDI 102 held 8 beats; 96 for 2 notes in Lead 3) | Over the verse-to-chorus build bars (26, 64) | 6:52-7:23 without it "in this transition period, it really gets a whole lot brighter and more shrill"; "use string selection when it makes sense for your song" |
| **D0 (26) Up Picked** | Clean 2: held for the whole 8-bar clean section starting bar 82/90 beat 1, under 4-note chords | Not explained (UNVERIFIED reason) |
| Down Picked / Alt keyswitches | **Never used** - everything else is Odin's default alternate picking | - |

### Settings (HEARD + SAW)
- Human Noises "mutes" at 87 (1:35): "When this double chord is muted, you still hear some kind of ring out ... there is
  going to be some release noise of your right hand muting the string". Recommended 4:49: "anywhere between 80 and a hundred
  should be fine. So that's already enough noise for it to be realistic." SAW tooltip: "Adds muting noises to your
  performances at the end of notes as the player closes their hand on the bridge."
- Clean arpeggios (8:36): "your notes need to ring out into each other. Like if we were to shorten these ... It sounds bad".
  MIDI: clean chord notes held 4 beats, overlapping.

### Genre advice (HEARD 9:32)
"Pop punk tends to be more of a focus on just you know the vocalist doing something emotional, having a really big hook or
melody without really trying to go like look at me, I'm the riff master. So that also means overall your work on your
guitar MIDI will be far far less." => pop punk = flat max-velocity open chords (125), PM verses at 34, very few keyswitches.

---

## 4. Concrete rules this batch supports (with source)
1. Default articulation is Alternate Picking with velocity choosing the mute type: 125 open (never 127 by accident, that
   is a pinch), 25-34 bloomy palm mute, 15-24 closed mute, 0-14 dead (walkthrough 0:59 SAW; Paramore 1:12, 4:11, 5:34 HEARD).
2. Short fast pickup notes inside an 8th chug become palm mutes; the main pulse stays open (walkthrough 15:00).
3. Fast single-note runs: hammer-on/pull-off keyswitches on the notes a human couldn't pick; pull-off between two hammer-ons
   (walkthrough 17:01-18:33).
4. Slide keyswitch on the last note of a lead phrase, beat 4, into the next section (Paramore 3:21, 6:22, MIDI).
5. Scrape/noise keyswitch in some rhythm rests, on beat 2 or 4, not every rest, never repeat the same one, never on leads
   (Paramore 2:01-5:05, MIDI; Architects 0:00 caption).
6. Build into a chorus = velocity crescendo 34 -> 65 -> ... -> 125 on the chords (Paramore 6:34, MIDI).
7. Modern metalcore chug: closed mutes for fast "shotgun" chugs, open/bloomy mutes for slow chugs; force Down/Up with
   keyswitches where the pick direction must land on a specific note (Architects 0:10-0:46 captions).
8. Cleans under ~60 velocity, notes overlapping/ringing (Paramore 8:23-9:02). Slow clean chords: Auto Chord Delay
   tempo-synced (walkthrough 9:59).
9. No vibrato on rhythm chords; vibrato on slow lead notes starts after a delay (walkthrough 4:01-4:36).
10. Fill gaps: real players don't leave Guitar-Pro gaps (walkthrough 11:02).

## 5. Where this differs from generic textbook guitar advice (FLAG)
- **"Down-pick at the start of a chorus"**: NOT what Solemn Tones does here. The Paramore chorus is plain alternate picking at
  velocity 125 with zero pick-direction keyswitches; Down Picked/Alt keyswitches never appear in the file. In Architects
  Down/Up keyswitches are used only "to ensure the alternative picking hits exactly when we want it to" - at specific
  spots (~0:39-0:42), not as a "section start" rule. So "down-pick at section start" is UNVERIFIED by this batch.
- Textbook says vary dynamics/humanize every note; Solemn Tones keeps velocities FLAT (125 everywhere in the chorus, 34
  everywhere in the verse) and leaves humanizing to the plug-in (Humanize knob, release noises). Variation comes from
  articulation changes and section-level velocity steps, not per-note jitter.
- Textbook "always palm-mute the chugs the same way": they pick mute type by chug speed/genre (closed = shotgun, open =
  bloom) and in pop punk deliberately leave "messy" mixed mutes in leads.
- Textbook "more realism = more noises": they say too much noise sounds unprofessional because engineers edit it out.
- Lower velocity = softer pick attack automatically; Soften Attack knob rarely needed.

---

## 6. Solemn Tones channel - other videos relevant to guitar programming / keyswitches (listed, NOT ingested)
Channel: https://www.youtube.com/channel/UCwmWmniB4EZGOAwoF-Y4jjQ (136 videos total; full list saved at
`C:\Users\only1\AppData\Local\Temp\claude\720250db6c6b595f\23d7b8f2-0dcb-4040-959d-8927adfa2f09\scratchpad\odin-videos\channel-all.txt`)

Odin 3 programming tutorials with MIDI files (best for genre rules):
- Programming Architects with MIDI Guitars (+ Midi Files) - https://www.youtube.com/watch?v=bVvOPYEOe2A
- Guitar Programming: Architects (+ Midi Files) - https://www.youtube.com/watch?v=GZwJTqQolOQ
- Guitar Programming: Motionless In White (+Midi Files) - https://www.youtube.com/watch?v=qcjFhbPp5wc
- Guitar Programming: Spiritbox (+ Midi Files) - https://www.youtube.com/watch?v=VYiII07j-QM
- Midi Guitar Programming: The Black Dahlia Murder - https://www.youtube.com/watch?v=CWObAqhmNYE
- Midi Guitar Programming: Dance Gavin Dance ( + Midi Files) - https://www.youtube.com/watch?v=qdrxqsEfgM0
- Midi Guitar Programming: Sleep Token (+ Midi files) - https://www.youtube.com/watch?v=2Gf2uE0qY3M
- Midi Guitar Programming: Three Days Grace (+ Midi files) - https://www.youtube.com/watch?v=BbNme89Xfbc
- Midi Guitar Programming: Slipknot (Files + DAW Deep Dive) - https://www.youtube.com/watch?v=1GVkTaD96rI
- Guitar Programming: Doom BFG Division (Odin 3.1 Midi) - https://www.youtube.com/watch?v=F2JlqvLvq30
- Midi Programming: Odin 3.2 & Sanguine Bass - AAA Void - https://www.youtube.com/watch?v=KlLwJoVAF7k
- DAW Deep Dive: Odin 3 - To The Hellfire - https://www.youtube.com/watch?v=a6Syx-zna5o
- Full Midi Metal Production : DAW Breakdown - https://www.youtube.com/watch?v=fJKAKGHpOVI
- How to: Produce THALL - https://www.youtube.com/watch?v=6Q-HFmDQNyA
- How to MIDI program PLINI (DAW Deep Dive) - https://www.youtube.com/watch?v=43nGqaDsx3s
- Viral Metal Hit: Jake Makes Metal / Kill Karl - DAW Deep Dive - https://www.youtube.com/watch?v=U-9TWrWIFL8
- Analyzing Nik Nocturnal's FAKE Song (Odin 3 Midi) - https://www.youtube.com/watch?v=eVAyRiLNFcg
- Programming LORNA SHORE With Virtual Instruments - https://www.youtube.com/watch?v=5DrcY_X7iLo
- Programming GUITARS With Slaughter To Prevail Cover (Midi Tutorial) - https://www.youtube.com/watch?v=dHn2X1RZf_Q

Technique / settings tutorials:
- 7 Tips For Midi Guitar Realism - https://www.youtube.com/watch?v=RwryEQGzrp4
- Gojira Scrapes: Odin 3 Tutorial ( + Midi download) - https://www.youtube.com/watch?v=KwN9zWtGYxs
- SLIDES IN ODIN 3 (Programming Tutorial) - https://www.youtube.com/watch?v=BzpxU_XnooQ
- Using DELAY COMPENSATION for midi guitars & violins - https://www.youtube.com/watch?v=XFeu-ctb9Xg
- Introducing: Odin Patch 3.2 - https://www.youtube.com/watch?v=uF9HKZ0_qy4
- Introducing: Odin Patch 3.1 + Overview - https://www.youtube.com/watch?v=XaJUWd1gYqc
- Differences Between Odin 2 & Odin 3 - https://www.youtube.com/watch?v=Zk4iB-Ga3ig
- Real Guitar VS Midi Guitar (Odin 3 Comparison) - https://www.youtube.com/watch?v=SUXecJ777Qo
- Turn a GUITAR PRO FILE into a FULL SONG with MIDI Instruments - https://www.youtube.com/watch?v=mQgsMQ4NAMM
- How To: PROGRAM MIDI LEADS (Virtual Guitar - Odin x Ari) - https://www.youtube.com/watch?v=_Lah2dQoTeI
- SOLOS & LEADS with Odin x Ari - https://www.youtube.com/watch?v=JepLqoSxAj8
- The Odin x Ari UPDATE Is Here : A Nerdy Explanation - https://www.youtube.com/watch?v=YFWQ-1262ZU
- Older Odin 2 / Odin 1 era (still useful for reasoning): In-Depth: How To Program Guitars (with The Odin)
  https://www.youtube.com/watch?v=yxp9HuTjjAU ; Fixing Guitar Pro Midis for The Odin https://www.youtube.com/watch?v=Jh22G6xvDUM ;
  How To Program Vibratos With The Odin Deluxe https://www.youtube.com/watch?v=qfdlG3_AKQQ ; Quick Tips: Bends with Odin 2
  https://www.youtube.com/watch?v=B-g8WdSSYzc ; HOW TO: PROGRAM SLIDES IN ODIN 2 https://www.youtube.com/watch?v=BWP5dRv0CWA ;
  Programming SOLOS with Misstiq on Odin 2 https://www.youtube.com/watch?v=5sIc5XUqsBg ; The Odin: Metalcore / Djent /
  Deathcore / Pop Punk Examples (XCi66XcnE5I, o-Kcem8HM44, nngX9ai0x6E, zH5CGQtgdA4).
- Playthrough-only (no talking, but on-screen piano roll like the Architects one): Wage War, Spiritbox, Paramore,
  Falling In Reverse, Black Dahlia Murder, BFG Division, Dance Gavin Dance, Sleep Token, Animal I Have Become, Psychosocial
  (all "- Odin 3.x Playthrough"), Motionless In White FULL MIDI Playthrough.


---

# Batch C - Solemn Tones "Guitar Programming: Motionless In White (+Midi Files)"

Source: https://youtu.be/qcjFhbPp5wc (channel: Solemn Tones, 20:51). Speaker = the Solemn Tones developer/producer (name not given in video). Genre: modern metalcore cover (he calls it "big metal core", "modern metalcore and octane core", 16:30-16:40). DAW: Logic Pro. 101 BPM, 4/4, 480 PPQ.

Evidence key: **HEARD** = he says it (English auto-captions, `en-orig`); **SAW** = on screen (frames I looked at); **MIDI** = measured from his downloaded MIDI file; **UNVERIFIED** = my reading/inference, not confirmed.

MIDI bar numbers = Logic bar numbers in the video (they match: e.g. video 10:03 shows bar 35 = MIDI bar 35).

## Files (downloaded from the Google Drive link in the description, public, no login)

Folder: `C:\Users\only1\AppData\Local\Temp\claude\720250db6c6b595f\23d7b8f2-0dcb-4040-959d-8927adfa2f09\scratchpad\odin-videos\song-midi\MOTIONLESS IN WHITE MIDI\`

| File | Size |
|---|---|
| ODIN 3 MIDI.mid | 10,185 B (1 track "Odin III 01", 1,415 notes incl. 25 keyswitch notes, no CC / pitch bend) |
| BEHEMOTH BASS MIDI.mid | 4,875 B (560 notes) |
| RAGNAR DRUMS MIDI.mid | 7,817 B |
| MIW SYNTHS N FX.wav | 58,552,890 B (backing stem) |

Note: only the "odin 3 rhythm" MIDI is supplied; the "odin 3 lofi" track (13:08 "just some lowfi effect") is not a separate file.

## Odin III factory keyswitch list (SAW, 2:06 zoomed crop; Logic naming C3 = MIDI 60)

- Picking/mutes (C0..B0 = MIDI 24-35): Alternate Picked, Down Picked, Up Picked, Alt/Down/Up Mute Semi Closed, Alt/Down/Up Mute Closed, Alt/Down/Up Mute Dead (B0 = 35 = **Up Mute Dead**).
- C-1..B-1 (12-23): Down Open Palm Mute, Hammer-On, Pull-Off, Up Open Palm Mute, Bend Up Fast, Bend Up Slow, *Auto Slide, Scrapes & Noises, Tremolo Picked, **Natural Harmonic (A-1=21), Pinch Harmonic (A#-1=22), Nasty Pinch Harmonic (B-1=23)**.
- C-2..B-2 (0-11): slides, *Auto Power Chord, Finger Pluck Open/Dead, Thumb Slap.
- **String Selection: F6=String 8 (101), F#6=String 7 (102), G6=String 6 (103), G#6=5, A6=4, A#6=3, B6=2, C7=String 1.**
- **Velocity Triggers: 127 = Pinch Harmonics; ~126-35 = Alt Picking; 34-25 = Palm Mutes; 24-15 = Palm Mutes Closed; 14-8 = Palm Mutes Dead.** (Top of the Alt Picking range is blurry: "125/126 - 35", UNVERIFIED exact number.)
- These match the numbers in `X:\AI-2\midi-agent-skill\instruments\odin-iii.json` (up_mute_dead 35, pinch 22, nasty 23, string_8 101).

## Odin settings on screen (SAW)

- Preset "Panned Rhythm..." ; Amp = DI; Pickup = **Bridge Modern** (0:49); Humanize knob ~12% (blurry, UNVERIFIED); output -1.3 dB; default active articulation "Alternate Picked".
- Playing tab (1:01, 11:03): Soften Attack 0%; Force Vibrato OFF; String Select Mode = Auto, position dropdown "Positio..." (he says Position 4); **Human Noises ON** (string ~0%, mute ~50%, left hand ~0% - blurry, UNVERIFIED); Auto Chord Delay knob value unreadable (~"3 ms"?, UNVERIFIED). Tooltip SAW: "Adds muting noises to your performance at the end of notes as the player places their hand on the bridge".
- HEARD 0:48-1:33: "we are on bridge modern which is just a slightly more modern pickup sound. It doesn't have quite as much of that Steven Carpenter fisherman fluence pickup mids". "on playing, we are in position four because there weren't actually that many higher notes ... anytime you do have a higher note, I really wanted it to sound nice and warm ... I really wanted [it] to not be too [pingy]". Position 1 = "the complete opposite", position 2 = "more in the middle". Reason: the original "is a very dull kind of" tone.

## 1. Every articulation used, where, and why

### A. Default = Odin's velocity-triggered Alternate Picking (no pick-direction keyswitches at all)
- MIDI: **zero** Down Picked / Up Picked / Alt Picked keyswitches in the whole song. All full notes are velocity **125** (827 of 1,390 playable notes) - inside Odin's Alt-Picking velocity band, just under 127.
- UNVERIFIED inference: 125 rather than 127 because 127 auto-triggers the pinch harmonic (HEARD 3:56: "the default harmonic is going to be triggered whenever a note is 127 velocity in Odin 3").
- **Relevance to "down pick at the start of a chorus":** this pro did NOT program a down-pick keyswitch at chorus/section starts. He left pick direction to Odin's automatic alternate picking. Whether Odin's alt-picking resets to a downstroke after a rest/section start is not stated in the video - UNVERIFIED.

### B. Up Mute Dead (B0) = left-hand-muted rake / "cha cha" (4 uses)
- Bar 5 beat 4 (pickup before the song starts): 3x C2 16ths at v127 under B0. HEARD 1:53-2:28: "before the song even starts, we have that cha cha ... that's up mute dead ... B 0 ... on those three notes ... if we were to [loop] these, it would sound well horrendous. But that's kind of the sound you're going for. It's really that left hand muting the strings completely and then just raking over it ... which is pretty common in metal."
- Bar 21 beat 4: 4x C3 16ths under B0 (HEARD 8:03 "another moment of that dead mute up vibe ... B 0 once again").
- Bars 47 and 74 beat 4 (last beat before a chorus): B0 held over 4x 16th power chords at v20. HEARD 10:01: "You can make it even more dead with a key switch going for the dead palm mutes."
- Placement (MIDI): always the **last beat of a bar, as a pickup into the next section/downbeat**. Keyswitch starts exactly on the first note (0-tick lead) and is held over the group.

### C. Pinch Harmonic keyswitch (A#-1), chosen over Odin's 127 "Nasty" default (9 uses)
- Where (MIDI): bars 7, 11, 37, 85 beat 4.5-4.75: two 16ths G#3 + C4 at v127 = end-of-bar squeal fill on the **last two 16ths of the bar, high strings**, after a low-string chug. Also bars 41, 45 (two G3 16ths at end of bar), bar 23 (single C3 on the last 16th), bar 25 beat 4 (4x C3 16ths).
- HEARD 3:53-5:09: "We have two harmonics and the default harmonic is going to be triggered whenever a note is 127 velocity ... But that one actually sounded a bit too [explicit] ... the 127 velocities, they have a little bit of that vib[rato] on them and it's not necessarily the same harmonic either as the regular ones ... nasty pinch harmonics again are the ones going to be triggered also at 127. But ... I actually have a key switch here triggering the pinch harmonics ... If I were to remove this you would have a different pitch and also a different volume ... that was way way higher. So if you wanted to play those nasty, I would probably do like an octave lower ... in order to match the original, it was better to have the regular pinch harmonics. Even though they're performed with a little less schwang, they matched better what the original was doing."
- 8:27-8:46: "we can't just do the [same thing] every time ... I just thought it would be more interesting to do something different here ... once again doing the regular pinch harmonics instead of the nasty because ... it's going to be a different note."
- Rule: **harmonic choice is driven by matching the pitch of the reference recording**, not by "nasty is more metal". Nasty sounds about an octave higher than the written note.

### D. String-selection keyswitches (F#6 = string 7, G6 = string 6) on a held-chord melody (4 per use, bars 13, 39, 87)
- MIDI bar 13: low G#1 held 2 beats + melody D3 (string 7), D#3 (string 6), then A#1 held + D#3 (string 7), A#2 (string 7). Keyswitches are short (1/16 beat-long) and placed **30-89 ticks BEFORE** the note (the only keyswitches in the file with a lead).
- HEARD 6:52-7:50: "I have some key switches to really alter which notes are being played where ... if I remove them ... They were being played just a bit closer to the headstock. I really liked when they were a bit warmer ... whenever you don't necessarily agree with what the engine is doing with the automatic string selection ... just a lot warmer."
- Rule: force higher melody notes onto **thicker/lower strings** (higher up the neck) for a warmer, darker tone than Odin's auto string choice.

### E. Velocity-triggered palm mutes (no keyswitch) - round vs tight
- Odin bands (SAW): 34-25 Palm Mutes, 24-15 Palm Mutes Closed, 14-8 Dead.
- Bar 9 beat 4: 4x C# power chord 16ths at v33 (round), then bar 10-11 tight bursts at v20 (1/32 + 1/32 + 16th + 16th). HEARD 5:17-6:27: "these palm mutes are going to be the more rounder, chunky ones. And these are going to be tighter ... velocity 33. They're going to be round. Here velocity 20. They're going to be a bit tighter ... It changes how close your hand is to the end of the bridge versus more up onto the strings ... it's like a moment of softness, but then after that, you really need your chugs to get super tight."
- End-of-chorus decay (bars 35, 55, 83): sustained chords 30 -> 30 -> 25 -> 20 (MIDI). HEARD 9:22-9:57: "you notice them getting tighter over time ... first two, they're just at 30, but then the last one is at 20 ... you have a moment of big epicness ... the big chorus, but you want to lay that down into a moment of silence before you go back into the verse ... the ending notes, you really want all the energy to be sucked out of the room."
- Chorus fills (bars 29, 30, 49, 50, 72): two 16ths at v20 on beat 4.5-4.75 after a 1.5-beat open chord; bar 31/51/70: one v20 8th on beat 2.5 in the middle of open chords (MIDI).
- Breakdown (bars 59-66): open G octave hits at 125 with **v20 16th on the "a" of beats 1 and 2** between them (MIDI).
- Rule: **within a chord stream, open notes = 125, palm-muted notes are the off-beat/fill notes; palm-mute tightness rises (velocity falls) as a phrase ends.**

### F. Velocity-1 "between-chug" dead notes (Odin closed/dead mute)
- MIDI: 34x C2 16ths at v1 sitting in the gaps of the chord chug (root string only, never the whole chord), e.g. bars 6-12, 36-38, 84-86, on 16th positions .50 of beats 1-3 (the "e-and" gap). SAW 3:18: purple notes on the C2 lane between red chord notes.
- HEARD 2:32-3:11: "rather than having complete silences between each chug ... I decided to add some more of those very very dead palm mutes in there ... without [them] this would sound like a very edited comped guitar, which is fine ... But I personally really like those little ... in between because it makes a performance a bit more dirty ... These are just notes at one velocity which is automatically going to trigger the closed mutes." (Odin's on-screen list says 14-8 = Dead, 24-15 = Closed; v1 is below the list - which layer v1 actually plays is UNVERIFIED beyond his word "closed mutes".)
- Rule: **guitar = fill gaps with v1 mute notes ("less comped"); bass = do NOT** (see section 4).

### G. Velocity-1 1/32 "fake hard mute" at phrase ends
- MIDI: v1 notes 33-60 ticks long (1/16 - 1/8 beat) on C2 right where silence should start, e.g. bars 14, 18, 21, 22, 24 beat 2.75, bar 44 beat 4 (SAW 11:30: one short purple note, velocity 1, at 44.4), bar 83 beat 3.5.
- HEARD 10:57-11:55: "Odin automatically already triggers mute noises ... human noises mutes, they're on. Meaning whenever a note is stopped, the left hand is muting the string and you get a little bit of a sound, but it's very subtle ... it doesn't sound like the guitarist is slapping the neck ... sometimes you do want that sound. So, in order to fake that sound ... you can play a quick dead palm mute right at the moment where you want that mute to hit ... a one velocity palm mute here happening on exactly the beat where I want to create silence."
- 15:29-15:47: "I wanted to create silence once again ... after this [hit] you have once again one of those mute thingies ... me as a guitarist, I would be playing ... a pretty hefty mute right here."
- Rule: **a hard stop = a 1/32 v1 note exactly on the beat the silence starts.**

## 2/3. Velocity, note lengths, timing, voicing, bends

- Velocity map actually used (MIDI): 125 full notes (827); 117 single high C4 accents in the intro chug; 127 only for pinch-harmonic notes (29); 80 chorus chord top notes (74); 53/55/57/58 chord top notes in verse/breakdown/outro (225); 33/30/27/25/20 palm mutes; 1 mute notes.
- **Chord top-note velocity** (power chord root-5th-octave, octave on top): HEARD 3:13-3:42 "the highest notes in the chords I made lower than 60 velocity. So they're at 55 velocity. And that will make them shine through a little bit less. You still want the note to kind of exist, but you don't want it overpowering your root note ... bigger chords ... if you were to put these at like 120 ... they just shine through a little bit more ... I went for a darker vibe ... that's all personal preference." 8:50-9:00 chorus: "the top notes are being played a bit softer velocity-wise, but they're not quite below 60. So they will shine through a bit better" (MIDI: 80). 13:14-14:02 breakdown/outro: notes below 60 "really start drowning into the rest of the chord ... Makes the [chord] a fair bit darker." 14:25-15:28: put notes "below 60 velocity" to make the guitar "less bright and that way make room for your orchestra" (reason: "guitars are ... this giant block of saturation").
- Per-section top-note value differs on repeats: 55 (bars 6-7), 53 (8-12), 58 (36-38), 57 (84-86) (MIDI). Whether that is deliberate variation is UNVERIFIED (not mentioned).
- **Timing: no MIDI timing humanization.** 1,381 of 1,390 playable onsets sit exactly on the 1/16 grid; the 9 off-grid ones are deliberate 1/32 palm-mute bursts (MIDI). Logic Quantize 1/16 shown (SAW). He relies on Odin's Humanize knob (~12%, UNVERIFIED value) instead.
- **Note lengths (MIDI):** chug notes are full-length 16ths (120 ticks) or 8ths (240) butted against each other - no gaps; rests are real silence or a v1 mute note.
- **Keyswitch timing (MIDI):** articulation keyswitches (B0, A#-1) start on the same tick as the first note and are held across the group (Odin latching not needed). String-select keyswitches lead the note by 30-89 ticks.
- **Pitch bend / vibrato / CC:** none in the MIDI file. Force Vibrato off (SAW). The only vibrato he mentions is the one baked into 127 (nasty) harmonics, which he avoided.
- **Strum / chord delay:** not discussed; Auto Chord Delay knob visible, value unreadable (UNVERIFIED). All chord notes start on the same tick (MIDI).
- Tuning/range (MIDI): lowest note F1 (MIDI 41)... root C2 chugs; 8-string drop tuning not stated - UNVERIFIED.

## Bass (Behemoth Bass) - for contrast (HEARD 16:41-20:36, MIDI)
- MIDI: 495 notes at **90**, 65 at **127**, nothing else. 45 of the 65 accents are on **beat 1**, 15 on **beat 3**.
- HEARD: "I did almost no humanizing ... constantly at 90, which is where you're still playing nice and hard ... those accent [notes], those really powerful ones, I'm mostly putting at the start of measures or like on important impacts." "Our 90 is their 127." Tremolo picking would go "to like 40 or 60" because "when you pick fast" you pick lighter.
- HEARD 18:01-19:36: no in-between mute notes on bass, on purpose: "I want my bass and drums to blend together ... punching you and silence ... because bass isn't taking center stage ... nobody is able to tell ... Those silences are really what's making the whole mix breathe." "if you were to make all these notes longer, it just wouldn't hit."
- Drums: "I didn't really do anything to them ... most people are expecting by now for modern metal core ... to just sound like this, just heavy all the way through" (16:03-16:41).

## 4. Where this differs from generic textbook guitar advice

1. **No down-pick keyswitches, no downstroke-on-the-downbeat programming.** Textbook: downpick chugs/section starts. Here: everything rides Odin's automatic alternate-picking velocity band (125). (MIDI)
2. **No timing humanization; flat velocities.** Textbook "humanize everything" is not done: grid-locked 16ths, one velocity per role (125 / 80 / 55 / 20 / 1). Realism comes from articulation choice and mute notes, not random jitter. (MIDI + HEARD 17:28 for bass)
3. **Velocity is an articulation selector, not a loudness knob.** 33 vs 20 = round vs tight palm mute (hand position), 1 = dead mute, 127 = harmonic. Max "loud" note is 125 so it doesn't flip into a harmonic. (SAW list + HEARD)
4. **Dirtier, not cleaner:** he adds v1 dead notes between chugs so it sounds "less comped" - but deliberately keeps the bass hyper-edited/comped. Different rule per instrument. (HEARD 2:32, 18:01)
5. **Lower the top note of a chord** (55, <60) to darken tone and make room for orchestra/synths, instead of EQ alone. (HEARD 3:13, 14:25)
6. **Harmonic type chosen by pitch match to the original**, not "nastier = more metal". (HEARD 4:43-5:09)
7. **Phrase-end energy drain:** palm mutes get tighter (30 -> 25 -> 20) into the silence before a verse; a 1/32 v1 note fakes a hard left-hand stop. (HEARD 9:22, 11:18)
8. **Force warmer strings** for melody over pedal notes (string-select keyswitches) rather than accept the engine's choice. (HEARD 7:15)

## Caveats
- Captions are YouTube auto-captions (`en-orig`); bracketed words in quotes are my best reading of garbled caption words ("[loop]" = caption "lube", likely "loop"; "[explicit]" = bleeped word; "[pingy]" = caption "peing"). The watch skill's default run picked a German auto-dub track; I re-fetched the English original track.
- Section names (verse/chorus/breakdown) are mine from the speaker's words plus the MIDI layout; bar ranges are MIDI-measured.
