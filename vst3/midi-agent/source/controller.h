// The edit controller: owns the Session the editor shows, talks to the
// processor by IMessage, runs the Python bridge and offline renders on a
// worker thread, and saves the bridge settings in its own state.
#pragma once

#include "source/session.h"

#include "public.sdk/source/vst/vsteditcontroller.h"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace midiagent {

class MidiAgentEditor;

class MidiAgentController : public Steinberg::Vst::EditController
{
public:
    MidiAgentController () = default;
    ~MidiAgentController () override;
    static Steinberg::FUnknown* createInstance (void*) { return static_cast<Steinberg::Vst::IEditController*> (new MidiAgentController ()); }

    Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API notify (Steinberg::Vst::IMessage* message) SMTG_OVERRIDE;
    void editorDestroyed (Steinberg::Vst::EditorView* editor) SMTG_OVERRIDE;

    // --- for the editor (UI thread) ----------------------------------------
    Session& session () { return session_; }

    /// Replace the composition (after generate / open / new) and push it.
    void setComposition (Composition comp, const std::string& path, const std::string& why);
    /// Send the current composition to the processor.
    void pushComposition ();
    void sendMute (int track, bool mute);
    void sendSolo (int track, bool solo);
    void sendPreview (bool on);
    void sendPoll ();
    void sendPanic ();
    void setParam (Steinberg::Vst::ParamID id, double normalized);

    /// Run `work` on the worker thread; it returns a completion the editor
    /// runs on the UI thread from pumpJobs (). One job at a time.
    bool startJob (const std::string& label, std::function<std::function<void ()> ()> work);
    void pumpJobs ();

    /// A path for files the plug-in writes on its own (drag files, auto saves).
    std::string workDir () const;

private:
    void applyDefaults ();

    Session session_;
    MidiAgentEditor* editor_ {nullptr};

    std::thread worker_;
    std::mutex jobMutex_;
    std::deque<std::function<void ()>> completions_;
    std::atomic<bool> busy_ {false};
};

} // namespace midiagent
