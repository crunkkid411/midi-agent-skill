---
name: midi-agent-skill
description: Research-first MIDI composition. The agent researches a genre once (cached as a JSON profile), then deterministic Python composers blend profiles and emit MIDI via a context-aware Lego Pipeline (chords/riff → drums → bass → melody → lead), rendered for any VST (Superior Drummer 3, Odin III, General MIDI, ...). Use when composing music, blending genres, or iterating on songs section-by-section.
license: MIT
---

# midi-agent-skill — Research-First MIDI Composition

> **Compose like a musician, not a template engine.**
> Real composers don't memorize all theory — they research what they
> need, extract a few principles, and build from there. This skill
> teaches the agent that process and caches the result so subsequent
> compositions in the same style are instant and deterministic.

---

## How It Works

```
User: "make a song blending scenecore, witch house, and shoegaze"
      │
      ▼
Agent reads SKILL.md → workflows/compose.md
      │
      ├─ For each genre, load genres/<slug>.json
      │   (or, if missing, run workflows/music-theory-research.md to create it)
      │
      ▼
Agent presents Composition Brief (plans/<title>.md) — waits for user approval
      │
      ▼
python skills/compose_from_profile.py \
    --genres scenecore,witch-house,shoegaze --weights 0.5,0.3,0.2 \
    --bars 8 --title song
      │
      ├─ load + blend profiles
      ├─ structure (form template, section bindings) → harmony timeline
      ├─ Lego Pipeline in the profile's layer_order, e.g.
      │     chords → drums → bass → melody → lead          (chord-led genres)
      │     rhythm_guitar → drums → bass → lead            (riff-led: kick + bass lock to the riff)
      │   (each layer reads previous layers — context-aware sequential generation)
      ├─ write plans/song.composition.json
      └─ render output/song.mid via skills/generate_midi.py
          (through each track's instrument map: SD3 note map, Odin III articulations, GM, ...)
      │
      ▼
Agent evaluates against the brief's principles; iterates by editing the
profile, writing override tracks, or regenerating individual sections
(verse2.B, chorus1.drop, etc.).
```

---

## Layout

```
midi-agent-skill/
├── SKILL.md                           ← this file
├── workflows/                         ← process docs (the agent's playbook)
│   ├── compose.md                     ← end-to-end composition workflow
│   ├── music-theory-research.md       ← how to research a genre + fill a profile
│   ├── genre-profile-spec.md          ← schema reference
│   ├── lego-pipeline.md               ← context-aware sequential generation
│   ├── quick-research.md              ← inline lookups
│   └── brainstorm.md                  ← multi-approach exploration
├── genres/                            ← cached genre profiles (JSON)
│   ├── _schema.json                   ← JSON Schema for profiles
│   ├── README.md
│   ├── lo-fi-hip-hop.json             ← seeded, expanded
│   ├── metalcore.json                 ← researched (web)
│   ├── crabcore.json                  ← researched (web)
│   ├── post-hardcore.json             ← researched from Underoath's They're Only Chasing Safety
│   └── scenecore.json                 ← seeded from Ace-Step DAW reference, expanded
├── instruments/                       ← VST / instrument maps (genre-independent)
│   ├── README.md
│   ├── superior-drummer-3.json        ← Toontrack default note map
│   ├── odin-iii.json                  ← Solemn Tones Odin III (Swift Mode + keyswitches)
│   ├── ujam-beatmaker.json            ← UJAM Beatmaker kit (Instrument mode)
│   ├── ujam-beatmaker-style.json      ← UJAM Beatmaker song-part phrase keys (Style mode)
│   ├── serum-2.json / vital.json      ← synths: CC map, pitch bend, glide
│   ├── gm-drums.json
│   └── _template.json
├── composers/                         ← deterministic Python builders
│   ├── theory.py                      ← scales, Roman numerals, voicings, tunings, drum lanes
│   ├── profile.py                     ← load / blend / query profiles + section bindings
│   ├── structure.py                   ← form / bars → sections (incl. micro) with bindings
│   ├── harmony.py                     ← shared chord timeline + motif repetition
│   ├── chords.py                      ← chords + pad (power chords, root-position voicing)
│   ├── riff.py                        ← rhythm guitar riffs (pedal tone, gallop, chug, ...)
│   ├── doubling.py                    ← double/quad-tracked takes (identical/variation/octave/harmony)
│   ├── automation.py                  ← filter sweeps, sidechain pump, wobble, risers, dives
│   ├── drums.py                       ← bound patterns, any lanes, fills, crashes, kick_follow
│   ├── bass.py                        ← sustained / walking / kick-locked / doubled / ...
│   ├── melody.py                      ← melody + lead (hears drums, repeats motifs)
│   └── pipeline.py                    ← Lego Pipeline + section regeneration
├── skills/
│   ├── compose_from_profile.py        ← main CLI entry point
│   ├── generate_midi.py               ← composition → .mid
│   ├── normalize_composition.py
│   ├── refine_composition.py
│   └── convert_to_wav.py              ← MIDI → WAV (FluidSynth)
├── midi_types/
│   ├── music.py                       ← Composition, Track, Note, Section
│   ├── genre_profile.py               ← profile dataclasses
│   ├── instrument_map.py              ← VST map loader
│   └── gm_instruments.py
├── tests/test_pipeline.py             ← python -m unittest discover -s tests
├── resources/                         ← default theory references (reference albums); classical-reference/ = archive
├── plans/                             ← per-song briefs + composition.json
├── claude_code/                       ← Ace-Step DAW reference (read-only; will be deleted later)
└── output/                            ← generated .mid / .wav files
```

