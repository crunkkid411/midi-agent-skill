// Headless host smoke test: loads the built .vst3 with the SDK hosting
// classes, pushes a composition into the processor, runs the process loop
// with a playing transport and checks that MIDI events and audio come out.
//
//   midiagent_host_smoke <path to MIDI Agent.vst3> <composition.json>
#include "core/composition.h"
#include "core/platform.h"
#include "source/messages.h"

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/processdata.h"

#include <cmath>
#include <cstdio>
#include <string>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace {
int failures = 0;
void check (bool ok, const char* what)
{
    std::printf ("%s %s\n", ok ? "[ OK ]" : "[FAIL]", what);
    if (!ok)
        ++failures;
}

bool sendJson (IComponent* component, const std::string& json)
{
    FUnknownPtr<IConnectionPoint> cp (component);
    if (!cp)
        return false;
    auto* msg = new HostMessage ();
    msg->setMessageID (midiagent::kMsgComposition);
    msg->getAttributes ()->setBinary (midiagent::kAttrJson, json.data (), static_cast<uint32> (json.size ()));
    tresult r = cp->notify (msg);
    msg->release ();
    return r == kResultTrue;
}

bool sendPreview (IComponent* component, bool on, int64 start, int64 end)
{
    FUnknownPtr<IConnectionPoint> cp (component);
    auto* msg = new HostMessage ();
    msg->setMessageID (midiagent::kMsgPreview);
    msg->getAttributes ()->setInt (midiagent::kAttrValue, on ? 1 : 0);
    msg->getAttributes ()->setInt (midiagent::kAttrStartTick, start);
    msg->getAttributes ()->setInt (midiagent::kAttrEndTick, end);
    tresult r = cp->notify (msg);
    msg->release ();
    return r == kResultTrue;
}
} // namespace

