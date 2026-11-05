# Guide for the next agent

Read this before you change anything under `vst3/midi-agent`.

## Where things are

| You want to change | Go to |
|---|---|
| how a composition JSON is read or written | `core/composition.cpp`, test in `tests/test_composition.cpp` |
| what a segment is | `core/segments.cpp` |
| the `.mid` or `.wav` files | `core/midifile.cpp`, `core/wavfile.cpp`, `core/renderer.cpp` |
| the sound | `core/synth.cpp` (timbres per role in `timbreForTrack`) |
| playback timing | `core/sequencer.cpp` (pure), `source/processor.cpp` (host clock, preview loop) |
| the Python command line | `core/agent_bridge.cpp`, test in `tests/test_agent_bridge.cpp` |
| a button, a panel, the layout | `source/editor.cpp` (`buildHeader`, `buildActionBar`, `buildGeneratePanel`) |
| piano roll behaviour | `source/ui/pianoroll_view.cpp` |
| colours, fonts, chrome | `source/ui/theme.cpp`, documented in `docs/UI-STYLE.md` |
| messages between controller and processor | `source/messages.h` |
| parameters, class IDs | `source/plugids.h` (never change the FUIDs) |

## The loop

1. Change code.
2. `cmake --build build/midi-agent && ctest --test-dir build/midi-agent --output-on-failure`
   (core tests and the host smoke test).
3. For a UI change on Linux: `scripts/gui-screenshot-linux.sh` and look at
   the PNG. For interaction, drive it with `xdotool` like the script shows.
4. Update `docs/STATUS.md` with what you verified and how.
5. Keep the composition JSON compatible with `midi_types/music.py`. The
   round trip test with the fixture must stay green. If the Python schema
   grows a field, add it to `Composition::fromJson` / `toJson` and to the
   fixture.

## Rules kept from the Cubase AI Toolkit

- Musical positions are ticks at 480 per quarter note, bars and beats.
  Never seconds.
- A damaged file is an error with a reason, never an empty song
  (`Composition::fromString` returns false and the editor logs `ERROR ...`).
- Standard library plus the VST3 SDK. No other dependency without a written
  reason in the pull request.
- The SDK is used the way Steinberg's examples use it: `AudioEffect`,
  `EditController`, `VSTGUIEditor`, `IMessage`, `smtg_add_vst3plugin`.

## How to regenerate the fixture

```bash
python skills/compose_from_profile.py --genres scenecore --bars 8 --seed 7 --title vst-fixture
cp plans/vst-fixture.composition.json vst3/midi-agent/fixtures/scenecore-8bar.composition.json
```

Then adjust the numbers in `tests/test_composition.cpp` and
`tests/test_segments.cpp` if the pipeline output changed.

## How to regenerate `core/gm_instruments.cpp`

```bash
python vst3/midi-agent/scripts/gen_gm_instruments.py
```

## Testing on Windows with Cubase

Follow `cubase-ai-toolkit/FACTORY_RULES.md` section 4 for anything that
opens Cubase: only a copy of `TEST-FILE.cpr`, never a real project. The
plug-in itself touches no `.cpr` file and only writes under `%TEMP%\midi-agent`
and the skill's `plans/` folder.
