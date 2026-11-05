// Class IDs and parameter IDs. The FUIDs were generated once (uuid4) on
// 2026-11-05 and must never change: Cubase identifies the plug-in by them.
#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace midiagent {

static const Steinberg::FUID kProcessorUID (0xF5E6DED7, 0x510A47B3, 0x9D80BABE, 0x9D6D9A4A);
static const Steinberg::FUID kControllerUID (0xC53EF04F, 0x38224E32, 0xAB3C9DA9, 0x4854C9D5);

#define MIDIAGENT_VST3_CATEGORY "Instrument|Synth"

enum ParamId : Steinberg::Vst::ParamID
{
    kParamGain = 100,     // 0..1, the built-in synth output level
    kParamSynthOn = 101,  // 0/1: render the built-in synth to the audio output
    kParamMidiOut = 102,  // 0/1: send the notes to the MIDI output bus
    kParamPreview = 103,  // 0/1: internal loop of the selected segment while the transport is stopped
};

// Serialized state version, bump when the layout of getState changes.
constexpr Steinberg::int32 kStateVersion = 1;

} // namespace midiagent
