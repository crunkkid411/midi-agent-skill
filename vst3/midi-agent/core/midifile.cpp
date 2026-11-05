#include "core/midifile.h"
#include "core/gm_instruments.h"
#include "core/platform.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace midiagent {

int midiChannelForTrack (int trackIndex, bool isDrum)
{
    if (isDrum)
        return 9;
    if (trackIndex < 9)
        return trackIndex;
    return std::min (15, trackIndex + 1);
}

namespace {

struct MidiEvent
{
    int64_t tick;
    int order; // sort key inside one tick: meta 0, note off 1, cc 2, note on 3
    std::vector<uint8_t> bytes;
};

void putVarLen (std::vector<uint8_t>& out, uint32_t v)
{
    uint8_t buf[5];
    int n = 0;
    buf[n++] = static_cast<uint8_t> (v & 0x7F);
    v >>= 7;
    while (v)
    {
        buf[n++] = static_cast<uint8_t> ((v & 0x7F) | 0x80);
        v >>= 7;
    }
    while (n--)
        out.push_back (buf[n]);
}

void put32 (std::vector<uint8_t>& out, uint32_t v)
{
    out.push_back (static_cast<uint8_t> (v >> 24));
    out.push_back (static_cast<uint8_t> (v >> 16));
    out.push_back (static_cast<uint8_t> (v >> 8));
    out.push_back (static_cast<uint8_t> (v));
}

void put16 (std::vector<uint8_t>& out, uint16_t v)
{
    out.push_back (static_cast<uint8_t> (v >> 8));
    out.push_back (static_cast<uint8_t> (v));
}

MidiEvent meta (int64_t tick, uint8_t type, const std::vector<uint8_t>& data)
{
    MidiEvent e {tick, 0, {0xFF, type}};
    putVarLen (e.bytes, static_cast<uint32_t> (data.size ()));
    e.bytes.insert (e.bytes.end (), data.begin (), data.end ());
    return e;
}

MidiEvent tempoEvent (int64_t tick, double bpm)
{
    uint32_t mpq = static_cast<uint32_t> (std::lround (60000000.0 / std::max (1.0, bpm)));
    return meta (tick, 0x51, {static_cast<uint8_t> (mpq >> 16), static_cast<uint8_t> (mpq >> 8), static_cast<uint8_t> (mpq)});
}

MidiEvent timeSigEvent (int64_t tick, int num, int den)
{
    uint8_t denPow = 2;
    switch (den)
    {
        case 1: denPow = 0; break;
        case 2: denPow = 1; break;
        case 4: denPow = 2; break;
        case 8: denPow = 3; break;
        case 16: denPow = 4; break;
        default: denPow = 2; break;
    }
    return meta (tick, 0x58, {static_cast<uint8_t> (num), denPow, 24, 8});
}

std::vector<uint8_t> encodeTrack (std::vector<MidiEvent>& events)
{
    std::stable_sort (events.begin (), events.end (), [] (const MidiEvent& a, const MidiEvent& b) {
        return a.tick != b.tick ? a.tick < b.tick : a.order < b.order;
    });
    std::vector<uint8_t> data;
    int64_t last = 0;
    for (const MidiEvent& e : events)
    {
        putVarLen (data, static_cast<uint32_t> (std::max<int64_t> (0, e.tick - last)));
        last = std::max (last, e.tick);
        data.insert (data.end (), e.bytes.begin (), e.bytes.end ());
    }
    putVarLen (data, 0);
    data.insert (data.end (), {0xFF, 0x2F, 0x00});
    std::vector<uint8_t> chunk = {'M', 'T', 'r', 'k'};
    put32 (chunk, static_cast<uint32_t> (data.size ()));
    chunk.insert (chunk.end (), data.begin (), data.end ());
    return chunk;
}

// Automation param -> CC, from generate_midi.DEFAULT_CC.
int defaultCC (const std::string& param)
{
    static const std::map<std::string, int> cc = {
        {"mod_wheel", 1}, {"volume", 7}, {"pan", 10}, {"expression", 11}, {"filter", 74}, {"resonance", 71},
        {"macro1", 21}, {"macro2", 22}, {"macro3", 23}, {"macro4", 24},
        {"macro5", 25}, {"macro6", 26}, {"macro7", 27}, {"macro8", 28},
    };
    auto it = cc.find (param);
    return it == cc.end () ? -1 : it->second;
}

} // namespace

