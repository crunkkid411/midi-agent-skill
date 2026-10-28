# Reference Albums (the evidence behind `resources/`)

Every default in `resources/` comes from the records on this page, which the
user picked as this project's core sound. When a rule in another resource
file cites a song, the numbers for that song are here.

**Precedence:** a genre profile in `genres/` > `resources/` (these defaults) >
`resources/classical-reference/` (textbook rules, only on request).

## The records

| Album | Year | Producer(s) | Labelled as |
|---|---|---|---|
| 3OH!3 — *Want* | 2008 | Matt Squire + 3OH!3; Benny Blanco on "Don't Trust Me", "Richman" | electropop, crunkcore, electro-hop, dance-pop |
| Breathe Carolina — *Hell Is What You Make It: Reloaded* | 2011 / 2012 | Ian Kirkpatrick (all but "Chemicals": Matt Squire) | electronicore, electropop, trance, post-hardcore |
| Lil Lotus — *ERRØR BØY* | 2021 | John Feldmann, Drew Fulk; Travis Barker drums on 9 & 12 | pop punk (with emo-rap roots) |
| Sueco — *It Was Fun While It Lasted* | 2022 | incl. Colin Brittain, Andrew Goldstein, John Feldmann; Travis Barker on "SOS" | pop punk / emo rap |
| I Set My Friends On Fire — *You Can't Spell Slaughter Without Laughter* | 2008 | Travis Richter, Nabil Moo | post-hardcore, electronicore, crunkcore, metalcore |
| Attack Attack! — *Attack Attack!* | 2010 | Joey Sturgis | metalcore, electronicore, post-hardcore |
| Lady Gaga — *The Fame* | 2008 | RedOne, Rob Fusari, Martin Kierszenbaum, Brian & Josh | electropop, synth-pop, dance-pop |
| 6arelyhuman — *Digital Dancer* | 2026-08-21 | 6arelyhuman (self-produced) | electroclash / 2000s club / crunk energy (Apple Music bio) |
| Ayesha Erotica — *Precum* | 2025-08-11 | Ayesha Erotica (self-produced) | electronic, hyperpop, pop-rap, electroclash |
| Underoath — *They're Only Chasing Safety* (already `genres/post-hardcore.json`) | 2004 | — | post-hardcore / screamo |

The genres already in the project (`metalcore`, `crabcore`, `post-hardcore`,
`scenecore`, `lo-fi-hip-hop`) count as references too. Their research is in
`docs/`.

## How the numbers were gathered

1. **Audio (all 119 tracks):** Apple Music's 30-second previews, measured
   with librosa. Tempo came from beat tracking, cross-checked with an
   onset-autocorrelation estimate. Key came from Krumhansl-Kessler profile
   matching on harmonic-only chroma.
2. **Transcriptions (65 songs):** Songsterr tabs, read note by note through
   its public data. They give the written tempo, meter changes, section
   names, tunings, and every drum hit, bass note, guitar note, synth note and
   vocal note.
3. **Human chord charts:** Hooktheory TheoryTab (17 songs) and Ultimate
   Guitar ("Paralyzed").
4. **Production facts:** Wikipedia album pages (which cite AllMusic,
   Alternative Press, Rock Sound and others), Sound On Sound and Audiofanzine
   interviews with Joey Sturgis, and Apple Music's artist bio for
   6arelyhuman.

