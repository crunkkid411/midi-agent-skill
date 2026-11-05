// General MIDI program table, mirrored from midi_types/gm_instruments.py.
#pragma once
#include <string>

namespace midiagent {

/// Program 0..127 -> the slug used in composition JSON ("acoustic-grand-piano").
const char* gmInstrumentName (int program);
/// Slug or alias ("piano") -> program 0..127. Unknown names map to 0, like the Python side.
int gmProgramForName (const std::string& name);

} // namespace midiagent