---

## Quick Start

```bash
pip install -r requirements.txt

# Compose an 8-bar lo-fi loop (default length)
python skills/compose_from_profile.py --genres lo-fi-hip-hop --title chillsong

# Blend three genres
python skills/compose_from_profile.py \
    --genres scenecore,lo-fi-hip-hop --weights 0.6,0.4 \
    --bars 16 --title hybrid

# Regenerate just the drums in verse2.B
python skills/compose_from_profile.py \
    --genres scenecore --resume plans/hybrid.composition.json \
    --regenerate verse2.B --layers drums --seed 17

# Full song from the profile's form, rendered for VSTs, one .mid per track
python skills/compose_from_profile.py --genres metalcore --form \
    --vst drums=superior-drummer-3,rhythm_guitar=odin-iii --split-tracks --title riffs
```

Output goes to `plans/<title>.composition.json` and `output/<title>.mid`
(plus `output/<title>/NN-<track>.mid` with `--split-tracks`).

---

## Why 8 Bars by Default

8 bars is the typical loop length and the right canvas for "make me a song
in style X." It's small enough to render fast, big enough to express
harmony + structure (a 4-bar A and a 4-bar B), and scales naturally to
longer arrangements via section regeneration. Use `--bars 16` or more for
multi-section songs; the structure planner will distribute the profile's
intro / verse / chorus / outro across the bar budget.

---

## Micro-Sections — The Nuance Layer

Genre profiles include `song_sections[].micro` for sub-blocks like A/B
inside a verse, or drop/break inside a chorus. The agent and user can
address these by dotted name:

> "Make the drums in verse 2 B a low-fi drum break."

```bash
python skills/compose_from_profile.py \
    --resume plans/song.composition.json \
    --genres lo-fi-hip-hop \
    --regenerate verse2.B --layers drums --seed 17
```

Only those 4 bars of the drum track get re-rendered. Everything else is
preserved.

---

## The Lego Pipeline (Context-Aware Sequential Generation)

```
structure → harmony → chords → drums → bass (locks to drums) → melody (reads chords + drums) → lead
structure → harmony → rhythm_guitar → drums (kick follows riff) → bass (doubles riff) → lead   ← riff-led profiles
```

