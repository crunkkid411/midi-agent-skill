// Standard MIDI File (type 1, 480 PPQ) writer. Mirrors skills/generate_midi.py
// for the General MIDI path: same channel assignment, tempo and time
// signature meta events, track names, program changes, pan and CC automation.
// Instrument maps (instruments/*.json: keyswitches, velocity bands, drum
// remaps) are NOT applied here; use the Python CLI for VST-mapped renders.
#pragma once
#include "core/composition.h"

#include <cstdint>
#include <string>
#include <vector>

namespace midiagent {

/// Channel for a track: drums -> 9, melodic tracks skip channel 9 (same as Python).
int midiChannelForTrack (int trackIndex, bool isDrum);

/// trackIndices empty = every track. Indices keep their original channel.
std::vector<uint8_t> writeMidi (const Composition& comp, const std::vector<int>& trackIndices = {});
bool writeMidiFile (const std::string& path, const Composition& comp, const std::vector<int>& trackIndices,
                    std::string& error);

} // namespace midiagent
