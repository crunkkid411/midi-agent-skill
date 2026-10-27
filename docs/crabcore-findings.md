# What crabcore exposed and where else it applies

Adding crabcore (Attack Attack!, I See Stars, Abandon All Ships, Enter
Shikari-style electronicore) surfaced eight things the pipeline couldn't
express. Each one turned out to be missing from other genres too, so each
is implemented generically and applied wherever it fits.

| # | What crabcore needed | Mechanism | Also applied to |
|---|---|---|---|
| 1 | Guitars are double-tracked, and the takes relate differently per section | `instruments[].double_track` `{takes, mode, variation_rate}` + section `double_track` binding: identical / variation / octave / harmony. Chugs always identical; every take humanized separately (`composers/doubling.py`) | **metalcore**: identical intro / pre-chorus / breakdown / outro, varied verses, octave-split choruses, harmonized bridge |
| 2 | Filter sweeps in builds, sidechain-pumped synths, wobble bass, risers, tape-stop dives | section `automation` lanes → CC / pitch bend through each instrument's `cc_map` (`composers/automation.py`) | **lo-fi**: low-passed intro opening up, filter dip in the drop-out, closing outro, light pad pump. **scenecore**: filtered intro, filter + pitch riser build, pumping drop, closing outro. **metalcore**: pad swell into the chorus, fade into the stop |
| 3 | A drum machine next to the kit (dance breaks, reinforced breakdowns) | `electronic_drums` role; patterns tagged `"layer": "electronic"`; section `electronic_pattern` | Any genre can use it (e.g. 808 layer under a live kit) |
| 4 | Tempo-shifting sections (slower dance break, slower second breakdown) | section `tempo` / `tempo_scale` → composition `tempo_map` | **scenecore**: slower break, the hyperpop tempo switch-up |
| 5 | Synths that double the breakdown chugs | chords style `follow_riff` (reads the riff's attacks) | Any riff-first genre |
| 6 | Trance-gated supersaw chords | chords style `trance_gate` (16-step gate in `articulations.gate`) | Any synth genre |
| 7 | Parts that exist only in some sections (synth lead in choruses, clean vocal only in the chorus) | `instruments[].play_in` | **metalcore** pad (pre-chorus + chorus) |
| 8 | Chords/pad style can't change per section; style bindings hit every role that shares the key | `chords_style` binding; any style binding may be a `{role: style}` dict | All genres |

Also found and fixed:
- **808 glides** (scenecore): `articulations.glide_rate` adds octave slides; synth maps overlap glide notes (`legato_overlap`) so Mono/Legato patches glide.
- **Same-pitch overlaps**: stacked identical stabs, or humanized flams of one drum, could crash the MIDI writer or cut notes short. The renderer now merges near-simultaneous hits and trims overlaps.
- **VSTs the user owns are now the defaults where they fit**: Beatmaker drums and Vital pad in lo-fi; Serum 2 / Vital synths, Beatmaker drums and a Serum 808 in scenecore; SD3, Odin III, Beatmaker, Serum 2 and Vital in crabcore.

## New instrument maps
- `ujam-beatmaker` — Instrument mode (C1–D#2 kit keys, from the Beatmaker 2.1 manual)
- `ujam-beatmaker-style` — Style mode: held song-part keys per section (Verse/Chorus/Breakdown/Fill/Special/End/Off)
- `serum-2`, `vital` — CC map (MIDI Learn to CC74/7/11/21-28), pitch-bend range, legato glide
