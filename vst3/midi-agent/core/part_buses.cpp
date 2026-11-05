#include "core/part_buses.h"

#include <algorithm>
#include <cctype>

namespace midiagent {

const char* partBusName (int bus)
{
    static const char* names[kPartBusCount] = {
        "Guitar L", "Guitar R", "Guitar Clean", "Guitar Acoustic", "Lead", "Drums", "Electronic Drums",
        "Bass", "Synth Bass", "Chords", "Pad", "Pad 2", "Melody", "Other"};
    return bus >= 0 && bus < kPartBusCount ? names[bus] : "Other";
}

int partBusForTrack (const Track& t)
{
    std::string name = t.name;
    std::transform (name.begin (), name.end (), name.begin (), [] (unsigned char c) { return static_cast<char> (std::tolower (c)); });
    auto has = [&] (const char* s) { return name.find (s) != std::string::npos; };
    auto endsWith = [&] (const char* s) {
        std::string x (s);
        return name.size () >= x.size () && name.compare (name.size () - x.size (), x.size (), x) == 0;
    };
    const std::string& role = t.role;
    if (role == "rhythm_guitar" || has ("rhythm guitar"))
    {
        if (has ("(clean)"))
            return kBusGuitarClean;
        if (has ("(acoustic)"))
            return kBusGuitarAcoustic;
        if (endsWith (" r"))
            return kBusGuitarR;
        return kBusGuitarL;
    }
    if (role == "lead")
        return kBusLead;
    if (role == "electronic_drums")
        return kBusElectronicDrums;
    if (role == "drums" || t.isDrum)
        return kBusDrums;
    if (role == "synth_bass")
        return kBusSynthBass;
    if (role == "bass")
        return kBusBass;
    if (role == "chords")
        return kBusChords;
    if (role == "pad")
        return has ("(") ? kBusPad2 : kBusPad;
    if (role == "melody")
        return kBusMelody;
    return kBusOther;
}

} // namespace midiagent
