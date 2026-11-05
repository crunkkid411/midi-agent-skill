#include "core/composition.h"
#include "core/platform.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace midiagent {

int64_t beatsToTicks (double beats)
{
    return static_cast<int64_t> (std::llround (beats * kPPQ));
}

double ticksToBeats (int64_t ticks)
{
    return static_cast<double> (ticks) / kPPQ;
}

bool parsePitch (const std::string& text, int& pitch)
{
    std::string s;
    for (char c : text)
        if (c != ' ' && c != '\t')
            s += c;
    if (s.empty ())
        return false;
    // Numeric drum shortcut: "36" -> 36
    bool numeric = true;
    size_t i0 = s[0] == '-' ? 1 : 0;
    if (i0 >= s.size ())
        numeric = false;
    for (size_t i = i0; i < s.size () && numeric; ++i)
        if (s[i] < '0' || s[i] > '9')
            numeric = false;
    if (numeric)
    {
        pitch = std::max (0, std::min (127, std::atoi (s.c_str ())));
        return true;
    }
    static const int semis[7] = {9, 11, 0, 2, 4, 5, 7}; // A B C D E F G
    char letter = static_cast<char> (std::toupper (static_cast<unsigned char> (s[0])));
    if (letter < 'A' || letter > 'G')
        return false;
    size_t i = 1;
    int acc = 0;
    if (i < s.size () && s[i] == '#')
    {
        acc = 1;
        ++i;
    }
    else if (i < s.size () && s[i] == 'b')
    {
        acc = -1;
        ++i;
    }
    if (i >= s.size ())
        return false;
    std::string oct = s.substr (i);
    size_t j = oct[0] == '-' ? 1 : 0;
    if (j >= oct.size ())
        return false;
    for (size_t k = j; k < oct.size (); ++k)
        if (oct[k] < '0' || oct[k] > '9')
            return false;
    int octave = std::atoi (oct.c_str ());
    int midi = (octave + 1) * 12 + semis[letter - 'A'] + acc;
    pitch = std::max (0, std::min (127, midi));
    return true;
}