std::vector<uint8_t> writeMidi (const Composition& comp, const std::vector<int>& trackIndices)
{
    std::vector<int> indices = trackIndices;
    if (indices.empty ())
        for (size_t i = 0; i < comp.tracks.size (); ++i)
            indices.push_back (static_cast<int> (i));

    std::vector<std::vector<uint8_t>> chunks;

    // Track 0 carries tempo and meter, like midiutil's track 0 in the Python writer.
    {
        std::vector<MidiEvent> ev;
        ev.push_back (tempoEvent (0, comp.bpm));
        for (const auto& t : comp.tempoMap)
            ev.push_back (tempoEvent (beatsToTicks (t.first), t.second));
        ev.push_back (timeSigEvent (0, comp.timeSigNum, comp.timeSigDen));
        std::vector<uint8_t> name (comp.title.begin (), comp.title.end ());
        ev.push_back (meta (0, 0x03, name));
        chunks.push_back (encodeTrack (ev));
    }

    for (int ti : indices)
    {
        if (ti < 0 || ti >= static_cast<int> (comp.tracks.size ()))
            continue;
        const Track& t = comp.tracks[static_cast<size_t> (ti)];
        uint8_t ch = static_cast<uint8_t> (midiChannelForTrack (ti, t.isDrum));
        std::vector<MidiEvent> ev;
        std::string name = t.displayName (static_cast<size_t> (ti));
        ev.push_back (meta (0, 0x03, std::vector<uint8_t> (name.begin (), name.end ())));
        if (!t.isDrum && t.vst.empty ())
        {
            uint8_t program = static_cast<uint8_t> (gmProgramForName (t.instrument));
            ev.push_back ({0, 2, {static_cast<uint8_t> (0xC0 | ch), program}});
        }
        if (t.hasPan)
        {
            int v = static_cast<int> (std::lround (64 + 63 * t.pan));
            ev.push_back ({0, 2, {static_cast<uint8_t> (0xB0 | ch), 10, static_cast<uint8_t> (std::max (0, std::min (127, v)))}});
        }
        for (const Json& a : t.automation.asArray ())
        {
            std::string param = a.get ("param").asString ();
            int64_t tick = beatsToTicks (a.get ("time").asNumber ());
            double value = a.get ("value").asNumber ();
            if (param == "pitch")
            {
                if (a.get ("semitones").isNumber ())
                    value = a.get ("semitones").asNumber () / 2.0;
                int bend = static_cast<int> (std::lround (value * 8191));
                bend = std::max (-8192, std::min (8191, bend)) + 8192;
                ev.push_back ({tick, 2, {static_cast<uint8_t> (0xE0 | ch), static_cast<uint8_t> (bend & 0x7F), static_cast<uint8_t> (bend >> 7)}});
                continue;
            }
            int cc = defaultCC (param);
            if (cc < 0)
                continue;
            int v = std::max (0, std::min (127, static_cast<int> (std::lround (value * 127))));
            ev.push_back ({tick, 2, {static_cast<uint8_t> (0xB0 | ch), static_cast<uint8_t> (cc), static_cast<uint8_t> (v)}});
        }

        // Same-pitch overlaps are shortened to end where the next note starts,
        // like generate_midi._untangle, so a note-off never cuts a later note.
        std::vector<Note> notes = t.notes;
        std::stable_sort (notes.begin (), notes.end (), [] (const Note& a, const Note& b) {
            return a.start != b.start ? a.start < b.start : a.pitch < b.pitch;
        });
        std::map<int, size_t> lastByPitch;
        for (size_t i = 0; i < notes.size (); ++i)
        {
            auto it = lastByPitch.find (notes[i].pitch);
            if (it != lastByPitch.end ())
            {
                Note& prev = notes[it->second];
                if (prev.end () > notes[i].start)
                    prev.length = std::max<int64_t> (1, notes[i].start - prev.start);
            }
            lastByPitch[notes[i].pitch] = i;
        }
        for (const Note& n : notes)
        {
            uint8_t pitch = static_cast<uint8_t> (std::max (0, std::min (127, n.pitch)));
            uint8_t vel = static_cast<uint8_t> (n.velocity127 ());
            ev.push_back ({n.start, 3, {static_cast<uint8_t> (0x90 | ch), pitch, vel}});
            ev.push_back ({n.end (), 1, {static_cast<uint8_t> (0x80 | ch), pitch, 0}});
        }
        chunks.push_back (encodeTrack (ev));
    }

    std::vector<uint8_t> out = {'M', 'T', 'h', 'd'};
    put32 (out, 6);
    put16 (out, 1);
    put16 (out, static_cast<uint16_t> (chunks.size ()));
    put16 (out, static_cast<uint16_t> (kPPQ));
    for (const auto& c : chunks)
        out.insert (out.end (), c.begin (), c.end ());
    return out;
}

bool writeMidiFile (const std::string& path, const Composition& comp, const std::vector<int>& trackIndices,
                    std::string& error)
{
    return platform::writeFile (path, writeMidi (comp, trackIndices), error);
}

} // namespace midiagent