### How far to trust each number
- **Felt BPM (audio)** matched the written tempo within 3 BPM on 51 of the
  55 songs that have both, counting matches at a 2×, ½×, 1.5× or 4/3
  ratio. The other 4 ("Today", "Loser", "LoveGame", "Beautiful, Dirty,
  Rich") were 4 to 5 BPM off. A ratio difference isn't an error. It's the
  *felt* pulse against the *written* grid (see `rhythm-patterns.md` → "Felt
  vs. written tempo").
- **Key** marked `(HT)` or `(UG)` was checked by a human transcription. Key
  marked `~` is the audio estimate. It matched the human charts on about 2
  of 3 songs, and most misses were the relative major/minor (C minor read
  as E♭ major). Treat `~` keys as "this pitch family", not as fact.
- **Tab data** comes from user transcriptions. They're good for patterns
  and parts, but not guaranteed note-perfect. Songsterr labels most vocal
  tracks "Tenor Sax" by default, so that isn't a real sax.
- **Not covered:** no reliable public source names the synths or DAWs used
  on *Want*, *Hell Is What You Make It*, *Digital Dancer* or *Precum*. Those
  are left as UNKNOWN rather than guessed.

## Production facts that shape the defaults (cited)

- **3OH!3:** "pre-programmed beats" instead of traditional instruments, plus
  "blown-out synths" and "modulated vocals". Called "the main blueprint for
  hyperpop" (Wikipedia, *Crunkcore*). Critics heard "bass heavy tunes …
  distorted guitars" and crunk fused with punk, electronic and emo (Clash,
  via Wikipedia *Want*).
- **Breathe Carolina:** "house-techno sequencers and melodic vocals parry
  with screamo-metal breakdowns" (Alternative Press on "Wooly"). "Blackout"
  is "an irresistible synth-pop song". The record shifted "less on scream
  vocals and more emphasis on clean vocals" (Wikipedia).
- **Attack Attack!:** "Most of the album was recorded in drop-B tuning".
  "A for Andrew" is in drop A and "AC-130" in drop G. "Smokahontas" "blends
  dubstep, hardcore, metalcore, pop and dance-pop". Reviewers name
  "autotuned, synth-laden dance portions" (Wikipedia). Producer Joey
  Sturgis: "Everything's to a click", every note checked "on the grid, on
  time and in tune", drums sample-replaced. Two guitar tracks, one left and
  one right, never four. "I double everything, triple some things and
  quadruple others" for vocals, and "four low screams that combine to
  support one centre scream". He mixes through his mastering limiter from
  the start, with drums "crushed" to punch through (Sound On Sound;
  Audiofanzine).
- **Electronicore in general:** "metalcore instrumentation, breakdowns, and
  heavy use of sequencers", "trance, electronica, and dubstep", "auto-tuned
  singing, and screamed vocals", and "dynamic transitions from soft
  electronica ballads to intense metalcore passages" (Wikipedia,
  *Electronicore*).
- **The Fame:** "stainless steel synths and dark disco rhythms" (AllMusic).
  "Just Dance", "LoveGame" and "Poker Face" share "clipped marching beats,
  sawing electronics". Auto-tuned vocals on "Starstruck" and "Paper
  Gangsta". 1980s synth-pop, 1970s glam, Bowie-style guitar riffs on "The
  Fame". The album credited with bringing four-on-the-floor back to US
  radio (Wikipedia). RedOne: "I am a melody person"; "Just Dance", "Poker
  Face" and "LoveGame" were written "in a week".
- **Lil Lotus / Sueco:** 2000s pop-punk revival made with rap-scene
  producers. BrooklynVegan hears "emo-rap" on "Over and Over Again" and a
  "full-blown pop-punk sugar rush" elsewhere. Sueco pairs "rapped verses
  with guitar-heavy choruses" (Prelude Press).
- **6arelyhuman:** electroclash and 2000s club beats with crunk energy
  (Apple Music bio). Their "krushclub" tag comes from "distorted,
  bit-reduced beats" (Nina, 2024; only seen as a search excerpt).
- **Ayesha Erotica:** "prolific hyperpop producer", "bubblegum tracks",
  "pop-rap bangers indebted to the music of the mid-2000s", beats compared
  to the Neptunes by Slayyyter (Wikipedia).
- **Electroclash:** "fuses 1980s electro, new wave and synth-pop with 1990s
  techno", "the extended pulsing sections of techno, house" plus "the
  trashier energy of rock and new wave" (Wikipedia, *Electroclash*).

## Per-track data

Felt BPM = measured from the audio. Written BPM = the tab's tempo map
(arrows mark tempo changes). Low string = the guitar's lowest open string
in the tab (E2 = standard, D2 = drop D, C2 = drop C, B1 = drop B, A1 = drop
A, G1 = drop G).

