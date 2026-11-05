// IMessage IDs and attribute names shared by the processor and controller.
//
// Controller -> processor
//   kMsgComposition   binary kAttrJson: the composition JSON (UTF-8)
//   kMsgMute          int kAttrTrack, int kAttrValue (0/1)
//   kMsgSolo          int kAttrTrack, int kAttrValue (0/1)
//   kMsgPreview       int kAttrValue (0/1), int kAttrStartTick, int kAttrEndTick
//   kMsgPoll          no attributes: asks for a kMsgTransport reply
//   kMsgPanic         no attributes: all notes off
// Processor -> controller
//   kMsgTransport     float kAttrPpq, int kAttrPlaying, float kAttrTempo, int kAttrVoices,
//                     int kAttrPreviewing
#pragma once

namespace midiagent {

constexpr const char* kMsgComposition = "MidiAgent.Composition";
constexpr const char* kMsgMute = "MidiAgent.Mute";
constexpr const char* kMsgSolo = "MidiAgent.Solo";
constexpr const char* kMsgPreview = "MidiAgent.Preview";
constexpr const char* kMsgPoll = "MidiAgent.Poll";
constexpr const char* kMsgPanic = "MidiAgent.Panic";
constexpr const char* kMsgTransport = "MidiAgent.Transport";

constexpr const char* kAttrJson = "json";
constexpr const char* kAttrTrack = "track";
constexpr const char* kAttrValue = "value";
constexpr const char* kAttrStartTick = "startTick";
constexpr const char* kAttrEndTick = "endTick";
constexpr const char* kAttrPpq = "ppq";
constexpr const char* kAttrPlaying = "playing";
constexpr const char* kAttrTempo = "tempo";
constexpr const char* kAttrVoices = "voices";
constexpr const char* kAttrPreviewing = "previewing";

} // namespace midiagent
