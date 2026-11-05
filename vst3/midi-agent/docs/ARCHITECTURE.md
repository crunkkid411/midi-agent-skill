# Architecture

## The three layers

```
 Python (midi-agent-skill)            C++ core (vst3/midi-agent/core)        VST3 plug-in (source/)
 ────────────────────────             ─────────────────────────────          ───────────────────────
 compose_from_profile.py  ──JSON──▶   composition.h  (model, 480 PPQ)  ◀──▶  controller.cpp (Session)
 genres/*.json                        segments.h     (leaf sections)          editor.cpp (views)
 instruments/*.json                   midifile.h     (SMF writer)             processor.cpp (audio thread)
                                      wavfile.h, synth.h, renderer.h
                                      sequencer.h    (ticks -> events)
                                      agent_bridge.h (argv + subprocess)
```

- **Python is the source of truth for the music.** The plug-in never ports
  the composers. It builds the command line (`agent_bridge.cpp`), runs it on
  a worker thread and reads `plans/<title>.composition.json`.
- **`core/` has no SDK dependency.** It builds and tests in seconds with
  `-DMIDIAGENT_BUILD_PLUGIN=OFF`. Put logic here, not in the views.
- **`source/` is the host glue.** It follows the layout of Steinberg's VST3
  Project Generator: `processor`, `controller`, `editor`, `factory`,
  `version.h`, `plugids.h`, `resource/win32resource.rc`.

## Data model (`core/composition.h`)

The C++ twin of `midi_types/music.py`. Notes keep integer ticks at 480 per
quarter note. On load, `start` (beats) and `duration` (a label such as `8`
or `d4`) become ticks. On save, lengths that match a label are written as
that label and other lengths as a division string (`"2.6666667"` for 1.5
beats) that `parse_duration` in Python reads back. Unknown keys on the
composition, a track or a section survive a round trip (`extra`). The only
field the plug-in adds is `"mute": true` on a track.

## Segments (`core/segments.h`)

A segment is a leaf section: a micro-section (`verse.A`) when the song has
them, else a top-level section. A song without sections gets synthetic 4 bar
segments (`sectionIndex == -1`), which can be edited and exported but not
regenerated, because `--regenerate` needs a section name.

`extractSegment` builds a stand-alone composition for export (notes shifted
to tick 0, one section, `bars` set). `replaceSegmentNotes` is how the piano
roll writes back.

## Playback (`core/sequencer.h`, `source/processor.cpp`)

`process ()` reads `ProcessContext::projectTimeMusic` and `tempo`, converts
the block to a tick range and asks the sequencer for note on/off events with
sample offsets. A position jump larger than an eighth of a beat, a stop, or a
`setProcessing (false)` releases every held note. While the transport is
stopped and `PREVIEW LOOP` is on, an internal clock loops the selected
segment's tick range (`kMsgPreview` carries `startTick` and `endTick`).

Events go two ways: to the built-in `Synth` (audio out) and to the event
output bus (`midiChannelForTrack` mirrors `_assign_channel` in
`generate_midi.py`: drums on channel 9, melodic tracks skip it).

Live MIDI from the host's event input plays the synth too (track id 1000 +
channel) and is not echoed to the output.

## Threads

| Thread | Owns | Talks through |
|---|---|---|
| Audio (processor) | `current_` composition, sequencer, synth | reads atomics; swaps `pending_` under `try_lock`; the old composition goes to `retired_` and is freed on the main thread |
| Main / UI (controller, editor) | `Session`, the views | `IMessage` to the processor (`messages.h`); a 40 ms idle timer polls transport (`kMsgPoll` -> `kMsgTransport`), drains job completions and pushes edits 120 ms after the last mouse change |
| Worker (controller) | one Python or render job at a time | `startJob` returns a completion closure that `pumpJobs` runs on the UI thread |

## State

- Processor state (`getState`): version, gain, switches, then the full
  composition JSON. The controller reads the same stream in
  `setComponentState`, so a saved Cubase project restores the song.
- Controller state: a small JSON with the Python path, skill folder,
  composition path, generate settings and snap.
- Parameters: `Synth Level` (0..1), `Synth On`, `MIDI Out`, `Preview Loop`.

## Drag and drop out

`DragChip` writes the file when the mouse moves 5 px with the button down,
then calls `CView::doDrag` with a `CDropSource` of type `kFilePath`. On
Windows VSTGUI turns that into a shell file drag (`CF_HDROP`), which Cubase
accepts on tracks. Files land in `%TEMP%\midi-agent\`.

## What is intentionally not here

- Instrument maps (`instruments/*.json`): keyswitches, velocity bands and
  drum remaps are applied by the Python renderer, not by the plug-in's MIDI
  writer. The plug-in exports General MIDI style note numbers.
- Tempo map playback: the sequencer uses the host tempo while playing. The
  offline renderer and the MIDI writer honour `tempo_map`.
- Track add / remove / rename inside the plug-in.
