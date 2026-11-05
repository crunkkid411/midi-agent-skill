#include "source/controller.h"
#include "core/platform.h"
#include "source/editor.h"
#include "source/messages.h"
#include "source/plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "public.sdk/source/vst/vstparameters.h"

#include <cstdlib>
#include <cstring>

namespace midiagent {

using namespace Steinberg;
using namespace Steinberg::Vst;

MidiAgentController::~MidiAgentController ()
{
    if (worker_.joinable ())
        worker_.join ();
}

void MidiAgentController::applyDefaults ()
{
    session_.agent.pythonExe = "python";
#ifndef _WIN32
    session_.agent.pythonExe = "python3";
#endif
    session_.request.genres = {"scenecore"};
    session_.request.bars = 8;
    session_.request.title = "midi-agent";
}

tresult PLUGIN_API MidiAgentController::initialize (FUnknown* context)
{
    tresult result = EditController::initialize (context);
    if (result != kResultTrue)
        return result;
    parameters.addParameter (new RangeParameter (STR16 ("Synth Level"), kParamGain, nullptr, 0.0, 1.0, 0.7, 0,
                                                 ParameterInfo::kCanAutomate));
    auto addSwitch = [this] (const TChar* title, ParamID id, bool on, int32 flags) {
        auto* p = new StringListParameter (title, id, nullptr, flags);
        p->appendString (STR16 ("Off"));
        p->appendString (STR16 ("On"));
        p->setNormalized (on ? 1.0 : 0.0);
        p->getInfo ().defaultNormalizedValue = on ? 1.0 : 0.0;
        parameters.addParameter (p);
    };
    addSwitch (STR16 ("Synth On"), kParamSynthOn, true, ParameterInfo::kCanAutomate | ParameterInfo::kIsList);
    addSwitch (STR16 ("MIDI Out"), kParamMidiOut, true, ParameterInfo::kCanAutomate | ParameterInfo::kIsList);
    addSwitch (STR16 ("Preview Loop"), kParamPreview, false, ParameterInfo::kIsList);
    applyDefaults ();
    session_.addLog ("MIDI Agent ready. Set the skill folder, then GENERATE.");

    // Environment overrides, for development hosts and for a machine wide default:
    //   MIDIAGENT_SKILL_ROOT   the midi-agent-skill folder
    //   MIDIAGENT_PYTHON       the Python executable
    //   MIDIAGENT_AUTOLOAD     a composition JSON to load on start
    if (const char* root = std::getenv ("MIDIAGENT_SKILL_ROOT"))
        if (*root)
        {
            session_.agent.skillRoot = root;
            session_.genres = listGenres (session_.agent);
            session_.addLog ("skill folder from MIDIAGENT_SKILL_ROOT: " + session_.agent.skillRoot);
        }
    if (const char* py = std::getenv ("MIDIAGENT_PYTHON"))
        if (*py)
            session_.agent.pythonExe = py;
    if (const char* file = std::getenv ("MIDIAGENT_AUTOLOAD"))
        if (*file)
        {
            Composition comp;
            std::string err;
            if (Composition::loadFile (file, comp, err))
            {
                session_.comp = std::move (comp);
                session_.compositionPath = file;
                session_.rebuildSegments ();
                session_.addLog (std::string ("loaded MIDIAGENT_AUTOLOAD: ") + file);
            }
            else
                session_.addLog ("ERROR MIDIAGENT_AUTOLOAD: " + err);
        }
    return kResultTrue;
}

tresult PLUGIN_API MidiAgentController::terminate ()
{
    if (worker_.joinable ())
        worker_.join ();
    return EditController::terminate ();
}

tresult PLUGIN_API MidiAgentController::setComponentState (IBStream* state)
{
    // the processor's state: parameters, then the composition JSON
    IBStreamer streamer (state, kLittleEndian);
    int32 version = 0;
    if (!streamer.readInt32 (version) || version < 1 || version > kStateVersion)
        return kResultFalse;
    double gain = 0.7;
    int32 synthOn = 1, midiOut = 1, len = 0;
    if (!streamer.readDouble (gain) || !streamer.readInt32 (synthOn) || !streamer.readInt32 (midiOut) ||
        !streamer.readInt32 (len))
        return kResultFalse;
    setParamNormalized (kParamGain, gain);
    setParamNormalized (kParamSynthOn, synthOn ? 1.0 : 0.0);
    setParamNormalized (kParamMidiOut, midiOut ? 1.0 : 0.0);
    if (len > 0 && len < (64 << 20))
    {
        std::string json (static_cast<size_t> (len), '\0');
        if (streamer.readRaw (&json[0], len) != len)
            return kResultFalse;
        Composition comp;
        std::string err;
        if (Composition::fromString (json, comp, err))
        {
            session_.comp = std::move (comp);
            session_.rebuildSegments ();
            session_.dirty = false;
            if (editor_)
                editor_->compositionReplaced ();
        }
        else
            session_.addLog ("ERROR project state: " + err);
    }
    return kResultTrue;
}

tresult PLUGIN_API MidiAgentController::setState (IBStream* state)
{
    // controller only state: the bridge settings as JSON
    IBStreamer streamer (state, kLittleEndian);
    int32 len = 0;
    if (!streamer.readInt32 (len) || len <= 0 || len > (1 << 20))
        return kResultFalse;
    std::string text (static_cast<size_t> (len), '\0');
    if (streamer.readRaw (&text[0], len) != len)
        return kResultFalse;
    Json j;
    std::string err;
    if (!Json::parse (text, j, err))
        return kResultFalse;
    session_.agent.pythonExe = j.get ("python").asString (session_.agent.pythonExe);
    session_.agent.skillRoot = j.get ("skill_root").asString ("");
    session_.compositionPath = j.get ("composition_path").asString ("");
    session_.request.title = j.get ("title").asString ("midi-agent");
    session_.request.bars = j.get ("bars").asInt (8);
    session_.request.bpm = j.get ("bpm").asInt (0);
    session_.request.seed = j.get ("seed").asInt (-1);
    session_.request.key = j.get ("key").asString ("");
    session_.request.useForm = j.get ("form").asBool (false);
    session_.request.genres.clear ();
    for (const Json& g : j.get ("genres").asArray ())
        session_.request.genres.push_back (g.asString ());
    session_.request.weights.clear ();
    for (const Json& w : j.get ("weights").asArray ())
        session_.request.weights.push_back (w.asNumber ());
    session_.snapTicks = j.get ("snap").asInt (kPPQ / 4);
    if (session_.agent.valid ())
        session_.genres = listGenres (session_.agent);
    return kResultTrue;
}

tresult PLUGIN_API MidiAgentController::getState (IBStream* state)
{
    Json j = Json::object ();
    j.set ("python", session_.agent.pythonExe);
    j.set ("skill_root", session_.agent.skillRoot);
    j.set ("composition_path", session_.compositionPath);
    j.set ("title", session_.request.title);
    j.set ("bars", session_.request.bars);
    j.set ("bpm", session_.request.bpm);
    j.set ("seed", session_.request.seed);
    j.set ("key", session_.request.key);
    j.set ("form", session_.request.useForm);
    Json genres = Json::array ();
    for (const auto& g : session_.request.genres)
        genres.push (g);
    j.set ("genres", genres);
    Json weights = Json::array ();
    for (double w : session_.request.weights)
        weights.push (w);
    j.set ("weights", weights);
    j.set ("snap", session_.snapTicks);
    std::string text = j.dump (0);
    IBStreamer streamer (state, kLittleEndian);
    streamer.writeInt32 (static_cast<int32> (text.size ()));
    streamer.writeRaw (text.data (), static_cast<int32> (text.size ()));
    return kResultTrue;
}

IPlugView* PLUGIN_API MidiAgentController::createView (FIDString name)
{
    if (name && std::strcmp (name, ViewType::kEditor) == 0)
    {
        editor_ = new MidiAgentEditor (this);
        return editor_;
    }
    return nullptr;
}

void MidiAgentController::editorDestroyed (EditorView* editor)
{
    if (editor == editor_)
        editor_ = nullptr;
}

tresult PLUGIN_API MidiAgentController::notify (IMessage* message)
{
    if (!message)
        return kInvalidArgument;
    FIDString id = message->getMessageID ();
    if (id && std::strcmp (id, kMsgTransport) == 0)
    {
        IAttributeList* a = message->getAttributes ();
        double ppq = 0, tempo = 120;
        int64 playing = 0, voices = 0, previewing = 0;
        a->getFloat (kAttrPpq, ppq);
        a->getInt (kAttrPlaying, playing);
        a->getFloat (kAttrTempo, tempo);
        a->getInt (kAttrVoices, voices);
        a->getInt (kAttrPreviewing, previewing);
        session_.transport.ppq = ppq;
        session_.transport.playing = playing != 0;
        session_.transport.tempo = tempo;
        session_.transport.voices = static_cast<int> (voices);
        session_.transport.previewing = previewing != 0;
        if (editor_)
            editor_->transportUpdated ();
        return kResultTrue;
    }
    return EditController::notify (message);
}

void MidiAgentController::setComposition (Composition comp, const std::string& path, const std::string& why)
{
    session_.comp = std::move (comp);
    session_.compositionPath = path;
    session_.selectedSegment = 0;
    session_.selectedTrack = 0;
    session_.rebuildSegments ();
    session_.dirty = false;
    ++session_.revision;
    session_.addLog (why);
    pushComposition ();
    if (editor_)
        editor_->compositionReplaced ();
}

void MidiAgentController::pushComposition ()
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    msg->setMessageID (kMsgComposition);
    std::string json = session_.comp.toString ();
    msg->getAttributes ()->setBinary (kAttrJson, json.data (), static_cast<uint32> (json.size ()));
    sendMessage (msg);
    msg->release ();
}

