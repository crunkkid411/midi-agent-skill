// The plug-in's MIDI outputs: one named event bus per part, in a FIXED order,
// so a Cubase template routes each instrument track to the same bus for every
// song. Never reorder or rename existing entries (saved projects route by
// index); add new ones at the end.
#pragma once
#include "core/composition.h"

namespace midiagent {

enum PartBus : int
{
    kBusGuitarL = 0,   // rhythm guitar, left take (or the only take)
    kBusGuitarR,       // rhythm guitar, right take
    kBusGuitarClean,   // rhythm guitar, "clean" tone sections
    kBusGuitarAcoustic,// rhythm guitar, "acoustic" tone sections
    kBusLead,          // lead
    kBusDrums,         // acoustic drums (Superior Drummer 3)
    kBusElectronicDrums,
    kBusBass,
    kBusSynthBass,
    kBusChords,
    kBusPad,
    kBusPad2,          // a second pad tone ("Pad (choir)")
    kBusMelody,
    kBusOther,
    kPartBusCount
};

const char* partBusName (int bus);
/// Which bus a track plays on, from its role and name ("Rhythm Guitar R",
/// "Rhythm Guitar (clean)", "Pad (choir)").
int partBusForTrack (const Track& track);

} // namespace midiagent