### 3OH!3 - Want

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Tapp | 125 |  | ~G minor |  |  |  |
| 2 | Punkb*Tch | 146 |  | ~G minor |  |  |  |
| 3 | Don't Trust Me | 130 | 130 | G minor (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/3oh-3-don-t-trust-me-tab-s60943) |
| 4 | Chokechain | 146 | 145 | ~F# minor | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/3oh-3-chokechain-tab-s30651) |
| 5 | I'm Not Your Boyfriend Baby | 140 |  | ~Ab major |  |  |  |
| 6 | I Can't Do It Alone | 160 |  | ~E major |  |  |  |
| 7 | Starstrukk | 140 | 140 | D minor (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/3oh-3-starstrukk-tab-s965679) |
| 8 | Richman | 162 | 160 | ~E minor | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/3oh-3-richman-tab-s4729192) |
| 9 | Photofinnish | 137 |  | ~G minor |  |  |  |
| 10 | Still Around | 74 |  | ~F major |  |  |  |
| 11 | Holler Til You Pass Out | 151 |  | ~E minor |  |  |  |
| 12 | Colorado Sunrise | 151 | 150 | ~F# major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/3oh-3-colorado-sunrise-tab-s60944) |

### Breathe Carolina - Hell Is What You Make It: Reloaded

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Rebirth: An Introduction | 93 |  | ~Eb major |  |  |  |
| 2 | Wooly | 142 | 140 | ~Eb major | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/breathe-carolina-wooly-tab-s85222) |
| 3 | Blackout | 123 | 123 | D#/Eb minor (HT) | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/breathe-carolina-blackout-tab-s372676) |
| 4 | Edge of Heaven | 136 | 136 | ~A minor | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/breathe-carolina-edge-of-heaven-tab-s372671) |
| 5 | Last Night (Vegas) | 112 |  | ~D major |  |  |  |
| 6 | Sweat It Out | 118 |  | ~Eb major |  |  |  |
| 7 | Gone So Long | 116 |  | ~Bb minor |  |  |  |
| 8 | They Say You Won't Come Back | 125 |  | ~A minor |  |  |  |
| 9 | Get Off Easy | 157 |  | ~D minor |  |  |  |
| 10 | Waiting | 120 |  | ~Ab minor |  |  |  |
| 11 | Take It Back | 110 |  | ~A minor |  |  |  |
| 12 | Chemicals | 128 |  | ~F minor |  |  |  |
| 13 | Lauren's Song | 148 | 74 | ~G major | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/breathe-carolina-lauren-s-song-tab-s372672) |
| 14 | Hit And Run | 124 |  | ~F# minor |  |  |  |
| 15 | Reaching For The Floor | 124 |  | ~Eb minor |  |  |  |

### Lil Lotus - Error Boy

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Think of Me Tonight | 190 | 190 | ~E major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lil-lotus-think-of-me-tonight-tab-s2568399) |
| 2 | Romantic Disaster (feat. Chrissy Costanza) | 125 |  | ~C# major |  |  |  |
| 3 | Over and over Again | 150 | 152 | ~B major | 2/4, 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lil-lotus-over-and-over-again-tab-s4415270) |
| 4 | Rooftop | 95 |  | ~C# minor |  |  |  |
| 5 | Butterfly K | 160 | 158 | ~E major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lil-lotus-butterfly-k-tab-s2513462) |
| 6 | Girl Next Door | 165 | 165 | ~F major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lil-lotus-girl-next-door-tab-s2513450) |
| 7 | Lips That Kill | 93 |  | ~A minor |  |  |  |
| 8 | Doctor Doctor | 117 |  | ~C minor |  |  |  |
| 9 | No Getting over This (feat. Travis Barker) | 110 |  | ~G major |  |  |  |
| 10 | Fake Love | 116 |  | ~C# minor |  |  |  |
| 11 | Why U Do Me Like This | 78 |  | ~C major |  |  |  |
| 12 | Don't F**k This Up (feat. Travis Barker) | 117 |  | ~Eb major |  |  |  |

