# Quick Research

> Lightweight inline research for agents who need to look something up
> mid-task without derailing into a full research cycle.
> Adapted from Ace-Step DAW's `quick-research` skill.

---

**Use this when**: You hit an unknown API, an unfamiliar piece of music
theory, or need to verify a technical approach — and the answer is
findable in 2-3 searches.

**Don't use this when**: You're researching a whole new genre. Use
`workflows/music-theory-research.md` instead — that produces a profile
that gets cached forever.

## When to Trigger

1. **Unknown API**: about to call a function you haven't used before.
2. **Error loop**: failed the same thing twice — stop guessing, search.
3. **Best-practice question**: "Is there a better way to express this in midiutil / GM / Roman numerals?"
4. **Theory edge case**: "What's the typical voicing for `iiø7` in a minor jazz cycle?"
5. **Genre-defining song verification**: "Is X actually a defining lo-fi reference, or am I hallucinating?"

## Protocol (3 steps, under 2 minutes)

### Step 1: Frame the Question (10 seconds)

Write a single sentence: *"I need to know X because Y."*

Bad: "Research lo-fi chords."
Good: "I need to know if `iiø7 → V7b9 → i` is a real lo-fi cadence or just jazz, because the user asked for chillhop and I want to use it."

### Step 2: Search (2-3 queries max)

- Include the specific term/genre/song.
- Add the year if the API/genre may have shifted recently.
- If the first credible result answers the question, stop. Don't search for confirmation.

### Step 3: Extract and Apply (30 seconds)

From the results, extract:
- **The answer** (1-2 sentences)
- **The source** (URL)
- **Any gotcha** (common mistake to avoid)

Then return to your task. Don't write a research document — that's for the full research workflow.

## Anti-Patterns

- **Research rabbit hole**: 5+ searches on a tangent. If 3 don't answer it, escalate to the full research workflow.
- **Confirmation searching**: You know the answer but keep looking. Trust the first credible source.
- **Over-documenting**: Writing a summary for a 30-second lookup. Just apply it.
- **Searching for things in the codebase**: If the answer is in the project files, use Grep/Read, not WebSearch.

## Output

No formal output. Apply the finding to your current task.
If the finding is non-obvious, leave a short comment in the relevant file
or update the genre profile's `notes` field.
