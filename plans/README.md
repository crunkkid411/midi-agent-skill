# Plans — Per-Song Composition Briefs and Outputs

This directory holds the per-song artifacts:

- **`<title>.md`** — the human-readable composition brief. Genre, key, BPM, principles, structure, tracks. Written by the agent before running the pipeline; reviewed by the user; informs the eventual self-evaluation.
- **`<title>.composition.json`** — the machine-readable composition produced by `skills/compose_from_profile.py`. Contains all sections, tracks, and notes. Re-runnable.
- **`<title>-<role>.json`** — optional. A user-supplied or hand-edited single-track override (chord progression, bassline, drum break, etc.) that gets passed into the pipeline via `--override-<role>`.

## Workflow

1. Agent writes `<title>.md` (the brief) per `workflows/compose.md` Step 3.
2. User approves the brief.
3. Agent runs the pipeline → `<title>.composition.json` + `output/<title>.mid`.
4. User listens, requests changes.
5. Agent edits the profile (if generic) or writes an override Track JSON (if specific), then either re-renders or `--regenerate`s a single section.

## File Naming

- Lowercase-hyphenated to match `<title>` slug used on the CLI.
- Brief is markdown; composition is JSON; override Tracks are JSON.

## Override Track JSON Format

A single Track that the pipeline will use instead of generating that role:

```json
{
  "name": "Custom Bass",
  "instrument": "fingered-bass",
  "is_drum": false,
  "notes": [
    { "pitch": "D2", "duration": "2", "velocity": 0.7, "start": 0 },
    { "pitch": "A2", "duration": "2", "velocity": 0.6, "start": 2 },
    { "pitch": "F2", "duration": "1", "velocity": 0.65, "start": 4 }
  ]
}
```

Fields:
- `pitch`: note name (`"C4"`, `"F#5"`) or numeric MIDI for drum tracks (`"36"` = kick).
- `duration`: `"4"` quarter, `"8"` eighth, `"d4"` dotted-quarter, etc.
- `velocity`: 0.0-1.0 (or 0-127). Optional; defaults to 100/127.
- `start`: beats from start of song. Optional; without it, notes are placed sequentially.

For drum overrides, set `"is_drum": true` and use numeric pitches (36=kick, 38=snare, 42=closed-hh, 46=open-hh, 49=crash, 51=ride).