### Sueco - It Was Fun While It Lasted

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Today | 101 | 105 → 95 → 90 | ~B major | 3/4 | E2 | [tab](https://www.songsterr.com/a/wsa/sueco-today-tab-s1285745) |
| 2 | Paralyzed | 150 | 150 | C minor (UG) | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/sueco-paralyzed-tab-s3430769) |
| 3 | It’s Going Good! | 95 | 155 → 185 → 190 | ~E major | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/sueco-it-s-going-good-tab-s3940247) |
| 4 | Hate You Too | 97 | 96 | ~E major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/sueco-hate-you-too-tab-s7205428) |
| 5 | Loser | 97 | 190 | ~Bb major | 4/4 | Eb2 | [tab](https://www.songsterr.com/a/wsa/sueco-loser-tab-s549477) |
| 6 | Drunk Dial | 132 |  | ~D major |  |  |  |
| 7 | Toxic Therapy | 95 | 95 | ~E minor | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/sueco-toxic-therapy-tab-s7672154) |
| 8 | SOS (feat. Travis Barker) | 100 | 150 → 120 → 150 | ~C# major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/sueco-sos-feat-travis-barker-tab-s2359804) |
| 9 | Motel 666 | 179 | 144 → 180 | ~Bb minor | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/sueco-motel-666-tab-s2944294) |
| 10 | Sober/Hungover (feat. Arizona Zervas) | 176 |  | ~F# minor |  |  |  |
| 11 | PRIMADONA | 96 |  | ~C# minor |  |  |  |
| 12 | It Was Fun While It Lasted | 94 | 94 | ~F# major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/sueco-it-was-fun-while-it-lasted-tab-s2944252) |

