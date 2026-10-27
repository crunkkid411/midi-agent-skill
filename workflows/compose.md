# Compose — Research-First Composition Workflow

> Process-oriented composition for AI agents using midi-agent-skill.
> Adapted from the Ace-Step DAW `compose` skill, retargeted from Strudel
> output to deterministic Python composers + MIDI.

---

## Core Principle

**Compose like a musician, not a template engine.**

A musician doesn't have all chord progressions memorized. They:
1. Listen to references in the target style
2. Analyze what makes those references work
3. Extract a few key principles
4. Create something new guided by those principles
5. Iterate until it feels right

This skill follows the same process — and the principles get cached as a
genre profile in `genres/` so subsequent songs in that style are instant
and deterministic.

---

## The Composition Process

### Step 0: Parse the User's Intent

Extract from their description:
- **Genre/Style** — the most important signal. Multiple genres → blend.
- **Reference artists/songs** — if mentioned, these are gold.
- **Mood/Energy** — calm, energetic, dark, uplifting, nostalgic.
- **Key** — if specified; otherwise, profile default or research.
- **BPM** — if specified; otherwise, profile midpoint.
- **Special constraints** — "no drums", "piano only", "with strings".
- **Length** — bars (default **8** — typical loop length). Don't compose 32 bars unless asked; 8 bars is the canvas most users want and it scales naturally to longer arrangements via the `regenerate_section` API.

**If the description is vague**, ask for clarification. A wrong assumption wastes a full generation cycle.

### Step 1: RESEARCH the Genre/Style

Check `genres/<slug>.json` first:

- **Profile exists** → load it; skip to Step 3 unless the user asked for something the profile doesn't cover.
- **Profile missing** → run the full research process in `workflows/music-theory-research.md`, then save the result to `genres/<slug>.json`.

For multiple genres in a blend, **each one** must have a profile (or be researched). Blending uses the profiles' weighted union.

### Step 2: ANALYZE and Extract Principles

After research, distill **2-4 principles** (default: 3). More than 4 → mechanical output. Fewer than 2 → too vague.

The principles get written into `profile.principles` as plain English strings. The composers don't read these directly, but they do drive:
- Drum pattern selection (energy)
- Voicing style (stab vs sustained)
- Bass style (sub-808 vs walking)
- Melody style (sparse pentatonic vs fast arpeggio)

### Step 3: Write a Composition Brief

Before running the pipeline, present the plan to the user as a markdown
file in `plans/<title>.md`:

```markdown
# <Song Title>

**Genre:** Lo-Fi Hip-Hop
**Reference:** Nujabes — Feather
**Key:** D Dorian
**BPM:** 78
**Bars:** 8

## Principles
1. Jazz-rooted harmony — i9 → iv9 → iiø7 → V7b9 with 7th/9th voicings
2. Boom-bap foundation — lazy kick, ghost snares, soft hi-hats with light swing
3. Warm sparse texture — Rhodes keys, sub bass, pentatonic vibraphone lead

## Structure
- **Loop A** (bars 1-4): Drums + bass + chords, no melody
- **Loop B** (bars 5-8): Add melodic lead, slightly more open hat

## Tracks
- Drums (channel 9, GM kit)
- Bass (electric-bass-finger, sustained roots)
- Chords (electric-piano-1, sparse held)
- Melody (vibraphone, pentatonic sparse)
```

**Wait for user approval** before running the pipeline. Briefs are cheap to revise; full re-renders are not.

### Step 4: COMPOSE — Run the Lego Pipeline

```bash
python skills/compose_from_profile.py \
    --genres lo-fi-hip-hop \
    --bars 8 \
    --title my_lofi_song \
    --seed 42
```

For a blend:

```bash
python skills/compose_from_profile.py \
    --genres scenecore,witch-house,shoegaze \
    --weights 0.5,0.3,0.2 \
    --bars 8 --title hybrid_song
```

For a full song, walk the profile's `form` template (intro → verse → … →
outro, optional slots rolled with the seed):

```bash
python skills/compose_from_profile.py --genres metalcore --form --seed 42 --title my_song
```

To render for specific instruments, target any instrument map in
`instruments/` from any genre (`--list-vsts` shows them). `--split-tracks`
also writes one .mid per track for dragging into a DAW template:

```bash
python skills/compose_from_profile.py --genres metalcore --form \
    --vst drums=superior-drummer-3,rhythm_guitar=odin-iii,lead=odin-iii \
    --split-tracks --title my_song
```

For user-provided ideas (e.g. they wrote a bass line in `plans/my-bass.json`):

```bash
python skills/compose_from_profile.py \
    --genres scenecore --bars 8 --title with_user_bass \
    --override-bass plans/my-bass.json
```

### Step 5: EVALUATE Against Principles

After the pipeline runs, verify against the brief's principles:

1. **Principle 1 check** — do the chords match the harmonic character? Open the composition.json and inspect the chord track's note groups.
2. **Principle 2 check** — does the rhythm feel right for the genre? Inspect the drum track.
3. **Principle 3 check** — is the texture/density right? Compare across sections.
4. **Key consistency** — every pitched note must belong to the song key (or be a documented chromatic accent).
5. **Velocity variation** — no flat velocity anywhere. Each track should have a range.
6. **Musical space** — are there enough rests? Especially the melody.

If anything fails, regenerate the offending layer or section (see Step 6).

### Step 6: Iterate — Targeted Regeneration

The whole point of micro-sections is that you can ask for nuanced changes:

> "Make the drums in verse 2 B a low-fi drum break."

```bash
python skills/compose_from_profile.py \
    --genres scenecore \
    --resume plans/song.composition.json \
    --regenerate verse2.B \
    --layers drums \
    --seed 17
```

That re-renders only the drum track inside `verse2.B` (4 bars) and leaves
everything else untouched. You can also temporarily edit the profile (e.g.
swap in a `halftime-drop` pattern weight) and regenerate just that section.

For broader changes:
- **Different drum pattern in one section** → set that section's `drum_pattern` binding in the profile (a name, or a list for one pattern per phrase), regenerate that section.
- **Different riff / bass / melody feel in one section** → set `riff_style`, `bass_style`, `melody_style`, `scale` or `progression` on the section or micro-section.
- **Silence a part somewhere** → add the role to the section's `mute` list. A one-hit pause → a section with `stop: true`.
- **Add a counter-melody** → write a Track JSON, pass it as `--override-lead`.
- **Change BPM** → re-run with `--bpm`. Note density may need adjustment.
- **Change key** → re-run with `--key 'F minor'`.
- **Change genre** → start over from Step 1 — genre defines everything.

---

## Handling Specific Requests

### "Make it sound like [artist/song]"

This is the highest-value input. Research heavily:
1. WebSearch: `"{song}" chord progression analysis`
2. WebSearch: `"{artist}" production style breakdown`
3. Update the relevant genre profile's `chord_progressions`, `references`,
   and `principles` — or fork to `<slug>-<artist>.json` if the variation is
   distinct enough to deserve its own profile.

### "I don't know what I want"

Guide with questions:
- Mood? (happy / sad / energetic / chill / dark / dreamy)
- Genre preference, or pick based on mood?
- Fast or slow?
- Instrument preferences?

Then research per Step 1.

### "Add [specific instrument/part]"

1. Write the new layer respecting the existing key + chord plan.
2. Save as `plans/<song>-<part>.json` in the Track shape.
3. Pass via `--override-<role>` and re-render.

### "Add a drum break in verse 2's B section"

This is the canonical micro-section workflow:
1. Set `"drum_pattern": "halftime-drop"` (or any pattern) on the verse's `B` micro-section — or write a custom drum track JSON.
2. Run with `--resume`, `--regenerate verse2.B`, `--layers drums`. The regenerated section keeps its bindings (energy, mutes, fills).

---

## What This Workflow Does NOT Include

- **Pre-built song templates** — research the genre once, cache the profile, generate forever.
- **Per-note AI composition** — that's expensive and rate-limited. The pipeline is deterministic.
- **Strudel/TidalCycles output** — that's the Ace-Step DAW. Here we emit Python composition JSON → MIDI.
- **Mixing, mastering, sound design** — the `sound_design` field records preset/FX notes; instrument maps (`instruments/`) handle note mapping and articulations, not tone.

The process is the value. The process adapts to any genre, any style, any reference. A template only works for the exact case it was written for.
