#include "source/controller.h"
#include "source/plugids.h"
#include "source/processor.h"
#include "source/version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "MIDI Agent"

BEGIN_FACTORY_DEF (stringCompanyName, stringCompanyWeb, stringCompanyEmail)

DEF_CLASS2 (INLINE_UID_FROM_FUID (midiagent::kProcessorUID),
            PClassInfo::kManyInstances,
            kVstAudioEffectClass,
            stringPluginName,
            Vst::kDistributable,
            MIDIAGENT_VST3_CATEGORY,
            FULL_VERSION_STR,
            kVstVersionString,
            midiagent::MidiAgentProcessor::createInstance)

DEF_CLASS2 (INLINE_UID_FROM_FUID (midiagent::kControllerUID),
            PClassInfo::kManyInstances,
            kVstComponentControllerClass,
            stringPluginName "Controller",
            0,
            "",
            FULL_VERSION_STR,
            kVstVersionString,
            midiagent::MidiAgentController::createInstance)

END_FACTORY