### I Set My Friends On Fire - You Can't Spell Slaughter Without Laughter

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Sh!T It Talks...I'm out of Here | 102 |  | ~F major |  |  |  |
| 2 | Brief Interviews with Hideous Men | 65 | 130 → 110 | ~D minor | 2/4, 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-brief-interviews-with-hideous-men-tab-s2976222) |
| 3 | Beauty Is in the Eyes of the Beerholder | 170 | 170 | ~G minor | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-beauty-is-in-the-eyes-of-the-beerholder-tab-s38077) |
| 4 | Things That Rhyme with Orange | 112 | 168 | ~C major | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-things-that-rhyme-with-orange-tab-s38071) |
| 5 | Asl | 84 | 85 → 80 → 85 | ~D minor | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-asl-tab-s38075) |
| 6 | Interlude | 103 |  | ~Eb major |  |  |  |
| 7 | Ravenous, Ravenous Rhinos | 148 | 148 | ~G minor | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-ravenous-ravenous-rhinos-tab-s38078) |
| 8 | Hxc 2 - Step | 160 |  | ~C minor |  |  |  |
| 9 | Wtfwjd | 86 | 173 | ~D minor | 4/4, 3/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-wtfwjd-tab-s38076) |
| 10 | Crank That | 140 | 138 → 145 → 103 | ~C major | 4/4, 2/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-crank-that-tab-s38073) |
| 11 | But the Nuns Are Watching | 107 | 215 → 115 | ~D major | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-but-the-nuns-are-watching-tab-s615081) |
| 12 | Reese's Pieces, I Don't Know Who John Cleese Is? | 95 | 98 → 138 → 140 | ~C minor | 4/4 | C2 | [tab](https://www.songsterr.com/a/wsa/i-set-my-friends-on-fire-reese-s-pieces-i-don-t-know-who-john-cleese-is-tab-s71677) |

### Attack Attack! - Attack Attack!

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Sexual Man Chocolate | 83 | 124 | ~B major | 4/4 | B1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-sexual-man-chocolate-tab-s63829) |
| 2 | Renob, Nevada | 95 | 190 | ~C minor | 4/4 | B1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-renob-nevada-tab-s68656) |
| 3 | I Swear I'll Change | 113 |  | ~Eb minor |  |  |  |
| 4 | Shut Your Mouth | 125 |  | ~A minor |  |  |  |
| 5 | A for Andrew | 160 | 120 | ~A major | 5/4, 4/4 | A1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-a-for-andrew-tab-s381174) |
| 6 | Smokahontas | 135 | 135 → 135 → 120 | ~C major | 4/4 | B1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-smokahontas-tab-s68660) |
| 7 | AC-130 | 126 | 125 → 115 → 108 | ~E minor | 4/4, 2/4, 1/4 | G1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-ac-130-tab-s64291) |
| 8 | Fumbles O'Brian | 165 | 165 | ~G minor | 4/4 | B1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-fumbles-o-brian-tab-s68655) |
| 9 | Turbo Swag | 82 | 165 | ~B minor | 4/4 | B1 | [tab](https://www.songsterr.com/a/wsa/attack-attack-turbo-swag-tab-s68659) |
| 10 | Lonely | 130 |  | ~B minor |  |  |  |

### Lady Gaga - The Fame

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Just Dance (feat. Colby O'Donis) | 119 | 121 | C# minor (HT) | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/lady-gaga-just-dance-feat-colby-o-donis-tab-s742306) |
| 2 | LoveGame | 105 | 110 | B minor (HT) | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-lovegame-tab-s64695) |
| 3 | Paparazzi | 114 | 115 | C minor verse / Ab major chorus (HT) | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/lady-gaga-paparazzi-tab-s35667) |
| 4 | Poker Face | 119 | 119 | G# minor verse / B major chorus (HT) | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/lady-gaga-poker-face-tab-s47451) |
| 5 | Eh, Eh (Nothing Else I Can Say) | 188 | 93 | E major (HT) | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/lady-gaga-eh-eh-nothing-else-i-can-say-tab-s64692) |
| 6 | Beautiful, Dirty, Rich | 120 | 116 | B minor (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-beautiful-dirty-rich-tab-s414038) |
| 7 | The Fame | 127 | 127 | G Dorian (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-the-fame-tab-s64705) |
| 8 | Money Honey | 119 | 119 | G# minor (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-money-honey-tab-s211101) |
| 9 | Starstruck (feat. Space Cowboy & Flo Rida) | 103 |  | G# minor (HT) |  |  |  |
| 10 | Boys Boys Boys | 123 | 120 | ~A major | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-boys-boys-boys-tab-s64689) |
| 11 | Paper Gangsta | 100 | 99 | Bb minor (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-paper-gangsta-tab-s64694) |
| 12 | Brown Eyes | 125 | 60 → 40 | Bb major (HT) | 4/4, 2/4 |  | [tab](https://www.songsterr.com/a/wsa/lady-gaga-brown-eyes-tab-s64690) |
| 13 | I Like It Rough | 120 | 120 | C Dorian (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-i-like-it-rough-tab-s211100) |
| 14 | Summerboy | 105 | 106 | E major (HT) | 4/4 | E2 | [tab](https://www.songsterr.com/a/wsa/lady-gaga-summerboy-tab-s414040) |
| 15 | Disco Heaven | 128 | 130 → 130 → 130 | ~D minor | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/lady-gaga-disco-heaven-tab-s64691) |

### 6arelyhuman - Digital Dancer

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Like That | 99 |  | ~Ab minor |  |  |  |
| 2 | Zombie XD | 170 |  | ~G minor |  |  |  |
| 3 | Abyss (feat. Baby Jane) | 150 |  | ~G major |  |  |  |
| 4 | Dirt | 130 |  | ~F# minor |  |  |  |
| 5 | Feels So Amazing | 143 |  | ~F minor |  |  |  |
| 6 | Freaks (feat. DEV) | 78 |  | ~F# minor |  |  |  |
| 7 | Club Crasher (feat. Millionaires) | 161 |  | ~F# minor |  |  |  |
| 8 | DDR (Dance Dance Revolution) | 92 |  | ~F minor |  |  |  |
| 9 | Parasite (feat. DeathbyRomy) | 130 |  | ~F# minor |  |  |  |
| 10 | Millionaire (feat. asteria & kets4eki) | 156 | 156 | ~A minor | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/6arelyhuman-millionaire-feat-asteria-kets4eki-tab-s4106054) |
| 11 | Red Room | 146 |  | ~B minor |  |  |  |
| 12 | CR4CK HOUSE | 161 | 161 | ~A minor | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/6arelyhuman-cr4ck-house-tab-s4471725) |
| 13 | Get Nasty (feat. Rico Nasty & Whethan) | 135 |  | ~Eb minor |  |  |  |
| 14 | Superficial Luv | 122 |  | ~Ab major |  |  |  |
| 15 | So Perfect (feat. MISS LUXURY) | 100 |  | ~A major |  |  |  |
| 16 | Broken | 162 |  | ~Ab minor |  |  |  |
| 17 | Get Away | 166 |  | ~F minor |  |  |  |

