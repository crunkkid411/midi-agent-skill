# Brainstorm

> Multi-approach exploration when there are multiple valid ways to solve
> a musical problem. Adapted from Ace-Step DAW's `brainstorm` skill.

---

**Use this when**: 2+ viable approaches exist and the wrong choice would
be expensive to redo.

**Don't use this when**: There's an obvious approach, or the task is
small enough that any approach works.

## Examples Where Brainstorming Pays Off

1. **Genre blend strategy**: average tempos? Use highest-weight profile's tempo? Use the overlap range?
2. **Section regeneration scope**: regenerate one micro-section, or regenerate the whole verse to keep it coherent?
3. **Override approach**: should the user-provided bass line replace generation, or augment it (e.g. user provides the riff, generator fills the rest)?
4. **Profile fork vs. variant fields**: when a sub-style differs significantly, fork to a new profile or add `variant` weights to the existing one?

## Protocol (4 steps, under 5 minutes)

### Step 1: Define the Decision

```
DECISION: [What needs to be decided]
CONSTRAINTS: [Non-negotiable]
CONTEXT: [What already exists]
```

### Step 2: Generate 2-3 Approaches

For each:
- **Name** (short)
- **How it works** (2-3 sentences)
- **Pros**
- **Cons**
- **Effort** (S / M / L)

Rules:
- Min 2, max 4. Don't pad with bad options.
- At least one should be the "boring/simple" option.
- At least one should be the "ideal if we had unlimited time" option.

### Step 3: Score and Select

| Criterion | Weight | Description |
|-----------|--------|-------------|
| Correctness | 3x | Does it actually solve it? |
| Simplicity | 2x | How easy to maintain? |
| Compatibility | 2x | How well does it fit existing patterns? |
| Extensibility | 1x | How easy to extend later? |

Pick the highest score. Tie → simpler wins.

### Step 4: Document Briefly

If the decision affects shared infrastructure (the schema, the pipeline,
the composer interfaces), record it as a comment in the relevant file.
For one-off song decisions, just commit and move on.

## Anti-Patterns

- **Analysis paralysis**: 20 minutes comparing approaches for a 10-minute task.
- **Phantom options**: Including an approach you'd never pick, just to fill the slate.
- **Ignoring the codebase**: Proposing approaches that conflict with existing patterns.
- **Premature optimization**: Picking the "fastest" approach when any is fast enough.
