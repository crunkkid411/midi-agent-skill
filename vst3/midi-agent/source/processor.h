// The audio processor: plays the composition in sync with the host transport,
// renders the built-in synth to the audio output and sends every part to its
// own MIDI output bus (core/part_buses.h). Two copies of the song arrive from
// the controller as JSON: the clean notes (synth, saved state) and the copy
// mapped for the real instruments (keyswitches, drum lanes) for MIDI out.
// It never runs its own clock: LOOP only stops notes past the loop end while
// the controller moves the host playhead back.
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
    void installComposition (const std::string& json, const std::string& mappedJson);
    void swapPendingComposition ();
    void handleParameterChanges (Steinberg::Vst::ProcessData& data);
    void sendTransport ();

    // The audio thread reads these; the main thread writes them under pendingMutex_.
    std::mutex pendingMutex_;
    std::shared_ptr<const Composition> pending_;
    std::shared_ptr<const Composition> pendingMapped_;
    std::shared_ptr<const Composition> retired_; // freed on the main thread
    std::shared_ptr<const Composition> retiredMapped_;
    std::atomic<bool> hasPending_ {false};
    std::shared_ptr<const Composition> current_;   // audio thread only
    std::shared_ptr<const Composition> mapped_;    // audio thread only
    std::vector<int> trackBus_;                    // audio thread, per track
    std::string stateJson_;                        // main thread: what getState writes
    std::vector<Timbre> timbres_;                  // audio thread, per track
    std::vector<float> pans_;

    Sequencer sequencer_;   // clean notes -> built-in synth
    Sequencer midiSeq_;     // mapped notes -> MIDI out
    Synth synth_;
    std::vector<SeqEvent> events_;
    std::vector<SeqEvent> midiEvents_;

    std::atomic<float> gain_ {0.7f};
    std::atomic<bool> synthOn_ {true};
    std::atomic<bool> midiOut_ {true};
    std::atomic<bool> loop_ {false};
    std::atomic<bool> panic_ {false};
    std::atomic<int64_t> loopStart_ {0};
    std::atomic<int64_t> loopEnd_ {0};
    // audition: one note at a time, started from the UI thread
    std::atomic<int> auditionPending_ {0}; // 0 none, 1 new note waiting
    std::atomic<int> auditionTrack_ {0}, auditionPitch_ {60}, auditionMidiPitch_ {60};
    std::atomic<float> auditionVelocity_ {0.8f};
    int auditionHeldTrack_ {-1}, auditionHeldPitch_ {0}, auditionHeldMidiPitch_ {0}, auditionHeldBus_ {0};
    int64_t auditionSamplesLeft_ {0};
    std::atomic<int> muteChanges_ {0};
    std::mutex muteMutex_;
    std::vector<std::pair<int, int>> muteQueue_; // (track, mute) ; solo uses track + 10000

    double sampleRate_ {44100.0};
    double lastTick_ {-1.0};      // host tick we expect next block to start at
    bool wasPlaying_ {false};

    // what the controller polls
    std::atomic<double> reportPpq_ {0.0};
    std::atomic<double> reportTempo_ {120.0};
    std::atomic<bool> reportPlaying_ {false};
    std::atomic<int> reportVoices_ {0};
};

} // namespace midiagent
