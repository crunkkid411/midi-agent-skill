// Instrument maps: the C++ twin of midi_types/instrument_map.py. Reads the
// skill's instruments/<slug>.json and turns instrument-agnostic notes (drum
// lanes, articulations) into what the target VST responds to: drum lane ->
// note, articulation -> held keyswitch note and/or velocity band, register fit.
//
// The editor keeps editing the clean notes; the processor plays (and the drag
// files hold) the mapped copy made by applyInstrumentMaps.
#pragma once
#include "core/composition.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace midiagent {

/// Articulation tag on the keyswitch notes applyInstrumentMaps adds. The
/// built-in synth skips them (they are switches, not sounds).
constexpr const char* kKeyswitchTag = "__keyswitch";

struct InstrumentMap
{
    std::string name;
    std::vector<std::string> aliases;
    std::string type {"melodic"};          // drums | melodic | phrase
    std::map<std::string, int> lanes;      // drums: lane -> note
    std::map<std::string, std::string> fallbacks;
    bool hasRange {false};
    int rangeLo {0}, rangeHi {127};
    int octaveOffset {0};
    std::string mode {"none"};             // none | velocity | keyswitch | keyswitch+velocity
    std::map<std::string, std::pair<int, int>> bands;
    std::map<std::string, int> keyswitches; // only the known ones (JSON null = unknown, left out)
    std::map<std::string, std::string> aliasesArt;
    bool latching {false};
    double keyswitchLeadBeats {0.03};
    std::string defaultArticulation;
    double legatoOverlapBeats {0.0};
    double pickGridBeats {0.0};            // notes off this grid are up-strokes (0 = off)
    std::map<std::string, std::string> upstrokes; // down-stroke articulation -> up-stroke partner
    std::map<std::string, std::pair<int, int>> velocityLimits; // resolved articulation or "default"

    static bool fromJson (const Json& j, InstrumentMap& out);

    int drumNote (const std::string& lane, int gmPitch) const;
    int fitPitch (int pitch) const;
    int keyswitchFor (const std::string& articulation) const;         // -1 = none
    int velocityFor (const std::string& articulation, double rel) const; // -1 = none
    // Same rule as midi_types/instrument_map.py InstrumentMap.directed.
    std::string directed (const std::string& articulation, double startBeats) const;
    // Same rule as InstrumentMap.limit_velocity: keeps accents out of Odin's 127 = pinch band.
    int limitVelocity (const std::string& articulation, int velocity) const;

private:
    std::vector<std::string> names (const std::string& articulation) const;
};

class InstrumentMaps
{
public:
    /// Every <dir>/*.json except files starting with '_'. Returns how many loaded.
    int loadDir (const std::string& dir);
    void add (InstrumentMap m);
    const InstrumentMap* find (const std::string& slug) const;
    bool empty () const { return maps_.empty (); }

private:
    std::vector<InstrumentMap> maps_;
};

/// A copy of `comp` with each track that names a known instrument map
/// (Track::vst) rendered for that instrument. Tracks without a map, and
/// phrase-type maps (UJAM style keys), are copied unchanged.
Composition applyInstrumentMaps (const Composition& comp, const InstrumentMaps& maps);

} // namespace midiagent
