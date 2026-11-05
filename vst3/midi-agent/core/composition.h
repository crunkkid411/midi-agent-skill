// The composition model: the C++ twin of midi_types/music.py.
//
// Positions are integer ticks at 480 per quarter note (Cubase's unit). The
// JSON files keep the Python convention (start in beats, duration as a label
// such as "8" or "d4"), and the conversion is in this file so a composition
// written by the plug-in loads in the Python pipeline unchanged.
#pragma once
#include "core/json.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace midiagent {

constexpr int kPPQ = 480; // ticks per quarter note, Cubase's resolution

int64_t beatsToTicks (double beats);
double ticksToBeats (int64_t ticks);

/// "C4" / "F#5" / "Bb3" / "36" -> 0..127. Returns false for a bad string.
bool parsePitch (const std::string& text, int& pitch);
/// 60 -> "C4" (the Python convention: C4 = 60).
std::string pitchName (int pitch);
/// "4" -> 1 beat, "d4" -> 1.5, "T8" -> 1/6, "3" -> 4/3. Unknown -> 1 beat.
double parseDurationBeats (const std::string& label);
/// The inverse of parseDurationBeats. Exact labels where one exists, else a
/// division string ("2.6666667" for 1.5 beats) that Python parses back.
std::string durationLabel (double beats);

struct Note
{
    int pitch {60};
    int64_t start {0};        // ticks from the song start
    int64_t length {kPPQ};    // ticks
    double velocity {0.787};  // 0.0..1.0 like the Python side (0.787 == 100/127)
    std::string articulation; // "palm_mute", "glide", ... (passthrough)
    std::string drum;         // drum lane name for drum tracks (passthrough)

    int64_t end () const { return start + length; }
    int velocity127 () const;
};

struct Track
{
    std::string name;
    std::string instrument; // GM slug, empty for drums
    std::string role;       // "drums", "bass", "lead", ...
    std::string vst;        // instrument map slug, empty for General MIDI
    bool isDrum {false};
    bool hasPan {false};
    double pan {0.0};
    bool mute {false};      // plug-in only; written as "mute": true when set
    Json automation;        // passthrough: [{"param","time","value"}, ...]
    std::vector<Note> notes;
    Json extra;             // unknown keys, kept for a clean round trip

    std::string displayName (size_t index) const;
};

struct Section
{
    std::string name;      // "verse1", "verse1.A", "chorus1.drop"
    int startBar {1};      // 1-indexed inclusive
    int endBar {1};
    double beatsPerBar {4.0};
    bool hasBeatOffset {false};
    double beatOffset {0.0};
    bool hasBeatLength {false};
    double beatLength {0.0};
    bool hasDensity {false};
    double density {0.0};
    Json bindings;         // passthrough
    Json extra;

    int bars () const { return endBar - startBar + 1; }
    bool isMicro () const { return name.find ('.') != std::string::npos; }
    std::string parentName () const;
    double startBeat () const;
    double endBeat () const;
};

struct Composition
{
    std::string title {"untitled"};
    double bpm {120.0};
    bool hasBars {false};
    int bars {0};
    int timeSigNum {4};
    int timeSigDen {4};
    std::string key;
    std::string tuning;
    std::vector<std::pair<double, double>> tempoMap; // (beat, bpm)
    std::vector<Section> sections;
    std::vector<Track> tracks;
    Json extra;

    static bool fromJson (const Json& json, Composition& out, std::string& error);
    static bool fromString (const std::string& text, Composition& out, std::string& error);
    static bool loadFile (const std::string& path, Composition& out, std::string& error);
    Json toJson () const;
    std::string toString () const;
    bool saveFile (const std::string& path, std::string& error) const;

    /// Ticks per bar from the global time signature.
    int64_t ticksPerBar () const;
    /// Length in ticks: the declared bars, or the last note end, whichever is later.
    int64_t lengthTicks () const;
    /// Bars covered by lengthTicks ().
    int barCount () const;
    /// Tempo at a tick, following tempoMap.
    double tempoAt (int64_t tick) const;
    /// A short, fresh 8 bar composition with one empty track per common role.
    static Composition makeEmpty (const std::string& title, double bpm, int bars);
};

} // namespace midiagent
