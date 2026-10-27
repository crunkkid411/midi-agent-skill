# midi-agent-skill

A research-first MIDI composition skill for AI agents. The agent
researches a genre once (cached as a JSON profile), then deterministic
Python composers blend profiles and emit MIDI through a context-aware
**Lego Pipeline** (chords → drums → bass → melody → lead). Each layer
"hears" all previous layers, so the song is coherent from the ground up.

The skill is **agent-agnostic** — anything that can read a SKILL.md and
run Python (Claude Code, Claude API, other agent harnesses) can drive it.

---

## What's Different

- **Research-first composition** — the agent reads `workflows/compose.md`, researches the genre's chord progressions, drum patterns, structure, and instrumentation, and saves a reusable `genres/<slug>.json` profile.
- **Deterministic Python composers** — once a profile exists, generation is rule-based and rate-limit-free. AI is used where it adds value (research, brief writing, evaluation), Python where determinism matters (note placement, MIDI rendering).
- **Genre blending** — combine 2-4 profiles with weights to compose in hybrid styles.
- **Micro-section addressability** — request "add a drum break in verse 2 B" and regenerate exactly those 4 bars without disturbing the rest of the song.
- **Layer overrides** — user-provided ideas (a bassline, a chord progression) slot directly into the pipeline alongside generated parts. Human-AI collaboration on the same composition.
- **Riff-first genres** — a rhythm-guitar composer (pedal tone, gallop, breakdown chugs, power chords on the tuning's open string). Profiles can reorder layers so the kick and bass lock to the riff.
- **Section-level control** — every section and micro-section can bind its own groove (or a sequence per phrase), progression, scale, styles, mutes and stops. Form templates have optional slots. Fills land at phrase ends. Motifs repeat with variation.
- **Any VST, any genre** — instrument maps translate drum lanes and articulations for Superior Drummer 3, Solemn Tones Odin III (Swift Mode velocity bands + held keyswitches), General MIDI or your own map, with per-track MIDI export for DAW templates.

---

## Repository Structure

```
/
├─ SKILL.md                          # Skill definition + workflow overview
├─ workflows/                        # Process docs (the agent's playbook)
│   ├─ compose.md                    # End-to-end composition workflow
│   ├─ music-theory-research.md      # How to research a genre + fill a profile
│   ├─ genre-profile-spec.md         # Profile schema reference
│   ├─ lego-pipeline.md              # Context-aware sequential generation
│   ├─ quick-research.md             # Inline lookups
│   └─ brainstorm.md                 # Multi-approach exploration
├─ genres/                           # Cached genre profiles (JSON)
│   ├─ _schema.json
│   ├─ lo-fi-hip-hop.json
│   ├─ metalcore.json                # Uses every profile field
│   └─ scenecore.json                # Seeded from Ace-Step DAW reference
├─ instruments/                      # VST maps: superior-drummer-3, odin-iii, gm-drums, _template
├─ composers/                        # Deterministic Python builders
│   ├─ theory.py                     # Scales, Roman numerals, voicings, tunings
│   ├─ profile.py                    # Load / blend profiles, section bindings
│   ├─ structure.py                  # Form / bars → sections (incl. micro)
│   ├─ harmony.py                    # Chord timeline + motif repetition
│   ├─ chords.py                     # Chords + pads
│   ├─ riff.py                       # Rhythm guitar riffs
│   ├─ drums.py                      # Bound grooves, fills, crashes, kick_follow
│   ├─ bass.py                       # Sustained / walking / kick-locked / doubled
│   ├─ melody.py                     # Melody + lead
│   └─ pipeline.py                   # Lego Pipeline + section regeneration
├─ skills/                           # Entry-point scripts
│   ├─ compose_from_profile.py       # Main CLI
│   ├─ generate_midi.py              # Composition → .mid
│   ├─ normalize_composition.py
│   ├─ refine_composition.py
│   └─ convert_to_wav.py             # MIDI → WAV (FluidSynth)
├─ midi_types/
│   ├─ music.py                      # Composition / Track / Note / Section
│   ├─ genre_profile.py              # Profile dataclasses
│   └─ gm_instruments.py
├─ resources/                        # Music theory lookup references
├─ plans/                            # Per-song briefs + composition.json
├─ claude_code/                      # Ace-Step DAW reference (for porting)
├─ soundfonts/                       # Place A320U.sf2 here
└─ output/                           # Generated .mid / .wav files
```

---

## Quick Start

```bash
pip install -r requirements.txt

# Single-genre 8-bar loop (default length)
python skills/compose_from_profile.py --genres lo-fi-hip-hop --title chillsong

# Blend genres
python skills/compose_from_profile.py \
    --genres scenecore,lo-fi-hip-hop --weights 0.6,0.4 \
    --bars 16 --title hybrid

# Regenerate just the drums in verse 2 B
python skills/compose_from_profile.py \
    --genres scenecore --resume plans/hybrid.composition.json \
    --regenerate verse2.B --layers drums --seed 17

# Full metalcore song for Superior Drummer 3 + Odin III, one .mid per track
python skills/compose_from_profile.py --genres metalcore --form \
    --vst drums=superior-drummer-3,rhythm_guitar=odin-iii --split-tracks --title riffs

# Tests
python -m unittest discover -s tests
```

Output goes to `plans/<title>.composition.json` and `output/<title>.mid`.

---

## Supported Instruments

All 128 General MIDI instruments are supported. Common examples:

| Category | Instruments |
|----------|-------------|
| Piano | `acoustic-grand-piano`, `electric-piano-1`, `harpsichord` |
| Guitar | `acoustic-guitar-nylon`, `electric-guitar-clean`, `distortion-guitar` |
| Bass | `acoustic-bass`, `electric-bass-finger`, `synth-bass-1` |
| Strings | `violin`, `cello`, `string-ensemble-1`, `orchestral-harp` |
| Brass | `trumpet`, `trombone`, `french-horn`, `brass-section` |
| Woodwind | `flute`, `clarinet`, `oboe`, `alto-sax` |
| Synth | `lead-1-square`, `pad-1-new-age`, `synth-strings-1` |

Simple aliases like `piano`, `guitar`, `bass`, `strings`, `brass`, `sax` are also supported.

See `midi_types/gm_instruments.py` for the full list.

---

## Installation

### 1. Install Python dependencies

```bash
pip install midiutil
```

Or using requirements.txt:

```bash
pip install -r requirements.txt
```

### 2. Download SoundFont (for WAV conversion)

1. Download **A320U.sf2** from [Musical Artifacts](https://musical-artifacts.com/artifacts/5906)
2. Place it in `soundfonts/A320U.sf2`

### 3. Install FluidSynth (for WAV conversion)

```bash
# macOS
brew install fluidsynth

# Linux
apt-get install fluidsynth
```

---

## Usage with Claude.ai

### 1. Enable Skills in Claude

1. Open **Claude Settings**
2. Enable **Skills**
3. Upload this repository as a **ZIP file**
4. Turn the skill **ON**

### 2. Use the Skill

Simply ask Claude something related to music:

```text
Added the “midi-generation” skill. Use it to create grand classical music. I look forward to bright, grand, and beautiful melodies layered together. Once you have the MIDI, please convert it to WAV.
```

Claude will automatically:
1. Detect that the request matches this skill
2. Use the provided Python scripts in `skills/` directory
3. Generate MIDI with proper GM instruments (each track on different channel)
4. Optionally convert to WAV

### Demo



https://github.com/user-attachments/assets/34f018e5-6354-40a9-9426-15d2ccc7c39a



---

## Composition Format

The skill uses a structured JSON format:

```json
{
  "title": "My Song",
  "bpm": 120,
  "tracks": [
    {
      "instrument": "acoustic-grand-piano",
      "notes": [
        { "pitch": "C4", "duration": "4" },
        { "pitch": "E4", "duration": "4" },
        { "pitch": "G4", "duration": "2" }
      ]
    },
    {
      "instrument": "acoustic-bass",
      "notes": [
        { "pitch": "C2", "duration": "1" }
      ]
    }
  ]
}
```

### Note Duration Values

| Value | Duration |
|-------|----------|
| `"1"` | Whole note (4 beats) |
| `"2"` | Half note (2 beats) |
| `"d2"` | Dotted half (3 beats) |
| `"4"` | Quarter note (1 beat) |
| `"d4"` | Dotted quarter (1.5 beats) |
| `"8"` | Eighth note (0.5 beats) |
| `"16"` | Sixteenth note (0.25 beats) |

---

## API Reference

### `generate_midi(composition: Composition) -> str`

Generate a MIDI file from a Composition object.

- **Input**: Composition object with title, bpm, and tracks
- **Output**: Path to the generated `.mid` file

### `generate_midi_from_dict(data: dict) -> str`

Generate a MIDI file from a dictionary.

- **Input**: Dictionary with title, bpm, and tracks
- **Output**: Path to the generated `.mid` file

### `convert_to_wav(midi_path: str, options: ConvertOptions) -> str`

Convert a MIDI file to WAV using FluidSynth and A320U.sf2.

- **Input**: Path to MIDI file
- **Output**: Path to the generated `.wav` file

### `normalize_composition(data: Any) -> Composition`

Validate and normalize user input into a proper Composition object.

### `refine_composition(composition: Composition, min_notes: int) -> Composition`

Adjust composition length (ensures minimum notes per track).

### `resolve_instrument(name: str) -> int`

Convert instrument name to GM program number (0-127).

---

## Example Prompts

```text
Create a calm ambient piece with pad synths and strings
```

```text
Generate a rock song with distortion guitar and drums at 140 BPM
```

```text
Compose a classical piece with violin, cello, and harpsichord
```

---

## Development Notes

- `SKILL.md` defines **when and how Claude should use this skill**
- The `skills/` directory contains **pre-built Python scripts that Claude must use**
- Claude should **NOT write custom code** - it should use the provided functions
- All instruments are GM-compatible for A320U.sf2 playback
- **Each track is assigned to a different MIDI channel** for proper multi-instrument playback

---

## License

MIT License

See LICENSE.txt for details including third-party licenses.

---

## References

- [A320U SoundFont](https://musical-artifacts.com/artifacts/5906)
- [General MIDI Specification](https://www.midi.org/specifications/midi1-specifications/general-midi-specifications)
- [Claude Agent Skills](https://docs.claude.com/en/docs/agents-and-tools/agent-skills/overview)
- [FluidSynth](https://www.fluidsynth.org/)
- [MIDIUtil](https://github.com/MarkCWirt/MIDIUtil)
