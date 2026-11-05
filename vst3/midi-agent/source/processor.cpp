#include "source/processor.h"
#include "core/midifile.h"
#include "source/messages.h"
#include "source/plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace midiagent {

using namespace Steinberg;
using namespace Steinberg::Vst;

MidiAgentProcessor::MidiAgentProcessor ()
{
    setControllerClass (kControllerUID);
    events_.reserve (1024);
}

tresult PLUGIN_API MidiAgentProcessor::initialize (FUnknown* context)
{
    tresult result = AudioEffect::initialize (context);
    if (result != kResultTrue)
        return result;
    addAudioOutput (STR16 ("Stereo Out"), SpeakerArr::kStereo);
    addEventInput (STR16 ("MIDI In"), 16);
    addEventOutput (STR16 ("MIDI Out"), 16);
    return kResultTrue;
}

tresult PLUGIN_API MidiAgentProcessor::setBusArrangements (SpeakerArrangement* inputs, int32 numIns,
                                                          SpeakerArrangement* outputs, int32 numOuts)
{
    if (numIns == 0 && numOuts == 1 && outputs[0] == SpeakerArr::kStereo)
        return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
    return kResultFalse;
}

tresult PLUGIN_API MidiAgentProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
    return symbolicSampleSize == kSample32 ? kResultTrue : kResultFalse;
}

tresult PLUGIN_API MidiAgentProcessor::setupProcessing (ProcessSetup& setup)
{
    sampleRate_ = setup.sampleRate;
    synth_.setSampleRate (sampleRate_);
    return AudioEffect::setupProcessing (setup);
}

tresult PLUGIN_API MidiAgentProcessor::setActive (TBool state)
{
    if (state)
    {
        synth_.reset ();
        lastTick_ = -1.0;
        wasPlaying_ = false;
    }
    return AudioEffect::setActive (state);
}

tresult PLUGIN_API MidiAgentProcessor::setProcessing (TBool state)
{
    if (!state)
        panic_.store (true); // release every note when the host stops processing
    return kResultTrue;
}

uint32 PLUGIN_API MidiAgentProcessor::getProcessContextRequirements ()
{
    return IProcessContextRequirements::kNeedProjectTimeMusic | IProcessContextRequirements::kNeedTempo |
           IProcessContextRequirements::kNeedTransportState;
}

void MidiAgentProcessor::installComposition (const std::string& json)
{
    Composition comp;
    std::string err;
    if (!Composition::fromString (json, comp, err))
        return;
    auto fresh = std::make_shared<const Composition> (std::move (comp));
    std::lock_guard<std::mutex> lock (pendingMutex_);
    stateJson_ = json;
    retired_.reset (); // the previous composition the audio thread handed back
    pending_ = fresh;
    hasPending_.store (true, std::memory_order_release);
}

void MidiAgentProcessor::swapPendingComposition ()
{
    if (!hasPending_.load (std::memory_order_acquire))
        return;
    std::unique_lock<std::mutex> lock (pendingMutex_, std::try_to_lock);
    if (!lock.owns_lock ())
        return; // try again next block, never block the audio thread
    hasPending_.store (false, std::memory_order_release);
    // mute flags travel inside the JSON (Track::mute); the editor re-sends solo after a push
    retired_ = std::move (current_);
    current_ = std::move (pending_);
    if (retired_ && sequencer_.anyHeld ())
    {
        events_.clear ();
        sequencer_.flush (0, events_);
        synth_.allNotesOff ();
    }
    sequencer_.setComposition (current_);
    timbres_.clear ();
    pans_.clear ();
    if (current_)
        for (const Track& t : current_->tracks)
        {
            timbres_.push_back (timbreForTrack (t));
            pans_.push_back (t.hasPan ? static_cast<float> (t.pan) : 0.0f);
        }
}

void MidiAgentProcessor::handleParameterChanges (ProcessData& data)
{
    if (!data.inputParameterChanges)
        return;
    int32 n = data.inputParameterChanges->getParameterCount ();
    for (int32 i = 0; i < n; ++i)
    {
        IParamValueQueue* q = data.inputParameterChanges->getParameterData (i);
        if (!q)
            continue;
        int32 count = q->getPointCount ();
        if (count <= 0)
            continue;
        ParamValue value;
        int32 sampleOffset;
        if (q->getPoint (count - 1, sampleOffset, value) != kResultTrue)
            continue;
        switch (q->getParameterId ())
        {
            case kParamGain: gain_.store (static_cast<float> (value)); break;
            case kParamSynthOn: synthOn_.store (value > 0.5); break;
            case kParamMidiOut: midiOut_.store (value > 0.5); break;
            case kParamPreview: preview_.store (value > 0.5); break;
            default: break;
        }
    }
}

