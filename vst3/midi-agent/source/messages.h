// IMessage IDs and attribute names shared by the processor and controller.
//
// Controller -> processor
//   kMsgComposition   binary kAttrJson: the composition JSON (UTF-8), binary
//                     kAttrMappedJson: the same song mapped for the real instruments
//   kMsgMute          int kAttrTrack, int kAttrValue (0/1)
//   kMsgSolo          int kAttrTrack, int kAttrValue (0/1)
//   kMsgLoop          int kAttrValue (0/1), int kAttrStartTick, int kAttrEndTick
//   kMsgAudition      int kAttrTrack, int kAttrPitch (synth), int kAttrMidiPitch (MIDI out),
//                     float kAttrVelocity (0..1): one short note
//   kMsgPoll          no attributes: asks for a kMsgTransport reply
//   kMsgPanic         no attributes: all notes off
// Processor -> controller
//   kMsgTransport     float kAttrPpq, int kAttrPlaying, float kAttrTempo, int kAttrVoices
#pragma once

namespace midiagent {

constexpr const char* kMsgComposition = "MidiAgent.Composition";
constexpr const char* kMsgMute = "MidiAgent.Mute";
constexpr const char* kMsgSolo = "MidiAgent.Solo";
constexpr const char* kMsgLoop = "MidiAgent.Loop";
constexpr const char* kMsgAudition = "MidiAgent.Audition";
constexpr const char* kMsgPoll = "MidiAgent.Poll";
constexpr const char* kMsgPanic = "MidiAgent.Panic";
constexpr const char* kMsgTransport = "MidiAgent.Transport";

constexpr const char* kAttrJson = "json";
constexpr const char* kAttrMappedJson = "mappedJson";
constexpr const char* kAttrPitch = "pitch";
constexpr const char* kAttrMidiPitch = "midiPitch";
constexpr const char* kAttrVelocity = "velocity";
constexpr const char* kAttrTrack = "track";
constexpr const char* kAttrValue = "value";
constexpr const char* kAttrStartTick = "startTick";
constexpr const char* kAttrEndTick = "endTick";
constexpr const char* kAttrPpq = "ppq";
constexpr const char* kAttrPlaying = "playing";
constexpr const char* kAttrTempo = "tempo";
constexpr const char* kAttrVoices = "voices";

} // namespace midiagent
