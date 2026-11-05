#include "core/instrument_map.h"
#include "core/platform.h"

#include <algorithm>
#include <cmath>

namespace midiagent {

bool InstrumentMap::fromJson (const Json& j, InstrumentMap& m)
{
    if (!j.isObject () || !j.get ("name").isString ())
        return false;
    m = InstrumentMap ();
    m.name = j.get ("name").asString ();
    for (const Json& a : j.get ("aliases").asArray ())
        if (a.isString ())
            m.aliases.push_back (a.asString ());
    m.type = j.get ("type").asString ("melodic");
    if (j.get ("lanes").isObject ())
        for (const auto& kv : j.get ("lanes").asObject ())
            if (kv.second.isNumber ())
                m.lanes[kv.first] = kv.second.asInt ();
    if (j.get ("fallbacks").isObject ())
        for (const auto& kv : j.get ("fallbacks").asObject ())
            if (kv.second.isString ())
                m.fallbacks[kv.first] = kv.second.asString ();
    const Json& range = j.get ("range");
    if (range.isArray () && range.size () == 2)
    {
        m.hasRange = true;
        m.rangeLo = range.at (0).asInt ();
        m.rangeHi = range.at (1).asInt ();
    }
    m.octaveOffset = j.get ("octave_offset").asInt (0);
    m.mode = j.get ("articulation_mode").asString ("none");
    if (j.get ("velocity_bands").isObject ())
        for (const auto& kv : j.get ("velocity_bands").asObject ())
            if (kv.second.isArray () && kv.second.size () == 2)
                m.bands[kv.first] = {kv.second.at (0).asInt (), kv.second.at (1).asInt ()};
    if (j.get ("keyswitches").isObject ())
        for (const auto& kv : j.get ("keyswitches").asObject ())
            if (kv.second.isNumber ())
                m.keyswitches[kv.first] = kv.second.asInt ();
    if (j.get ("articulation_aliases").isObject ())
        for (const auto& kv : j.get ("articulation_aliases").asObject ())
            if (kv.second.isString ())
                m.aliasesArt[kv.first] = kv.second.asString ();
    m.latching = j.get ("keyswitch_latching").asBool (false);
    m.keyswitchLeadBeats = j.get ("keyswitch_lead").asNumber (0.03);
    m.defaultArticulation = j.get ("default_articulation").asString ("");
    m.legatoOverlapBeats = j.get ("legato_overlap").asNumber (0.0);
    m.pickGridBeats = j.get ("pick_grid").asNumber (0.0);
    if (j.get ("upstrokes").isObject ())
        for (const auto& kv : j.get ("upstrokes").asObject ())
            if (kv.second.isString ())
                m.upstrokes[kv.first] = kv.second.asString ();
    if (j.get ("velocity_limits").isObject ())
        for (const auto& kv : j.get ("velocity_limits").asObject ())
            if (kv.second.isArray () && kv.second.size () == 2)
                m.velocityLimits[kv.first] = {kv.second.at (0).asInt (), kv.second.at (1).asInt ()};
    return true;
}

int InstrumentMap::limitVelocity (const std::string& articulation, int velocity) const
{
    const std::vector<std::string> n = names (articulation);
    auto it = n.empty () ? velocityLimits.end () : velocityLimits.find (n.front ());
    if (it == velocityLimits.end ())
        it = velocityLimits.find ("default");
    if (it == velocityLimits.end ())
        return velocity;
    return std::max (it->second.first, std::min (it->second.second, velocity));
}

std::string InstrumentMap::directed (const std::string& articulation, double startBeats) const
{
    const std::vector<std::string> n = names (articulation);
    if (pickGridBeats <= 0.0 || n.empty ())
        return articulation;
    auto it = upstrokes.find (n.front ());
    if (it == upstrokes.end ())
        return articulation;
    // snap to the nearest half-grid step first: humanize nudges notes a few ms off the grid
    const bool up = std::llround (startBeats / (pickGridBeats / 2.0)) % 2 != 0;
    return up ? it->second : articulation;
}

int InstrumentMap::drumNote (const std::string& lane, int gmPitch) const
{
    std::string l = lane;
    for (int guard = 0; guard < 16 && !l.empty (); ++guard)
    {
        auto it = lanes.find (l);
        if (it != lanes.end ())
            return it->second;
        auto f = fallbacks.find (l);
        if (f == fallbacks.end ())
            break;
        l = f->second;
    }
    return gmPitch;
}

int InstrumentMap::fitPitch (int pitch) const
{
    pitch += octaveOffset;
    if (hasRange && rangeHi - rangeLo >= 11)
    {
        while (pitch < rangeLo)
            pitch += 12;
        while (pitch > rangeHi)
            pitch -= 12;
    }
    return std::max (0, std::min (127, pitch));
}

std::vector<std::string> InstrumentMap::names (const std::string& articulation) const
{
    std::string a = articulation.empty () ? defaultArticulation : articulation;
    if (a.empty ())
        return {};
    auto it = aliasesArt.find (a);
    if (it != aliasesArt.end ())
        return {it->second, a};
    return {a};
}

int InstrumentMap::keyswitchFor (const std::string& articulation) const
{
    if (mode.find ("keyswitch") == std::string::npos)
        return -1;
    for (const std::string& n : names (articulation))
    {
        auto it = keyswitches.find (n);
        if (it != keyswitches.end ())
            return it->second;
    }
    return -1;
}

int InstrumentMap::velocityFor (const std::string& articulation, double rel) const
{
    if (mode.find ("velocity") == std::string::npos)
        return -1;
    if (mode == "keyswitch+velocity" && keyswitchFor (articulation) >= 0)
        return -1;
    for (const std::string& n : names (articulation))
    {
        auto it = bands.find (n);
        if (it != bands.end ())
        {
            double r = std::max (0.0, std::min (1.0, rel));
            return static_cast<int> (std::lround (it->second.first + (it->second.second - it->second.first) * r));
        }
    }
    return -1;
}

int InstrumentMaps::loadDir (const std::string& dir)
{
    int n = 0;
    for (const std::string& f : platform::listFiles (dir, ".json"))
    {
        if (f.empty () || f[0] == '_')
            continue;
        std::string text, err;
        Json j;
        InstrumentMap m;
        if (platform::readFile (platform::joinPath (dir, f), text, err) && Json::parse (text, j, err) &&
            InstrumentMap::fromJson (j, m))
        {
            add (std::move (m));
            ++n;
        }
    }
    return n;
}

void InstrumentMaps::add (InstrumentMap m)
{
    for (auto& existing : maps_)
        if (existing.name == m.name)
        {
            existing = std::move (m);
            return;
        }
    maps_.push_back (std::move (m));
}

const InstrumentMap* InstrumentMaps::find (const std::string& slug) const
{
    if (slug.empty ())
        return nullptr;
    for (const auto& m : maps_)
        if (m.name == slug)
            return &m;
    for (const auto& m : maps_)
        if (std::find (m.aliases.begin (), m.aliases.end (), slug) != m.aliases.end ())
            return &m;
    return nullptr;
}

namespace {
struct Span
{
    int key;
    int64_t start, end;
};

// Same rules as skills/generate_midi.py _keyswitch_events.
std::vector<Span> keyswitchEvents (std::vector<Span> spans, bool latching)
{
    std::stable_sort (spans.begin (), spans.end (), [] (const Span& a, const Span& b) { return a.start < b.start; });
    std::vector<Span> out;
    if (latching)
    {
        int current = -1;
        for (const Span& s : spans)
            if (s.key != current)
            {
                out.push_back ({s.key, s.start, s.start + kPPQ / 16});
                current = s.key;
            }
        return out;
    }
    for (const Span& s : spans)
    {
        if (!out.empty () && out.back ().key == s.key && s.start <= out.back ().end)
        {
            out.back ().end = std::max (out.back ().end, s.end);
            continue;
        }
        if (!out.empty () && out.back ().end > s.start)
            out.back ().end = std::max (out.back ().start + 5, s.start); // release before the next engages
        out.push_back (s);
    }
    return out;
}
} // namespace

Composition applyInstrumentMaps (const Composition& comp, const InstrumentMaps& maps)
{
    Composition out = comp;
    for (Track& t : out.tracks)
    {
        const InstrumentMap* m = maps.find (t.vst);
        if (!m || m->type == "phrase")
            continue;
        std::vector<Span> spans;
        const int64_t lead = beatsToTicks (m->keyswitchLeadBeats);
        const int64_t overlap = beatsToTicks (m->legatoOverlapBeats);
        for (Note& n : t.notes)
        {
            if (t.isDrum)
            {
                n.pitch = m->drumNote (n.drum, n.pitch);
                continue;
            }
            n.pitch = m->fitPitch (n.pitch);
            int v = m->velocityFor (n.articulation, n.velocity);
            if (v >= 0)
                n.velocity = std::max (1, std::min (127, v)) / 127.0;
            else // a band is a deliberate mute/pinch velocity; limit only the rest
                n.velocity = m->limitVelocity (n.articulation, static_cast<int> (std::lround (n.velocity * 127.0))) / 127.0;
            int ks = m->keyswitchFor (m->directed (n.articulation, ticksToBeats (n.start)));
            if (ks >= 0)
                spans.push_back ({ks, std::max<int64_t> (0, n.start - lead), n.end ()});
            if (n.articulation == "glide" && overlap > 0)
                n.length += overlap;
        }
        for (const Span& s : keyswitchEvents (std::move (spans), m->latching))
        {
            Note k;
            k.pitch = s.key;
            k.start = s.start;
            k.length = std::max<int64_t> (1, s.end - s.start);
            k.velocity = 100.0 / 127.0;
            k.articulation = kKeyswitchTag;
            t.notes.push_back (k);
        }
        std::stable_sort (t.notes.begin (), t.notes.end (), [] (const Note& a, const Note& b) { return a.start < b.start; });
    }
    return out;
}

} // namespace midiagent