void MidiAgentController::sendMute (int track, bool mute)
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    msg->setMessageID (kMsgMute);
    msg->getAttributes ()->setInt (kAttrTrack, track);
    msg->getAttributes ()->setInt (kAttrValue, mute ? 1 : 0);
    sendMessage (msg);
    msg->release ();
}

void MidiAgentController::sendSolo (int track, bool solo)
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    msg->setMessageID (kMsgSolo);
    msg->getAttributes ()->setInt (kAttrTrack, track);
    msg->getAttributes ()->setInt (kAttrValue, solo ? 1 : 0);
    sendMessage (msg);
    msg->release ();
}

void MidiAgentController::sendPreview (bool on)
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    const Segment* seg = session_.segment ();
    msg->setMessageID (kMsgPreview);
    msg->getAttributes ()->setInt (kAttrValue, on && seg ? 1 : 0);
    msg->getAttributes ()->setInt (kAttrStartTick, seg ? seg->startTick : 0);
    msg->getAttributes ()->setInt (kAttrEndTick, seg ? seg->endTick : 0);
    sendMessage (msg);
    msg->release ();
    session_.loopPreview = on && seg;
    setParam (kParamPreview, session_.loopPreview ? 1.0 : 0.0);
}

void MidiAgentController::sendPoll ()
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    msg->setMessageID (kMsgPoll);
    sendMessage (msg);
    msg->release ();
}