tresult PLUGIN_API MidiAgentProcessor::process (ProcessData& data)
{
    handleParameterChanges (data);
    swapPendingComposition ();
    if (panic_.exchange (false))
    {
        events_.clear ();
        sequencer_.flush (0, events_);
        events_.clear ();
        synth_.allNotesOff ();
    }

    if (muteChanges_.load (std::memory_order_acquire) > 0)
    {
        std::unique_lock<std::mutex> lock (muteMutex_, std::try_to_lock);
        if (lock.owns_lock ())
        {
            for (const auto& m : muteQueue_)
            {
                if (m.first >= 10000)
                    sequencer_.setTrackSolo (static_cast<size_t> (m.first - 10000), m.second != 0);
                else
                    sequencer_.setTrackMute (static_cast<size_t> (m.first), m.second != 0);
            }
            muteQueue_.clear ();
            muteChanges_.store (0, std::memory_order_release);
        }
    }

    const int32 numSamples = data.numSamples;
    events_.clear ();

    // --- incoming MIDI from the host: play the built-in synth live ---------
    if (data.inputEvents)
    {
        int32 count = data.inputEvents->getEventCount ();
        for (int32 i = 0; i < count; ++i)
        {
            Event e;
            if (data.inputEvents->getEvent (i, e) != kResultTrue)
                continue;
            if (e.type == Event::kNoteOnEvent)
                events_.push_back ({e.sampleOffset, 1000 + e.noteOn.channel, e.noteOn.pitch, e.noteOn.velocity, true});
            else if (e.type == Event::kNoteOffEvent)
                events_.push_back ({e.sampleOffset, 1000 + e.noteOff.channel, e.noteOff.pitch, 0.0f, false});
        }
    }

    // --- the sequencer: host clock or the preview loop ----------------------
    const ProcessContext* ctx = data.processContext;
    const bool hostPlaying = ctx && (ctx->state & ProcessContext::kPlaying) &&
                             (ctx->state & ProcessContext::kProjectTimeMusicValid);
    double tempo = (ctx && (ctx->state & ProcessContext::kTempoValid)) ? ctx->tempo : (current_ ? current_->bpm : 120.0);
    if (tempo < 1.0)
        tempo = 120.0;
    const double ticksPerSample = tempo * kPPQ / 60.0 / sampleRate_;
    const double samplesPerTick = 1.0 / ticksPerSample;
    const double blockTicks = numSamples * ticksPerSample;
    bool previewing = false;

    if (hostPlaying && numSamples > 0)
    {
        double fromTick = ctx->projectTimeMusic * kPPQ;
        if (!wasPlaying_ || lastTick_ < 0.0 || std::fabs (fromTick - lastTick_) > kPPQ / 8.0)
            sequencer_.flush (0, events_); // start or jump: release everything
        int64_t a = static_cast<int64_t> (std::ceil (fromTick - 1e-6));
        int64_t b = static_cast<int64_t> (std::ceil (fromTick + blockTicks - 1e-6));
        if (b <= a)
            b = a + 1;
        sequencer_.collect (a, b, samplesPerTick, 0, events_);
        lastTick_ = fromTick + blockTicks;
        reportPpq_.store (ctx->projectTimeMusic);
    }
    else
    {
        if (wasPlaying_)
            sequencer_.flush (0, events_);
        lastTick_ = -1.0;
        int64_t ps = previewStart_.load (), pe = previewEnd_.load ();
        if (preview_.load () && current_ && pe > ps && numSamples > 0)
        {
            previewing = true;
            if (!wasPreviewing_ || previewTick_ < ps || previewTick_ >= pe)
                previewTick_ = static_cast<double> (ps);
            double remaining = blockTicks;
            int offset = 0;
            while (remaining > 0.0)
            {
                double chunk = std::min (remaining, static_cast<double> (pe) - previewTick_);
                int64_t a = static_cast<int64_t> (std::ceil (previewTick_ - 1e-6));
                int64_t b = static_cast<int64_t> (std::ceil (previewTick_ + chunk - 1e-6));
                if (b > a)
                    sequencer_.collect (a, std::min (b, pe), samplesPerTick, offset, events_);
                previewTick_ += chunk;
                remaining -= chunk;
                offset += static_cast<int> (chunk * samplesPerTick);
                if (previewTick_ >= pe - 1e-6)
                {
                    sequencer_.flush (std::min (numSamples - 1, offset), events_);
                    previewTick_ = static_cast<double> (ps);
                }
            }
            reportPpq_.store (previewTick_ / kPPQ);
        }
        else if (wasPreviewing_)
            sequencer_.flush (0, events_);
    }
    wasPlaying_ = hostPlaying;
    wasPreviewing_ = previewing;
    reportPlaying_.store (hostPlaying);
    reportPreviewing_.store (previewing);
    reportTempo_.store (tempo);

    std::stable_sort (events_.begin (), events_.end (), [] (const SeqEvent& x, const SeqEvent& y) {
        return x.sampleOffset != y.sampleOffset ? x.sampleOffset < y.sampleOffset : (!x.noteOn && y.noteOn);
    });

    // --- MIDI out -------------------------------------------------------------
    if (midiOut_.load () && data.outputEvents)
    {
        for (const SeqEvent& e : events_)
        {
            if (e.track >= 1000)
                continue; // live input is not echoed
            Event out {};
            out.busIndex = 0;
            out.sampleOffset = std::max (0, std::min (numSamples - 1, e.sampleOffset));
            out.flags = Event::kIsLive;
            bool drum = current_ && static_cast<size_t> (e.track) < current_->tracks.size () &&
                        current_->tracks[static_cast<size_t> (e.track)].isDrum;
            int16 channel = static_cast<int16> (midiChannelForTrack (e.track, drum));
            if (e.noteOn)
            {
                out.type = Event::kNoteOnEvent;
                out.noteOn.channel = channel;
                out.noteOn.pitch = static_cast<int16> (e.pitch);
                out.noteOn.velocity = e.velocity;
                out.noteOn.length = 0;
                out.noteOn.noteId = -1;
                out.noteOn.tuning = 0.0f;
            }
            else
            {
                out.type = Event::kNoteOffEvent;
                out.noteOff.channel = channel;
                out.noteOff.pitch = static_cast<int16> (e.pitch);
                out.noteOff.velocity = 0.0f;
                out.noteOff.noteId = -1;
                out.noteOff.tuning = 0.0f;
            }
            data.outputEvents->addEvent (out);
        }
    }

    // --- audio ----------------------------------------------------------------
    if (data.numOutputs < 1 || numSamples <= 0)
        return kResultTrue;
    float* left = data.outputs[0].channelBuffers32[0];
    float* right = data.outputs[0].numChannels > 1 ? data.outputs[0].channelBuffers32[1] : nullptr;
    std::memset (left, 0, sizeof (float) * static_cast<size_t> (numSamples));
    if (right)
        std::memset (right, 0, sizeof (float) * static_cast<size_t> (numSamples));
    float scratchR[4096];
    float* r = right ? right : scratchR;
    if (!right)
        std::memset (scratchR, 0, sizeof (scratchR));

    const bool synthOn = synthOn_.load ();
    int pos = 0;
    for (const SeqEvent& e : events_)
    {
        int at = std::max (pos, std::min (numSamples, e.sampleOffset));
        if (synthOn && at > pos && at - pos <= 4096)
            synth_.render (left + pos, r + pos, at - pos);
        else if (synthOn && at > pos)
        {
            // very large blocks: render in slices so the mono scratch buffer fits
            int p = pos;
            while (p < at)
            {
                int n = std::min (4096, at - p);
                synth_.render (left + p, right ? right + p : scratchR, n);
                p += n;
            }
        }
        pos = at;
        if (e.track >= 1000)
        {
            Timbre tb = (e.track - 1000) == 9 ? Timbre::Drums : Timbre::Saw;
            if (e.noteOn)
                synth_.noteOn (e.track, e.pitch, e.velocity, tb, 0.0f);
            else
                synth_.noteOff (e.track, e.pitch);
            continue;
        }
        size_t ti = static_cast<size_t> (e.track);
        if (e.noteOn)
            synth_.noteOn (e.track, e.pitch, e.velocity, ti < timbres_.size () ? timbres_[ti] : Timbre::Saw,
                           ti < pans_.size () ? pans_[ti] : 0.0f);
        else
            synth_.noteOff (e.track, e.pitch);
    }
    if (synthOn && pos < numSamples)
    {
        int p = pos;
        while (p < numSamples)
        {
            int n = std::min (4096, numSamples - p);
            synth_.render (left + p, right ? right + p : scratchR, n);
            p += n;
        }
    }
    if (!synthOn)
        synth_.allNotesOff ();
    const float g = gain_.load ();
    for (int32 i = 0; i < numSamples; ++i)
    {
        left[i] = std::tanh (left[i] * g * 1.5f);
        if (right)
            right[i] = std::tanh (right[i] * g * 1.5f);
    }
    data.outputs[0].silenceFlags = synth_.isSilent () ? 3 : 0;
    reportVoices_.store (synth_.activeVoices ());
    return kResultTrue;
}