int main (int argc, char** argv)
{
    if (argc < 3)
    {
        std::fprintf (stderr, "usage: midiagent_host_smoke <plugin.vst3> <composition.json>\n");
        return 2;
    }
    std::string error;
    auto module = VST3::Hosting::Module::create (argv[1], error);
    check (module != nullptr, "module loads");
    if (!module)
    {
        std::fprintf (stderr, "%s\n", error.c_str ());
        return 1;
    }
    std::string json;
    check (midiagent::platform::readFile (argv[2], json, error), "composition file reads");

    auto* host = new HostApplication ();
    PluginContextFactory::instance ().setPluginContext (static_cast<FUnknown*> (static_cast<IHostApplication*> (host)));

    const auto& factory = module->getFactory ();
    bool found = false;
    for (const auto& info : factory.classInfos ())
    {
        if (info.category () != kVstAudioEffectClass)
            continue;
        found = true;
        std::printf ("class: %s (%s)\n", info.name ().c_str (), info.subCategoriesString ().c_str ());
        PlugProvider provider (factory, info, true);
        check (provider.initialize (), "plug provider initializes (component + controller connected)");
        IComponent* component = provider.getComponent ();
        IEditController* controller = provider.getController ();
        check (component != nullptr, "component created");
        check (controller != nullptr, "controller created");
        if (!component || !controller)
            break;

        FUnknownPtr<IAudioProcessor> processor (component);
        check (processor != nullptr, "component is an audio processor");

        ProcessSetup setup {kRealtime, kSample32, 512, 44100.0};
        check (processor->setupProcessing (setup) == kResultTrue, "setupProcessing");
        component->activateBus (kAudio, kOutput, 0, true);
        component->activateBus (kEvent, kInput, 0, true);
        component->activateBus (kEvent, kOutput, 0, true);
        check (component->setActive (true) == kResultTrue, "setActive");
        check (processor->setProcessing (true) == kResultTrue, "setProcessing");

        check (sendJson (component, json), "composition message accepted");

        HostProcessData data;
        check (data.prepare (*component, 512, kSample32), "process data prepared");
        EventList inEvents (64), outEvents (4096);
        ParameterChanges inParams, outParams;
        data.inputEvents = &inEvents;
        data.outputEvents = &outEvents;
        data.inputParameterChanges = &inParams;
        data.outputParameterChanges = &outParams;
        data.numSamples = 512;
        ProcessContext ctx {};
        ctx.state = ProcessContext::kPlaying | ProcessContext::kTempoValid | ProcessContext::kProjectTimeMusicValid;
        ctx.sampleRate = 44100.0;
        ctx.tempo = 147.0;
        ctx.projectTimeMusic = 0.0;
        data.processContext = &ctx;

        midiagent::Composition comp;
        midiagent::Composition::fromString (json, comp, error);
        const double qpBlock = 512.0 * ctx.tempo / 60.0 / ctx.sampleRate;
        const double totalQuarters = 8.0; // two bars of 4/4
        int blocks = static_cast<int> (std::ceil (totalQuarters / qpBlock));
        int noteOns = 0, noteOffs = 0;
        double energy = 0.0;
        bool firstEventOffsetOk = true;
        for (int b = 0; b < blocks; ++b)
        {
            outEvents.clear ();
            check (processor->process (data) == kResultTrue || b > 0, "process returns ok");
            for (int32 i = 0; i < outEvents.getEventCount (); ++i)
            {
                Event e;
                outEvents.getEvent (i, e);
                if (e.sampleOffset < 0 || e.sampleOffset >= 512)
                    firstEventOffsetOk = false;
                if (e.type == Event::kNoteOnEvent)
                    ++noteOns;
                else if (e.type == Event::kNoteOffEvent)
                    ++noteOffs;
            }
            float* l = data.outputs[0].channelBuffers32[0];
            for (int i = 0; i < 512; ++i)
                energy += static_cast<double> (l[i]) * l[i];
            ctx.projectTimeMusic += qpBlock;
            ctx.projectTimeSamples += 512;
        }
        int expectedOns = 0;
        const int64_t coveredTicks = static_cast<int64_t> (std::ceil (blocks * qpBlock * midiagent::kPPQ - 1e-6));
        for (const auto& t : comp.tracks)
            for (const auto& n : t.notes)
                if (n.start < coveredTicks)
                    ++expectedOns;
        std::printf ("note ons %d (expected about %d), note offs %d, rms %.4f\n", noteOns, expectedOns, noteOffs,
                     std::sqrt (energy / (blocks * 512.0)));
        check (noteOns == expectedOns, "every note of the played range was sent to the MIDI output");
        check (firstEventOffsetOk, "event sample offsets are inside the block");
        check (energy > 1.0, "the built-in synth produced audio while playing");

        // stop: every held note is released
        ctx.state = ProcessContext::kTempoValid | ProcessContext::kProjectTimeMusicValid;
        outEvents.clear ();
        processor->process (data);
        for (int32 i = 0; i < outEvents.getEventCount (); ++i)
        {
            Event e;
            outEvents.getEvent (i, e);
            if (e.type == Event::kNoteOffEvent)
                ++noteOffs;
        }
        check (noteOffs == noteOns, "note offs match note ons after stop");

        // preview loop while stopped: the internal clock plays the first bar
        check (sendPreview (component, true, 0, 4 * midiagent::kPPQ), "preview message accepted");
        int previewOns = 0;
        for (int b = 0; b < blocks; ++b)
        {
            outEvents.clear ();
            processor->process (data);
            for (int32 i = 0; i < outEvents.getEventCount (); ++i)
            {
                Event e;
                outEvents.getEvent (i, e);
                if (e.type == Event::kNoteOnEvent)
                    ++previewOns;
            }
        }
        std::printf ("preview note ons over two loops: %d\n", previewOns);
        check (previewOns > 0, "preview loop plays notes while the host is stopped");
        sendPreview (component, false, 0, 0);
        outEvents.clear ();
        processor->process (data);

        // state round trip
        MemoryStream stream;
        check (component->getState (&stream) == kResultTrue, "getState");
        int64 pos = 0;
        stream.seek (0, IBStream::kIBSeekSet, &pos);
        check (component->setState (&stream) == kResultTrue, "setState");
        stream.seek (0, IBStream::kIBSeekSet, &pos);
        check (controller->setComponentState (&stream) == kResultTrue, "controller setComponentState");
        check (stream.getSize () > static_cast<int64> (json.size ()), "state holds the composition JSON");

        processor->setProcessing (false);
        component->setActive (false);
        data.unprepare ();
        break;
    }
    check (found, "an audio effect class exists in the module");
    std::printf ("%s: %d failures\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
