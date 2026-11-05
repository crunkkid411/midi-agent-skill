# Building MIDI Agent

## Requirements

| Need | Windows (the target) | Linux (CI, tests) |
|---|---|---|
| Compiler | Visual Studio 2022 Build Tools (`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`), C++ workload | GCC 13 or Clang 16 |
| CMake | 3.25 or newer | 3.25 or newer, Ninja recommended |
| VST3 SDK | `X:\AI-2\Steinberg-VST-factory\vst3sdk` (the SDK with `vstgui4/` inside), or `git clone --recursive https://github.com/steinbergmedia/vst3sdk` | same |
| VSTGUI Linux packages | - | `libx11-xcb-dev libxcb-util-dev libxcb-cursor-dev libxcb-keysyms1-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libfontconfig1-dev libcairo2-dev libpango1.0-dev libsqlite3-dev libglib2.0-dev libfreetype-dev libexpat1-dev libgtkmm-3.0-dev libwayland-dev wayland-protocols` |
| Python | 3.10+ with `midiutil` (for the generate buttons, not for the build) | same |

The SDK version this was developed against is VST 3.8.1 (vstgui 4.15). The
SDK is MIT licensed.

## Windows

```powershell
cd <repository root>
cmake -S vst3/midi-agent -B build/midi-agent -G "Visual Studio 17 2022" -A x64 `
      -DSMTG_VST3_SDK_DIR=X:/AI-2/Steinberg-VST-factory/vst3sdk
cmake --build build/midi-agent --config Release
ctest --test-dir build/midi-agent -C Release --output-on-failure
```

`scripts/build-windows.ps1` runs the same commands and accepts `-SdkDir`
and `-Config`.

Output: `build\midi-agent\VST3\Release\MIDI Agent.vst3` (a folder bundle with
`Contents\x86_64-win\MIDI Agent.vst3` and `moduleinfo.json`). Copy the
bundle to `C:\Program Files\Common Files\VST3\`. Cubase 11 scans that folder
on start.

Steinberg's validator runs as part of the build when
`-DSMTG_RUN_VST_VALIDATOR=ON` is passed (the SDK default). The build log then
holds the validator report.

## Linux

```bash
cmake -S vst3/midi-agent -B build/midi-agent -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DSMTG_VST3_SDK_DIR=$HOME/vst3sdk
cmake --build build/midi-agent -j
ctest --test-dir build/midi-agent --output-on-failure
```

`scripts/build-linux.sh` does this with `VST3_SDK_DIR` from the environment.
The plug-in bundle lands in `build/midi-agent/VST3/Release/MIDI Agent.vst3`.
Cubase does not run on Linux; this build exists for compile checks, the
tests, the validator and GUI screenshots.

## Core only (no SDK)

```bash
cmake -S vst3/midi-agent -B build/core -DMIDIAGENT_BUILD_PLUGIN=OFF
cmake --build build/core && ./build/core/midiagent_tests
```

This builds `core/` and the unit tests in a few seconds. Most logic changes
can be developed and tested this way.

## Tests

| Target | What it checks | How it runs |
|---|---|---|
| `midiagent_tests` | JSON, composition round trip with the fixture, segments, MIDI and WAV writers, synth, sequencer, Python command lines | `ctest` or run the binary |
| `midiagent_host_smoke` | loads the built `.vst3` with the SDK hosting classes: bus setup, composition message, 2 bars of process calls with a playing transport, MIDI out event count equals the note count, audio energy, note offs after stop, the preview loop, state round trip | `ctest` (needs the plug-in) |
| `validator` (SDK) | Steinberg's conformance tests | `build/midi-agent/bin/Release/validator "build/midi-agent/VST3/Release/MIDI Agent.vst3"` |

## Opening the GUI without a DAW (Linux)

`scripts/gui-screenshot-linux.sh` starts Xvfb, builds the SDK `editorhost`
sample, opens the plug-in editor with the fixture loaded and writes a PNG.
It is how the screenshots in `docs/screenshots/` were made and how a future
agent can see a UI change without Cubase. The environment variables
`MIDIAGENT_AUTOLOAD` (a composition JSON) and `MIDIAGENT_SKILL_ROOT` are read
by the controller on start for this purpose.

Two Linux details the script handles: `editorhost` needs the `_XEMBED_INFO`
atom to exist on the X server (a tiny Xlib client interns it and stays
connected), and the SDK's `editorhost` is built from the SDK tree with
`SMTG_ENABLE_VST3_HOSTING_EXAMPLES=ON`.

## Troubleshooting

- `SMTG_VST3_SDK_DIR is empty`: pass the SDK root (the folder with
  `pluginterfaces/`, `public.sdk/`, `vstgui4/`).
- `vstgui4 not found`: the SDK was cloned without `--recursive`.
- Cubase does not list the plug-in: check `moduleinfo.json` exists inside the
  bundle and that the bundle is in `Common Files\VST3`. Cubase 11 ignores
  plug-ins whose validator run failed only when the plug-in is blocklisted;
  check `Studio > VST Plug-in Manager`.
- `GENERATE` says `could not start python`: set `PYTHON` to a full path, for
  example `C:\Users\<you>\AppData\Local\Programs\Python\Python312\python.exe`.