tresult PLUGIN_API MidiAgentProcessor::setState (IBStream* state)
{
    IBStreamer streamer (state, kLittleEndian);
    int32 version = 0;
    if (!streamer.readInt32 (version) || version < 1 || version > kStateVersion)
        return kResultFalse;
    double gain = 0.7;
    int32 synthOn = 1, midiOut = 1, len = 0;
    if (!streamer.readDouble (gain) || !streamer.readInt32 (synthOn) || !streamer.readInt32 (midiOut) ||
        !streamer.readInt32 (len))
        return kResultFalse;
    gain_.store (static_cast<float> (gain));
    synthOn_.store (synthOn != 0);
    midiOut_.store (midiOut != 0);
    std::string json;
    if (len > 0 && len < (64 << 20))
    {
        json.resize (static_cast<size_t> (len));
        if (streamer.readRaw (&json[0], len) != len)
            return kResultFalse;
        installComposition (json);
    }
    return kResultTrue;
}

tresult PLUGIN_API MidiAgentProcessor::getState (IBStream* state)
{
    IBStreamer streamer (state, kLittleEndian);
    streamer.writeInt32 (kStateVersion);
    streamer.writeDouble (gain_.load ());
    streamer.writeInt32 (synthOn_.load () ? 1 : 0);
    streamer.writeInt32 (midiOut_.load () ? 1 : 0);
    std::string json;
    {
        std::lock_guard<std::mutex> lock (pendingMutex_);
        json = stateJson_;
    }
    streamer.writeInt32 (static_cast<int32> (json.size ()));
    if (!json.empty ())
        streamer.writeRaw (json.data (), static_cast<int32> (json.size ()));
    return kResultTrue;
}

