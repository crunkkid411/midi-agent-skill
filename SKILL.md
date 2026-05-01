---
name: midi-agent-skill
description: Research-first MIDI composition. The agent researches a genre once (cached as a JSON profile), then deterministic Python composers blend profiles and emit MIDI via a context-aware Lego Pipeline (chords → drums → bass → melody → lead). Use when composing music, blending genres, or iterating on songs section-by-section.
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
      ├─ Lego Pipeline: structure → chords → drums → bass → melody → lead
      │   (each layer reads previous layers — context-aware sequential generation)
      ├─ write plans/song.composition.json
      └─ render output/song.mid via skills/generate_midi.py
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
│   ├── lo-fi-hip-hop.json             ← seeded
│   └── scenecore.json                 ← seeded from Ace-Step DAW reference
├── composers/                         ← deterministic Python builders
│   ├── theory.py                      ← scales, Roman numerals, drum mapping
│   ├── profile.py                     ← load / blend / query profiles
│   ├── structure.py                   ← bars → sections (incl. micro)
│   ├── chords.py
│   ├── drums.py                       ← honors per-section density
│   ├── bass.py                        ← locks to drum kicks for 808 styles
│   ├── melody.py                      ← used for both melody and lead
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
│   └── gm_instruments.py
├── resources/                         ← lookup-only theory references
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
```

Output goes to `plans/<title>.composition.json` and `output/<title>.mid`.

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
structure → chords → drums → bass (locks to drums) → melody (reads chords + drums) → lead
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
2. Run 4-6 web searches (chord progressions, drum patterns, structure, references).
3. Fill `genres/<slug>.json` per `genres/_schema.json` and `workflows/genre-profile-spec.md`.
4. Compose an 8-bar test loop. Listen. Iterate the profile until it sounds right.
5. Commit. The profile is now cached for every future composition in that style.

---

## Sound Design (Forward-Compat)

Profiles include a `sound_design` field for synth presets, sample paths,
and FX chains. Composers ignore it today but research notes about preset
choices belong here so the data is captured. Future work: user-provided
drum sample swaps, synth-preset round-trip with external tooling.

---

## What This Skill Does NOT Do

- **Per-note AI generation** — too expensive, rate-limited. The pipeline is deterministic.
- **Strudel / TidalCycles output** — that's the Ace-Step DAW. Here we emit composition JSON → MIDI.
- **Mixing / mastering / sound design** — `sound_design` field is a placeholder.
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

New optional fields: `velocity` (0.0-1.0) and `start` (beats) per note;
`is_drum` per track for GM channel-9 drum routing; `sections` and `key`
on the composition.

---

## Music Theory References (Lookup Only)

The `resources/` directory has reference files (chord-progressions.md,
voice-leading.md, modes-scales.md, counterpoint.md, etc). These exist
for the agent to **look up specific things** mid-task. Don't read them
all — read the one that matches the current question. For genre-specific
research, prefer the web search workflow in
`workflows/music-theory-research.md` over these generic references.
