# Status (2026-11-05)

Version 0.1.0. First build. Nothing here has run inside Cubase yet.

## Verified

| What | How | Result |
|---|---|---|
| Core library | `midiagent_tests` on Linux, GCC 13 | 34 tests pass |
| Composition round trip with a real skill output | `fixtures/scenecore-8bar.composition.json` (made by `compose_from_profile.py --genres scenecore --bars 8 --seed 7`) | load, save, reload gives identical JSON; Python keys kept |
| Plug-in conformance | Steinberg `validator`, SDK 3.8.1, Linux build | 47 tests pass, 0 failed |
| Host behaviour | `midiagent_host_smoke` (SDK hosting classes) | every note of the played range reaches the MIDI output with in-block sample offsets; audio RMS 0.17 while playing; note offs equal note ons after stop; the preview loop plays while stopped; state round trip holds the JSON |
| Editor | SDK `editorhost` under Xvfb, driven with `xdotool` | opens at 1240x800; segment tiles select; track rows select; click adds a note; drag sets its length; `GENERATE` runs Python on the worker thread and reloads the result; `DRAG MIDI · SEG` writes a 6 chunk `.mid`; `DRAG WAV · SEG` writes a 7 s 24 bit `.wav`; `PREVIEW LOOP` toggles |
| Screenshots | `docs/screenshots/` | made from the runs above |

## Not verified

- Cubase 11 on Windows: loading, the MIDI output bus routing, `Render in
  Place`, the drag onto a track (VSTGUI Win32 file drag), the `GENERATE`
  path with the Windows Python launcher, the file dialogs.
- The MSVC build. The code uses only the standard library and the SDK, and
  `platform.cpp` has a Win32 branch (`CreateProcessW`, wide file APIs), but
  no Windows compiler has seen it yet. Expect small fixes.
- High DPI. The editor is a fixed 1240x800 and does not report
  `canResize`. VSTGUI scales on Windows with the system DPI.
- Audio quality of the sketch synth. It is a placeholder so renders exist.

## Known limits

- `REGEN SEG` / `REGEN TRACK` need named sections and a `role` on the track.
  Synthetic `bars 1-4` segments and hand-made tracks cannot be regenerated.
- The plug-in's MIDI export ignores `instruments/*.json` (keyswitches,
  velocity bands, drum lane remaps). For VST-mapped files use the Python
  CLI: `python skills/compose_from_profile.py --resume <json> --vst ...`.
- One worker job at a time. A second `GENERATE` while one runs is logged
  and ignored.
- `REGEN` writes the current composition to `plans/<title>.composition.json`
  inside the skill folder first (the pipeline reads it with `--resume`).
- Muting is saved in the JSON as `"mute": true`; the Python side ignores it.
- The editor pushes edits to the audio thread 120 ms after the last change.
  Notes under the playhead during that window play the old data.
- No undo. `CLEAR` and `REGEN` replace notes without asking.

## Roadmap (ideas, in order of value)

1. Run in Cubase 11 and fix what breaks. Record the results here.
2. Undo / redo for piano roll edits (keep a stack of `Track::notes` copies
   in `Session`).
3. Resizable editor (`canResize`, `checkSizeConstraint`, relayout).
4. Apply instrument maps on export (port `midi_types/instrument_map.py`
   reading `instruments/*.json`; the JSON parser is there).
5. Velocity lane under the piano roll.
6. Add / remove / rename tracks, pick the role and instrument.
7. Drum names from the track's instrument map instead of General MIDI.
8. A better synth, or sample playback from `soundfonts/`.