Each layer is built knowing the cumulative result of all previous layers.
Bass composers receive the drum track and lock to kick hits. Melody
composers read the chord plan and rest over busy drum moments. This is
the same idea as Ace-Step's audio-model context-awareness — but achieved
with deterministic music-theory rules driven by the genre profile.

Any layer can be replaced via `--override-<role>` so user-provided ideas
slot directly into the pipeline alongside generated parts. This is the
human-AI collaboration model: the AI plays expert producer, the human
brings the idea, both contribute layers to the same composition.

Full details: `workflows/lego-pipeline.md`.

---

## Adding a New Genre

1. Read `workflows/music-theory-research.md`.
2. Run 6-8 web searches (chord progressions, drum patterns and fills, form, riffs/tuning, references).
3. Fill `genres/<slug>.json` per `genres/_schema.json` and `workflows/genre-profile-spec.md`,
   including the granularity fields: named progressions, ≥4 drum patterns, section bindings,
   `form`, `fills`, `motif`, `humanize` (+ `tuning`, `layer_order` for guitar-led genres).
4. Compose an 8-bar loop and a `--form` render. Inspect per section. Run the tests.
5. Commit. The profile is now cached for every future composition in that style.

## Targeting VSTs

Instrument maps in `instruments/` are genre-independent. Composers write
drum lanes, articulations (palm mute, pinch harmonic, glide, …) and
automation (filter, volume, pitch, macros). The renderer maps them per track:
- Superior Drummer 3: kit notes
- Odin III: velocity bands and held keyswitches
- UJAM Beatmaker: kit notes, or song-part phrase keys in Style mode
- Serum 2 / Vital: CCs, pitch bend and legato glides
- General MIDI otherwise Set a role's `vst`
in the profile, or `--vst role=slug` for any genre. See `instruments/README.md`.

---

## Sound Design

Profiles include a `sound_design` field for synth presets, sample paths,
FX chains and the VSTs/kits the profile was tuned for. Note mapping and
articulations live in `instruments/`, tone lives in your DAW.

---

## What This Skill Does NOT Do

- **Per-note AI generation** — too expensive, rate-limited. The pipeline is deterministic.
- **Strudel / TidalCycles output** — that's the Ace-Step DAW. Here we emit composition JSON → MIDI.
- **Mixing / mastering / tone** — instrument maps handle notes and articulations; the sound is the plugin's.
- **Pre-built song templates** — research the genre once, cache the profile, generate forever.

---

## Backwards Compatibility (Direct MIDI Use)

If you just want to hand-write a composition and render MIDI without using
the genre profile system, that still works exactly as before:

```python
from skills.generate_midi import generate_midi_from_dict

generate_midi_from_dict({
    "title": "My Song",
    "bpm": 120,
    "tracks": [
        {"instrument": "acoustic-grand-piano", "notes": [
            {"pitch": "C4", "duration": "4"},
            {"pitch": "E4", "duration": "4"},
        ]}
    ]
})
```

New optional fields: `velocity` (0.0-1.0), `start` (beats), `articulation`
and `drum` (lane) per note; `is_drum`, `role`, `vst` and `pan` per track;
`sections` (with `bindings`), `key` and `tuning` on the composition.

---

## Music Theory References (Lookup Only)

The `resources/` directory holds the **default** theory references
(chord-progressions.md, rhythm-patterns.md, voice-leading.md,
modes-scales.md, music-theory.md, counterpoint.md, orchestration.md). They
come from the user's reference albums (3OH!3, Breathe Carolina, Lil Lotus,
Sueco, I Set My Friends On Fire, Attack Attack!, Lady Gaga *The Fame*,
6arelyhuman, Ayesha Erotica, Underoath). The evidence is in
`resources/reference-albums.md`. Read only the one file that matches the
current question.

Precedence: **a genre profile in `genres/` beats `resources/`, and
`resources/` beats `resources/classical-reference/`.** The classical folder
(species counterpoint, "no parallel fifths", orchestra ranges) is used only
when the user asks for classical, orchestral, choral or jazz writing. For a
new genre, prefer the research workflow in
`workflows/music-theory-research.md`.
