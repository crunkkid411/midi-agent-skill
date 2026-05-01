# Lego Pipeline — Context-Aware Sequential Generation

> Adapted from Ace-Step DAW's "Lego Pipeline" concept: each layer of a
> composition is built knowing the cumulative result of all previous
> layers. In Ace-Step this used the 1.5B-parameter ACE-Step audio model;
> here we achieve the same coherence with deterministic Python rules.
> Proper music theory + a well-researched genre profile gets us most of
> the way there.

---

## The Order

```
structure → chords → drums → bass (locks to drums) → melody (reads drums + chords) → lead
```

Each module lives in `composers/`:

| Layer | Module | Reads |
|-------|--------|-------|
| Structure | `structure.py` | profile, requested bars |
| Chords | `chords.py` | profile, structure |
| Drums | `drums.py` | profile, structure |
| Bass | `bass.py` | profile, chord plan, **drum track** |
| Melody | `melody.py` | profile, chord plan, **drum track** |
| Lead | `melody.py` (with `role="lead"`) | profile, chord plan, drum track |

The orchestrator in `composers/pipeline.py` runs them in this order and
returns a `Composition`.

## Why This Order

1. **Structure first** — bar count and section plan dictate everything downstream. Section density modulates layer behavior.
2. **Chords before drums** — the harmonic plan is the spine. Even `i-VI-III-VII` decided here means everyone speaks the same key.
3. **Drums before bass** — bass needs to lock to the kick (especially for sub-808 / trap styles). Bass composers receive the drum track and can read kick hit times.
4. **Bass before melody** — melody composers can already see chord roots through the chord track, but having the drum track also lets them rest over busy drum moments.
5. **Lead last** — sits on top of everything; the most expressive, the most rests.

## Layer Override Hooks

Any layer can be replaced with user-supplied content:

```python
from composers.pipeline import compose
from midi_types.music import Track, Note

user_bass = Track(
    name="Bass",
    instrument="fingered-bass",
    notes=[Note("D2", "2", velocity=0.7), Note("A2", "2", velocity=0.6), ...]
)

composition = compose(
    profile=profile,
    bars=8,
    overrides={"bass": user_bass},
)
```

When a layer is overridden, downstream composers still read it. So if the
user provides a custom bass line, the melody composer will still see it
(via the cumulative track list) and can be made to respect it. (Today the
melody composer uses chord tones, not the bass — extending it to read the
bass is a natural future addition.)

## Section-Level Regeneration

The pipeline exposes `regenerate_section(composition, profile, name, layers, seed)`
which:

1. Locates the named section.
2. Re-runs the requested layers for just that bar range.
3. Splices the new notes into the existing composition, time-shifted.
4. Returns a new Composition. Other sections and layers are untouched.

This is the foundation for "add a drum break in verse 2 B" — the agent
edits the profile (or temporarily substitutes a different drum pattern)
and calls regenerate_section for the target.

## What Each Layer Knows About the Others

- **Chords** know: key, scale, progression, voicing style, chord register.
- **Drums** know: section density (drops hi-hats / halves kicks at low density), pattern energy, swing, ghost density.
- **Bass** knows: chord roots and fifths in its register, the kick pattern (for 808/lock styles), its style descriptor (sustained / walking / arpeggio / 808-locked).
- **Melody** knows: chord tones (for downbeat targeting), section density (rests over moderate density), its scale (pentatonic for sparse styles, full diatonic for fast arpeggios).
- **Lead** knows: same as melody but rendered with the `role="lead"` instrument config (typically bright, higher register, more aggressive style like `fast_arpeggio`).

## Density-Driven Behavior

Each section has a `density` field (0.0-1.0). Composers consult it to:

- **Drums**: drop hi-hat lane below 0.35 density. Halve clap below 0.4.
- **Drums**: pick the lowest-energy pattern for sections under 0.4 density.
- **Melody**: skip the section entirely below 0.5 density. Reduce note probability per slot above that.
- **Bass**: not currently density-aware (bass is the spine). Future work: switch from sustained to walking at high density.

Set `density` per micro-section to control intra-section dynamics — this
is how a verse can have a sparse A and a dense B without breaking the
chord/key continuity.

## Extending the Pipeline

To add a new layer (e.g. "pad" or "fx"):

1. Add a new role to the genre profile schema (already permitted: `pad`, `fx`).
2. Add a composer module (e.g. `composers/pad.py`) following the same pattern: `generate_pad(profile, sections, chord_plan, drum_track, ...) -> Track`.
3. Add it to the `LAYER_ORDER` list in `pipeline.py` at the appropriate position. (Pads typically run after chords but before melody.)
4. Wire it into `compose()` with the same override-hook pattern.

The pipeline is intentionally simple — adding a layer should be ~50 lines
of new code plus 5 lines of orchestration.
