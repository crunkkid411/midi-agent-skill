# Music Theory Research — How to Fill a Genre Profile

> Process-oriented research workflow. Adapted from the Ace-Step DAW
> `music-theory-engine` skill. The output of this process is a JSON file
> in `genres/` that the deterministic composers consume forever after.

---

## Core Principle

**Real composers don't memorize all theory — they research what they need for each piece.**

Your workflow when asked to compose in a genre that doesn't yet have a profile:

```
1. RESEARCH   → What does this genre/style actually sound like?
2. ANALYZE    → What patterns, chords, rhythms, song structures define it?
3. EXTRACT    → What are the 2-4 key principles and section conventions?
4. CAPTURE    → Fill out genres/<slug>.json against genres/_schema.json
5. EVALUATE   → Compose a short loop, listen, refine the profile
```

---

## Phase 1: RESEARCH — Finding the Right References

For a genre you need to understand:

1. **Search for theory analysis**:
   - WebSearch: `"{genre}" chord progressions analysis`
   - WebSearch: `"{genre}" song structure typical arrangement`
   - WebSearch: `"{genre}" drum pattern programming rhythm`
   - WebSearch: `"{genre}" bass line techniques production`

2. **Find specific reference songs**:
   - WebSearch: `"{song name}" chord progression key BPM`
   - WebSearch: `"{song name}" music analysis breakdown`
   - Hooktheory, Chordify, Ultimate Guitar are particularly useful.

3. **Find micro-section conventions**:
   - WebSearch: `"{genre}" verse chorus structure breakdown`
   - WebSearch: `"{genre}" drop intro outro arrangement`
   - This step is essential for filling `song_sections[].micro` — without
     it, the agent loses the ability to make nuanced section-level requests work.

4. **Find production / instrumentation conventions**:
   - WebSearch: `"{genre}" instrumentation typical sounds`
   - WebSearch: `"{genre}" production techniques mix`

### What to Extract

For each genre, identify:
- **Key/Scale**: which key and scale dominate? (e.g. minor pentatonic for blues, Dorian for lo-fi)
- **Chord Language**: triads? 7ths? 9ths? Power chords? Open voicings?
- **Rhythmic Feel**: straight 8ths, swing, syncopated? What's the kick/snare pattern?
- **Texture**: sparse or dense? Which instruments? Which register?
- **Form**: how long are sections? Are there sub-sections (A/B, drop, break)? What's the energy curve?
- **BPM range**: lower and upper bounds.

---

## Phase 2: ANALYZE — Extracting Patterns

### Chord Progression Analysis

When you find a reference song's chords:

1. **Identify the key** — what note feels like home?
2. **Convert to Roman numerals** — captures the pattern key-independently:
   - Major key: I, ii, iii, IV, V, vi, vii°
   - Natural minor: i, ii°, III, iv, v, VI, VII
   - In practice, minor uses V (major from harmonic minor) for dominant function — the raised 7th creates the leading tone.
3. **Note voicings** — triads or extensions (7ths, 9ths)? This becomes `voicing_style`.
4. **Note rhythm** — bars per chord? Syncopated entry? This becomes `bars_per_chord`.

### Drum Pattern Analysis

1. **Map the pulse** — where do you tap your foot?
2. **Map the kick** — where are the bass drum hits in the 16-step bar?
3. **Map the snare/clap** — typically beats 2 & 4 (backbeat) or elsewhere?
4. **Map the hi-hat** — what subdivision (8ths, 16ths, triplets)?
5. **Note ghost notes** — soft hits between main beats.
6. **Note swing amount** — straight (0.5), light swing (0.55-0.6), hard swing (0.65+).

Encode each pattern as a 16-step grid string per lane:
- `'x'` = hit, `'.'` = rest, `'g'` = ghost note (low velocity)
- 16 chars covers one bar at 16th-note resolution. Use 32 chars for triplet-heavy patterns.

### Song Structure Analysis (the nuance layer)