std::string pitchName (int pitch)
{
    static const char* names[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    pitch = std::max (0, std::min (127, pitch));
    int octave = pitch / 12 - 1;
    return std::string (names[pitch % 12]) + std::to_string (octave);
}

namespace {
struct DurationEntry
{
    const char* label;
    double beats;
};
// DURATION_MAP from skills/generate_midi.py, same order.
const DurationEntry kDurations[] = {
    {"1", 4.0}, {"2", 2.0}, {"d2", 3.0}, {"4", 1.0}, {"d4", 1.5}, {"8", 0.5}, {"d8", 0.75},
    {"16", 0.25}, {"32", 0.125}, {"T4", 1.0 / 3.0}, {"T8", 1.0 / 6.0},
};
} // namespace

double parseDurationBeats (const std::string& label)
{
    for (const auto& d : kDurations)
        if (label == d.label)
            return d.beats;
    char* end = nullptr;
    double division = std::strtod (label.c_str (), &end);
    if (end != label.c_str () && *end == '\0' && division > 0)
        return 4.0 / division;
    return 1.0;
}

std::string durationLabel (double beats)
{
    for (const auto& d : kDurations)
        if (std::fabs (d.beats - beats) < 1e-6)
            return d.label;
    if (beats <= 0)
        return "16";
    char buf[32];
    std::snprintf (buf, sizeof (buf), "%.7g", 4.0 / beats);
    return buf;
}

int Note::velocity127 () const
{
    double v = velocity;
    if (v > 1.0)
        return std::max (1, std::min (127, static_cast<int> (std::lround (v))));
    return std::max (1, std::min (127, static_cast<int> (std::lround (v * 127.0))));
}

std::string Track::displayName (size_t index) const
{
    if (!name.empty ())
        return name;
    if (!role.empty ())
        return role;
    if (!instrument.empty ())
        return instrument;
    return "track" + std::to_string (index + 1);
}

std::string Section::parentName () const
{
    size_t dot = name.find ('.');
    std::string base = dot == std::string::npos ? name : name.substr (0, dot);
    return base;
}

double Section::startBeat () const
{
    if (hasBeatOffset)
        return beatOffset;
    return (startBar - 1) * beatsPerBar;
}

double Section::endBeat () const
{
    if (hasBeatLength)
        return startBeat () + beatLength;
    return startBeat () + bars () * beatsPerBar;
}

namespace {
const char* const kKnownTrackKeys[] = {"name", "instrument", "is_drum", "role", "vst", "pan", "automation", "notes", "mute"};
const char* const kKnownSectionKeys[] = {"name", "start_bar", "end_bar", "density", "bindings", "beats_per_bar", "beat_offset", "beat_length"};
const char* const kKnownTopKeys[] = {"title", "bpm", "tracks", "bars", "time_signature", "sections", "key", "tuning", "tempo_map"};

template <size_t N>
bool isKnown (const std::string& key, const char* const (&keys)[N])
{
    for (const char* k : keys)
        if (key == k)
            return true;
    return false;
}

template <size_t N>
Json collectExtra (const Json& obj, const char* const (&keys)[N])
{
    Json extra = Json::object ();
    for (const auto& kv : obj.asObject ())
        if (!isKnown (kv.first, keys))
            extra.set (kv.first, kv.second);
    return extra;
}

void mergeExtra (Json& target, const Json& extra)
{
    for (const auto& kv : extra.asObject ())
        target.set (kv.first, kv.second);
}
} // namespace

bool Composition::fromJson (const Json& json, Composition& out, std::string& error)
{
    if (!json.isObject ())
    {
        error = "composition must be a JSON object";
        return false;
    }
    Composition c;
    c.title = json.get ("title").asString ("untitled");
    c.bpm = json.get ("bpm").asNumber (90.0);
    if (json.get ("bars").isNumber ())
    {
        c.hasBars = true;
        c.bars = json.get ("bars").asInt ();
    }
    const Json& ts = json.get ("time_signature");
    if (ts.isArray () && ts.size () == 2)
    {
        c.timeSigNum = std::max (1, ts.at (0).asInt (4));
        c.timeSigDen = std::max (1, ts.at (1).asInt (4));
    }
    c.key = json.get ("key").asString ("");
    c.tuning = json.get ("tuning").asString ("");
    for (const Json& t : json.get ("tempo_map").asArray ())
        if (t.isArray () && t.size () == 2)
            c.tempoMap.emplace_back (t.at (0).asNumber (), t.at (1).asNumber ());

    for (const Json& s : json.get ("sections").asArray ())
    {
        if (!s.isObject () || !s.has ("name"))
            continue;
        Section sec;
        sec.name = s.get ("name").asString ();
        sec.startBar = s.get ("start_bar").asInt (1);
        sec.endBar = s.get ("end_bar").asInt (sec.startBar);
        sec.beatsPerBar = s.get ("beats_per_bar").asNumber (4.0);
        if (s.get ("beat_offset").isNumber ())
        {
            sec.hasBeatOffset = true;
            sec.beatOffset = s.get ("beat_offset").asNumber ();
        }
        if (s.get ("beat_length").isNumber ())
        {
            sec.hasBeatLength = true;
            sec.beatLength = s.get ("beat_length").asNumber ();
        }
        if (s.get ("density").isNumber ())
        {
            sec.hasDensity = true;
            sec.density = s.get ("density").asNumber ();
        }
        sec.bindings = s.get ("bindings");
        sec.extra = collectExtra (s, kKnownSectionKeys);
        c.sections.push_back (std::move (sec));
    }

    for (const Json& t : json.get ("tracks").asArray ())
    {
        if (!t.isObject ())
            continue;
        Track tr;
        tr.name = t.get ("name").asString ("");
        tr.instrument = t.get ("instrument").asString ("");
        tr.isDrum = t.get ("is_drum").asBool (false);
        tr.role = t.get ("role").asString ("");
        tr.vst = t.get ("vst").asString ("");
        if (t.get ("pan").isNumber ())
        {
            tr.hasPan = true;
            tr.pan = t.get ("pan").asNumber ();
        }
        tr.mute = t.get ("mute").asBool (false);
        tr.automation = t.get ("automation");
        tr.extra = collectExtra (t, kKnownTrackKeys);
        double cursor = 0.0; // beats, for notes without "start"
        for (const Json& n : t.get ("notes").asArray ())
        {
            if (!n.isObject () || !n.has ("pitch") || !n.has ("duration"))
                continue;
            Note note;
            std::string pitchText = n.get ("pitch").isNumber () ? std::to_string (n.get ("pitch").asInt ())
                                                                 : n.get ("pitch").asString ();
            if (!parsePitch (pitchText, note.pitch))
                continue; // the Python renderer skips bad notes with a warning too
            std::string durText = n.get ("duration").isNumber () ? std::to_string (n.get ("duration").asInt ())
                                                                  : n.get ("duration").asString ();
            double beats = parseDurationBeats (durText);
            note.length = std::max<int64_t> (1, beatsToTicks (beats));
            if (n.get ("start").isNumber ())
                note.start = beatsToTicks (n.get ("start").asNumber ());
            else
            {
                note.start = beatsToTicks (cursor);
                cursor += beats;
            }
            if (n.get ("velocity").isNumber ())
                note.velocity = n.get ("velocity").asNumber ();
            note.articulation = n.get ("articulation").asString ("");
            note.drum = n.get ("drum").asString ("");
            tr.notes.push_back (std::move (note));
        }
        c.tracks.push_back (std::move (tr));
    }
    c.extra = collectExtra (json, kKnownTopKeys);
    out = std::move (c);
    error.clear ();
    return true;
}

bool Composition::fromString (const std::string& text, Composition& out, std::string& error)
{
    Json json;
    if (!Json::parse (text, json, error))
        return false;
    return fromJson (json, out, error);
}

bool Composition::loadFile (const std::string& path, Composition& out, std::string& error)
{
    std::string text;
    if (!platform::readFile (path, text, error))
        return false;
    return fromString (text, out, error);
}

Json Composition::toJson () const
{
    Json root = Json::object ();
    root.set ("title", title);
    double rb = std::round (bpm);
    if (std::fabs (rb - bpm) < 1e-9)
        root.set ("bpm", static_cast<int> (rb));
    else
        root.set ("bpm", bpm);

    Json tracksJson = Json::array ();
    for (const Track& t : tracks)
    {
        Json tj = Json::object ();
        tj.set ("name", t.name.empty () ? Json (nullptr) : Json (t.name));
        tj.set ("instrument", t.instrument.empty () ? Json (nullptr) : Json (t.instrument));
        tj.set ("is_drum", t.isDrum);
        if (!t.role.empty ())
            tj.set ("role", t.role);
        if (!t.vst.empty ())
            tj.set ("vst", t.vst);
        if (t.hasPan)
            tj.set ("pan", t.pan);
        if (t.automation.isArray () && t.automation.size () > 0)
            tj.set ("automation", t.automation);
        if (t.mute)
            tj.set ("mute", true);
        Json notes = Json::array ();
        for (const Note& n : t.notes)
        {
            Json nj = Json::object ();
            nj.set ("pitch", t.isDrum ? std::to_string (n.pitch) : pitchName (n.pitch));
            nj.set ("duration", durationLabel (ticksToBeats (n.length)));
            nj.set ("velocity", n.velocity);
            nj.set ("start", ticksToBeats (n.start));
            if (!n.articulation.empty ())
                nj.set ("articulation", n.articulation);
            if (!n.drum.empty ())
                nj.set ("drum", n.drum);
            notes.push (std::move (nj));
        }
        tj.set ("notes", std::move (notes));
        mergeExtra (tj, t.extra);
        tracksJson.push (std::move (tj));
    }
    root.set ("tracks", std::move (tracksJson));

    if (hasBars)
        root.set ("bars", bars);
    if (timeSigNum != 4 || timeSigDen != 4)
        root.set ("time_signature", Json::Array {Json (timeSigNum), Json (timeSigDen)});
    if (!sections.empty ())
    {
        Json secs = Json::array ();
        for (const Section& s : sections)
        {
            Json sj = Json::object ();
            sj.set ("name", s.name);
            sj.set ("start_bar", s.startBar);
            sj.set ("end_bar", s.endBar);
            if (s.hasDensity)
                sj.set ("density", s.density);
            if (s.bindings.isObject () && s.bindings.size () > 0)
                sj.set ("bindings", s.bindings);
            if (std::fabs (s.beatsPerBar - 4.0) > 1e-9)
                sj.set ("beats_per_bar", s.beatsPerBar);
            if (s.hasBeatOffset)
                sj.set ("beat_offset", s.beatOffset);
            if (s.hasBeatLength)
                sj.set ("beat_length", s.beatLength);
            mergeExtra (sj, s.extra);
            secs.push (std::move (sj));
        }
        root.set ("sections", std::move (secs));
    }
    if (!key.empty ())
        root.set ("key", key);
    if (!tuning.empty ())
        root.set ("tuning", tuning);
    if (!tempoMap.empty ())
    {
        Json tm = Json::array ();
        for (const auto& t : tempoMap)
            tm.push (Json::Array {Json (t.first), Json (t.second)});
        root.set ("tempo_map", std::move (tm));
    }
    mergeExtra (root, extra);
    return root;
}

std::string Composition::toString () const
{
    return toJson ().dump (2) + "\n";
}

bool Composition::saveFile (const std::string& path, std::string& error) const
{
    return platform::writeFile (path, toString (), error);
}

int64_t Composition::ticksPerBar () const
{
    // beats per bar in quarter notes: num * (4 / den)
    double quarters = timeSigNum * 4.0 / std::max (1, timeSigDen);
    return beatsToTicks (quarters);
}

int64_t Composition::lengthTicks () const
{
    int64_t len = hasBars ? bars * ticksPerBar () : 0;
    for (const Track& t : tracks)
        for (const Note& n : t.notes)
            len = std::max (len, n.end ());
    for (const Section& s : sections)
        len = std::max (len, beatsToTicks (s.endBeat ()));
    return len;
}

int Composition::barCount () const
{
    int64_t tpb = ticksPerBar ();
    int64_t len = lengthTicks ();
    if (tpb <= 0)
        return 0;
    return static_cast<int> ((len + tpb - 1) / tpb);
}

double Composition::tempoAt (int64_t tick) const
{
    double t = bpm;
    double beat = ticksToBeats (tick);
    for (const auto& e : tempoMap)
        if (e.first <= beat)
            t = e.second;
    return t;
}

Composition Composition::makeEmpty (const std::string& title, double bpm, int bars)
{
    Composition c;
    c.title = title;
    c.bpm = bpm;
    c.hasBars = true;
    c.bars = bars;
    struct RoleDef
    {
        const char* name;
        const char* role;
        const char* instrument;
        bool drum;
    };
    const RoleDef roles[] = {
        {"Chords", "chords", "lead-2-sawtooth", false},
        {"Drums", "drums", "", true},
        {"Bass", "bass", "synth-bass-1", false},
        {"Lead", "lead", "lead-1-square", false},
    };
    for (const auto& r : roles)
    {
        Track t;
        t.name = r.name;
        t.role = r.role;
        t.instrument = r.instrument;
        t.isDrum = r.drum;
        c.tracks.push_back (std::move (t));
    }
    Section s;
    s.name = "loop";
    s.startBar = 1;
    s.endBar = bars;
    c.sections.push_back (std::move (s));
    return c;
}

} // namespace midiagent