void MidiAgentController::sendPanic ()
{
    IMessage* msg = allocateMessage ();
    if (!msg)
        return;
    msg->setMessageID (kMsgPanic);
    sendMessage (msg);
    msg->release ();
    session_.loopPreview = false;
}

void MidiAgentController::setParam (ParamID id, double normalized)
{
    beginEdit (id);
    setParamNormalized (id, normalized);
    performEdit (id, normalized);
    endEdit (id);
}

bool MidiAgentController::startJob (const std::string& label, std::function<std::function<void ()> ()> work)
{
    if (busy_.load ())
    {
        session_.addLog ("busy: " + session_.busyLabel + " is still running");
        return false;
    }
    if (worker_.joinable ())
        worker_.join ();
    busy_.store (true);
    session_.busy = true;
    session_.busyLabel = label;
    session_.addLog (label + "...");
    worker_ = std::thread ([this, work] () {
        std::function<void ()> done;
        try
        {
            done = work ();
        }
        catch (const std::exception& e)
        {
            std::string what = e.what ();
            done = [this, what] () { session_.addLog ("ERROR " + what); };
        }
        std::lock_guard<std::mutex> lock (jobMutex_);
        completions_.push_back (std::move (done));
        busy_.store (false);
    });
    return true;
}

void MidiAgentController::pumpJobs ()
{
    std::deque<std::function<void ()>> ready;
    {
        std::lock_guard<std::mutex> lock (jobMutex_);
        ready.swap (completions_);
    }
    for (auto& f : ready)
        if (f)
            f ();
    if (!busy_.load () && session_.busy)
    {
        session_.busy = false;
        session_.busyLabel.clear ();
        if (worker_.joinable ())
            worker_.join ();
    }
}

std::string MidiAgentController::workDir () const
{
    return platform::tempDir ();
}

} // namespace midiagent