For each major section (intro, verse, chorus, drop, bridge, outro), look for **micro-sections**:
- Does the verse split into A (sparse) and B (denser, building)?
- Does the drop have a peak section then a variation?
- Are there 1-2 bar drum breaks between sections?

Record these in `song_sections[].micro` with the dotted name convention:

```json
{
  "name": "verse",
  "bars": 8,
  "micro": [
    { "name": "A", "bars": 4, "density": 0.6, "note": "Sparse, build" },
    { "name": "B", "bars": 4, "density": 0.85, "note": "Full kit kicks in" }
  ]
}
```

This is what makes `regenerate_section('verse2.B', layers=['drums'])` viable.

---

## Phase 3: EXTRACT — Distill Principles

Reduce your research to **2-4 principles** (default: 3). More than 4 leads
to over-constrained, mechanical output. Fewer than 2 is too vague. Simple
genres (punk, ambient) may need only 2; complex (jazz, progressive) may
need 4.

Examples:

**Lo-Fi Hip-Hop**
1. Jazz-influenced harmony with 7th/9th voicings and Dorian color
2. Laid-back boom-bap drums with ghost notes, light swing, 70-90 BPM
3. Sparse pentatonic melody with lots of space and reverb/delay

**EDM Drop**
1. Four-on-the-floor kick with off-beat hi-hats at 128 BPM
2. Minor key, simple progression (often just 2 chords), heavy bass
3. Energy contrast: stripped breakdown → full drop

**Jazz Ballad**
1. Extended harmony (maj7, m9, 13) with smooth voice leading
2. Rubato/free timing feel, brushes on drums, walking or pedal bass
3. Melody uses chromatic approach tones, telling a story with dynamics

These go into `profile.principles` as plain English — the agent uses them
in the composition brief and in the self-evaluation step.

---

## Phase 4: CAPTURE — Write the Profile

1. Copy `genres/lo-fi-hip-hop.json` as a starting template.
2. Validate against `genres/_schema.json`.
3. Use lowercase-hyphenated slug for `name` and filename.
4. Add aliases for spelling variants (`"lofi"`, `"lo-fi"`, etc).
5. Cite at least 2 specific references in `references[]`.
6. Note research sources in `research_sources[]` (URLs, books, podcast eps).

---

## Phase 5: EVALUATE — Validate the Profile

Before committing, sanity-check by composing a short loop:

```bash
python skills/compose_from_profile.py --genres <slug> --bars 8 --title test_<slug>
```

Then:
1. Listen to `output/test_<slug>.mid` (any GM-capable player works).
2. Compare against your reference songs. Does it pass the "blind genre test"?
3. If not, iterate: tweak chord weights, drum pattern, register, voicing style. Recompose.
4. When it passes, commit the profile.

---

## When You Don't Know Something

**Don't guess — research.**

| Situation | Action |
|-----------|--------|
| Don't know the genre's typical chords | WebSearch: `"{genre}" common chord progressions` |
| Don't know the right scale | WebSearch: `"{genre}" scales modes used` |
| Don't know the drum pattern | WebSearch: `"{genre}" drum pattern programming` |
| Don't know the BPM range | WebSearch: `"{genre}" typical BPM tempo` |
| Don't know section conventions | WebSearch: `"{genre}" verse chorus structure` |
| Don't know instrumentation | WebSearch: `"{genre}" instrumentation typical sounds` |

**The MIDI pitch table is the one thing worth memorizing:** Middle C = C4 = MIDI 60. Each octave = 12 semitones.

---

## Anti-Patterns

1. **Don't capture everything** — extract 2-4 principles, not 20.
2. **Don't trust a single source** — cross-reference at least 2.
3. **Don't skip the listen-test** — a profile that compiles but doesn't sound right wastes the next 50 compositions.
4. **Don't bake user-specific preferences into the profile** — keep those in plans/ briefs. The profile is the genre, not the song.
