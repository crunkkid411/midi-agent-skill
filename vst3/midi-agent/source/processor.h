// The audio processor: plays the composition in sync with the host transport
// (or loops one segment on its own clock while the transport is stopped),
// renders the built-in synth to the audio output and the notes to the MIDI
// output bus. The composition arrives from the controller as JSON by IMessage.
#pragma once

#include "core/composition.h"
#include "core/sequencer.h"
#include "core/synth.h"

#include "public.sdk/source/vst/vstaudioeffect.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace midiagent {

class MidiAgentProcessor : public Steinberg::Vst::AudioEffect
{
public:
    MidiAgentProcessor ();
    static Steinberg::FUnknown* createInstance (void*) { return static_cast<Steinberg::Vst::IAudioProcessor*> (new MidiAgentProcessor ()); }

    Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
                                                       Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& setup) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setProcessing (Steinberg::TBool state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API notify (Steinberg::Vst::IMessage* message) SMTG_OVERRIDE;
    Steinberg::uint32 PLUGIN_API getProcessContextRequirements () SMTG_OVERRIDE;

private:
    void installComposition (const std::string& json);
    void swapPendingComposition ();
    void handleParameterChanges (Steinberg::Vst::ProcessData& data);
    void sendTransport ();

    // The audio thread reads these; the main thread writes them under pendingMutex_.
    std::mutex pendingMutex_;
    std::shared_ptr<const Composition> pending_;
    std::shared_ptr<const Composition> retired_; // freed on the main thread
    std::atomic<bool> hasPending_ {false};
    std::shared_ptr<const Composition> current_;   // audio thread only
    std::string stateJson_;                        // main thread: what getState writes
    std::vector<Timbre> timbres_;                  // audio thread, per track
    std::vector<float> pans_;

    Sequencer sequencer_;
    Synth synth_;
    std::vector<SeqEvent> events_;

    std::atomic<float> gain_ {0.7f};
    std::atomic<bool> synthOn_ {true};
    std::atomic<bool> midiOut_ {true};
    std::atomic<bool> preview_ {false};
    std::atomic<bool> panic_ {false};
    std::atomic<int64_t> previewStart_ {0};
    std::atomic<int64_t> previewEnd_ {0};
    std::atomic<int> muteChanges_ {0};
    std::mutex muteMutex_;
    std::vector<std::pair<int, int>> muteQueue_; // (track, mute) ; solo uses track + 10000

    double sampleRate_ {44100.0};
    double lastTick_ {-1.0};      // host tick we expect next block to start at
    double previewTick_ {0.0};
    bool wasPlaying_ {false};
    bool wasPreviewing_ {false};

    // what the controller polls
    std::atomic<double> reportPpq_ {0.0};
    std::atomic<double> reportTempo_ {120.0};
    std::atomic<bool> reportPlaying_ {false};
    std::atomic<bool> reportPreviewing_ {false};
    std::atomic<int> reportVoices_ {0};
};

} // namespace midiagent
