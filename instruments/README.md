# Instruments — VST / Instrument Maps

Genre profiles say *what* to play. Instrument maps say *how a specific
instrument wants to receive it*. They are independent, so any genre can
target any instrument.

Composers write instrument-agnostic intent:
- drum notes carry a **lane** (`kick`, `snare`, `china`, `ride_bell`, `tom_mid`, …)
- pitched notes carry an **articulation** (`palm_mute`, `open`, `pinch_harmonic`, `dead`, …)

At render time `skills/generate_midi.py` applies the track's map:

| Map type | What it does |
|---|---|
| drums | lane → kit note (`lanes`), missing piece → another lane (`fallbacks`) |
| melodic | articulation → keyswitch note and/or velocity band; pitch folded into `range` (+ `octave_offset`) |

`articulation_mode`:
- `none`: notes only
- `velocity`: the articulation picks a velocity band (e.g. Odin III Swift Mode)
- `keyswitch`: the articulation's keyswitch note is sent. `keyswitch_latching: false` holds it for every note it affects (Odin III). `true` sends one short press whenever the articulation changes.
- `keyswitch+velocity`: a keyswitch where one is known, otherwise the velocity band

## Using a map

```bash
# Profile default (instruments[].vst in the genre JSON)
python skills/compose_from_profile.py --genres metalcore --form

# Any VST on any genre, per role
python skills/compose_from_profile.py --genres lo-fi-hip-hop --form --vst drums=superior-drummer-3

# One .mid per track for a DAW template
python skills/compose_from_profile.py --genres metalcore --form --split-tracks

python skills/compose_from_profile.py --list-vsts
```

VST tracks get no program change. Load the plugin on that track (or that
file's track) in your DAW.

## Shipped maps

### superior-drummer-3 (aliases: sd3)
Toontrack's default note map:
- kick 36, snare 38, rimshot 40, sidestick 37
- hats: closed 42, tight 63, pedal 44, open 46, loose 60
- cymbals: crash 49 / 57 / 28, china 27 / 52, ride 51, ride bell 53, splash 55
- toms: 48 / 47 / 45 / 43 / 41

These are the same notes metalerator targets for SD3. If you remapped SD3
under Settings › MIDI In, edit `lanes`. Keep velocities varied, because
SD3's velocity layers make flat velocities sound robotic.

### odin-iii (aliases: odin3)
Solemn Tones Odin III, an 8-string baritone guitar sampler (VST3/AU/AAX, no
Kontakt). It goes down to double drop C (MIDI 24).
- **Works out of the box through Swift Mode.** Odin III picks the articulation from velocity: low velocity gives chugs and scrapes, mid-low gives tight palm mutes, mid-high gives alternate picking, and high gives pinch harmonics. The map's `velocity_bands` are estimates of those ranges. Play a held note at a few velocities and adjust the edges if needed.
- **Keyswitches are non-latching.** The renderer holds each keyswitch across the notes it affects. Solemn Tones doesn't publish the exact keyswitch notes. Open the keyswitch list (top-right of the plugin UI) and fill the `null` entries in `odin-iii.json`. Each one you fill switches that articulation from a velocity band to a keyswitch automatically.
- Set Odin's tuning to match the profile (metalcore: drop C). Leave string select on Auto.
- Double-tracked parts come out as `Rhythm Guitar L` and `Rhythm Guitar R`. Use the dual panned preset or two instances.
- If notes land an octave off from your tab/DAW convention, set `octave_offset` to ±12 (or use Odin's Octave Drop button).

### gm-drums (aliases: gm)
General MIDI percussion, the implicit default for drum tracks with no VST.

## Adding a VST

1. Copy `_template.json` to `<slug>.json`.
2. Drums: fill `lanes` (lane → note) and `fallbacks`. Melodic: set `range`, `articulation_mode`, `keyswitches` and/or `velocity_bands`, and `keyswitch_latching`.
3. Research the instrument (manual, keyswitch chart, MIDI map), cite it in `notes`, and mark anything you couldn't verify.
4. Point a profile role at it (`"vst": "<slug>"`) or pass `--vst role=<slug>`.