### Ayesha Erotica - Precum

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | WHORE IDOL (feat. MkX) | 123 |  | ~D minor |  |  |  |
| 2 | HOW2FUCK | 123 |  | ~E major |  |  |  |
| 3 | STAR 69 | 108 |  | ~F minor |  |  |  |
| 4 | GANG | 154 |  | ~C# minor |  |  |  |
| 5 | BITCH | 160 |  | ~Ab major |  |  |  |
| 6 | GIDDY UP | 116 |  | ~Ab minor |  |  |  |
| 7 | RUNAWAY | 171 |  | ~Eb major |  |  |  |
| 8 | MENLO PARK | 130 | 130 | ~D major | 4/4 |  | [tab](https://www.songsterr.com/a/wsa/ayesha-erotica-menlo-park-tab-s5877039) |

### Underoath - They're Only Chasing Safety

| # | Track | Felt BPM (audio) | Written BPM (tab) | Key | Meter | Low string | Tab |
|---|---|---|---|---|---|---|---|
| 1 | Reinventing Your Exit |  | 156 |  | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-reinventing-your-exit-tab-s1827124) |
| 2 | A Boy Brushed Red Living In Black And White |  | 85 |  | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-a-boy-brushed-red-living-in-black-and-white-tab-s21908) |
| 3 | Down, Set, Go |  | 170 → 173 |  | 4/4, 3/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-down-set-go-tab-s57382) |
| 4 | It's Dangerous Business Walking Out Your Front Door |  | 139 → 139 |  | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-it-s-dangerous-business-walking-out-your-front-door-tab-s29627) |
| 5 | Some Will Seek Forgiveness, Others Escape |  | 137 |  | 3/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-some-will-seek-forgiveness-others-escape-tab-s59830) |
| 6 | Young And Aspiring |  | 180 |  | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-young-and-aspiring-tab-s20956) |
| 7 | I'm Content With Losing |  | 165 |  | 1/8, 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-i-m-content-with-losing-tab-s57388) |
| 8 | Writing On The Walls |  | 184 |  | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-writing-on-the-walls-tab-s57403) |
| 9 | Moving For The Sake Of Motion |  | 185 |  | 4/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-moving-for-the-sake-of-motion-tab-s5430674) |
| 10 | The Impact Of Reason |  | 173 |  | 4/4, 3/4, 2/4 | D2 | [tab](https://www.songsterr.com/a/wsa/underoath-the-impact-of-reason-tab-s421831) |

## Sources
- Apple Music / iTunes catalog lookups (track lists, 30-second previews): albums 287413480, 541150176, 1564976012, 1607219379, 1485044331, 1763654860, 1440818588, 6824854001, 1832008655
- Songsterr transcriptions: links in the tables above
- Hooktheory TheoryTab: https://www.hooktheory.com/theorytab/artists/l/lady-gaga , https://www.hooktheory.com/theorytab/view/3oh3/dont-trust-me , https://www.hooktheory.com/theorytab/view/3oh3/starstrukk-(feat.-katy-perry) , https://www.hooktheory.com/theorytab/view/breathe-carolina/blackout
- Ultimate Guitar: https://tabs.ultimate-guitar.com/tab/sueco/paralyzed-chords-3836558
- Wikipedia: Want (3OH!3 album), Hell Is What You Make It, Error Boy, Paralyzed (Sueco song), You Can't Spell Slaughter Without Laughter, Attack Attack! (album), The Fame, Ayesha Erotica, Crunkcore, Electronicore, Electroclash
- Joey Sturgis: https://www.soundonsound.com/people/joey-sturgis , https://en.audiofanzine.com/sound-technique/editorial/articles/joey-sturgis-has-it-covered.html
- RedOne: https://www.songwriteruniverse.com/redone-songwriter-interview-2016/
- Reviews: https://www.brooklynvegan.com/stream-lil-lotus-new-pop-punk-solo-album-error-boy-his-metalcore-band-if-i-die-firsts-new-ep/ , https://www.kerrang.com/album-review-lil-lotus-error-boy , https://preludepress.com/?p=48740
- 6arelyhuman bio: https://music.apple.com/artist/1527936639