void MidiAgentProcessor::sendTransport ()
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    msg->setMessageID (kMsgTransport);
    IAttributeList* a = msg->getAttributes ();
    a->setFloat (kAttrPpq, reportPpq_.load ());
    a->setInt (kAttrPlaying, reportPlaying_.load () ? 1 : 0);
    a->setFloat (kAttrTempo, reportTempo_.load ());
    a->setInt (kAttrVoices, reportVoices_.load ());
    a->setInt (kAttrPreviewing, reportPreviewing_.load () ? 1 : 0);
    sendMessage (msg);
    msg->release ();
}

tresult PLUGIN_API MidiAgentProcessor::notify (IMessage* message)
{
    if (!message)
        return kInvalidArgument;
    FIDString id = message->getMessageID ();
    IAttributeList* a = message->getAttributes ();
    if (!id || !a)
        return kResultFalse;
    if (std::strcmp (id, kMsgComposition) == 0)
    {
        const void* data = nullptr;
        uint32 size = 0;
        if (a->getBinary (kAttrJson, data, size) == kResultTrue && data && size > 0)
            installComposition (std::string (static_cast<const char*> (data), size));
        return kResultTrue;
    }
    if (std::strcmp (id, kMsgMute) == 0 || std::strcmp (id, kMsgSolo) == 0)
    {
        int64 track = 0, value = 0;
        a->getInt (kAttrTrack, track);
        a->getInt (kAttrValue, value);
        std::lock_guard<std::mutex> lock (muteMutex_);
        muteQueue_.emplace_back (static_cast<int> (track) + (std::strcmp (id, kMsgSolo) == 0 ? 10000 : 0),
                                 static_cast<int> (value));
        muteChanges_.store (static_cast<int> (muteQueue_.size ()), std::memory_order_release);
        return kResultTrue;
    }
    if (std::strcmp (id, kMsgPreview) == 0)
    {
        int64 on = 0, start = 0, end = 0;
        a->getInt (kAttrValue, on);
        a->getInt (kAttrStartTick, start);
        a->getInt (kAttrEndTick, end);
        previewStart_.store (start);
        previewEnd_.store (end);
        preview_.store (on != 0);
        return kResultTrue;
    }
    if (std::strcmp (id, kMsgPanic) == 0)
    {
        preview_.store (false);
        panic_.store (true);
        return kResultTrue;
    }
    if (std::strcmp (id, kMsgPoll) == 0)
    {
        sendTransport ();
        return kResultTrue;
    }
    return AudioEffect::notify (message);
}

} // namespace midiagent
