# What *They're Only Chasing Safety* exposed and where else it applies

Underoath's 2004 album is **2000s post-hardcore / screamo**. Reviewers call it
screamo, post-hardcore, melodic hardcore, emo and sometimes metalcore. It
became `genres/post-hardcore.json`, with the album and band as aliases. Modelling
it surfaced six gaps:

| # | What the album needed | Mechanism | Also applied to |
|---|---|---|---|
| 1 | The "Down, Set, Go" breakdown switches from 4/4 to 3/4 | section `meter` binding; sections carry `beats_per_bar` + absolute `beat_offset`; every composer, the harmony timeline, fills, regeneration and the renderer (time-signature events) use real bar lengths. Drum patterns can declare the `meter` their grid is written in | All genres. Every composer used to assume 4 beats per bar |
| 2 | Clean intros, an acoustic section and a church-choir bridge are different instruments | `instruments[].tones` + section `tone` binding → separate `<Name> (<tone>)` tracks with their own GM program / VST / double-tracking | **metalcore**: its clean intro arpeggio was being played on the distorted Odin and now has its own clean track |
| 3 | Half the vocals are a clean, sung hook | melody style `vocal_line`: syllabic stepwise phrases that land on a held chord tone and then breathe | **metalcore** gains a clean-vocal guide part for choruses; **crabcore**'s auto-tuned chorus uses it instead of held notes |
| 4 | Two guitars with different jobs: chords vs a high single-note figure with feedback bends | lead style `pedal_ostinato` (moving chord tone against a fixed high pedal, `bend` at phrase ends); riff style `palm_eighths` | Available to any guitar genre |
| 5 | Full open chords (Am x02210, AM7), not just power chords | voicing `guitar_open`; `open_chords` / `power_strum` now respect the section/role voicing | |
| 6 | `AM7` in the bridge | **bug fix:** `maj7` / `maj9` / `M7` / `Δ` were parsed as dominant 7ths | **lo-fi** `IVmaj9` now really is a major 9 (it played a flat 7) |

Related additions: Odin III gets a `bend` velocity band until its bend
keyswitch is filled in. Tone tracks use only the VST their tone names, so an
acoustic part never lands on the electric-guitar plugin.

## Sources
- https://en.wikipedia.org/wiki/They're_Only_Chasing_Safety
- https://www.allmusic.com/album/theyre-only-chasing-safety-mw0000457752
- https://www.scenepointblank.com/reviews/underoath/theyre-only-chasing-safety/
- https://guitartabs.cc/tabs/u/underoath/reinventing_your_exit_crd.html
- https://guitartabs.cc/tabs/u/underoath/a_boy_brushed_red_living_in_black_and_white_crd.html
- https://www.guitartabs.cc/tabs/u/underoath/down_set_go_crd.html
- https://getsongbpm.com/song/reinventing-your-exit/MZxZY5
- https://acousticbrainz.org/7eaca3f4-3574-478d-84ed-ec27f7d54289
