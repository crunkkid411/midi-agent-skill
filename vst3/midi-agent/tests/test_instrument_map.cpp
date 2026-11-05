#include "core/instrument_map.h"
#include "core/part_buses.h"
#include "core/transport_remote.h"
#include "tests/test_framework.h"

using namespace midiagent;

static InstrumentMap guitarMap ()
{
    std::string err;
    Json j;
    const std::string text = R"({"name":"odin-iii","type":"melodic","range":[24,88],
        "articulation_mode":"keyswitch+velocity","keyswitch_latching":false,
        "keyswitches":{"palm_mute":12,"sustain":13,"chug":null},
        "velocity_bands":{"chug":[100,127]},
        "articulation_aliases":{"tremolo":"sustain"}})";
    CHECK (Json::parse (text, j, err));
    InstrumentMap m;
    CHECK (InstrumentMap::fromJson (j, m));
    return m;
}

TEST_CASE (instrument_map_keyswitch_and_velocity)
{
    InstrumentMap m = guitarMap ();
    CHECK_EQ (m.keyswitchFor ("palm_mute"), 12);
    CHECK_EQ (m.keyswitchFor ("tremolo"), 13);  // alias
    CHECK_EQ (m.keyswitchFor ("chug"), -1);     // null = unknown
    CHECK_EQ (m.velocityFor ("chug", 1.0), 127);
    CHECK_EQ (m.velocityFor ("palm_mute", 1.0), -1); // keyswitched: velocity untouched
    CHECK_EQ (m.fitPitch (100), 88);
    CHECK_EQ (m.fitPitch (10), 34);
}

TEST_CASE (instrument_map_upstrokes_off_the_eighth_grid)
{
    std::string err;
    Json j;
    const std::string text = R"({"name":"odin-iii","articulation_mode":"keyswitch",
        "keyswitches":{"down_open_palm_mute":12,"up_open_palm_mute":15},
        "articulation_aliases":{"palm_mute":"down_open_palm_mute"},
        "pick_grid":0.5,"upstrokes":{"down_open_palm_mute":"up_open_palm_mute"},
        "velocity_limits":{"default":[35,126],"up_open_palm_mute":[35,127]}})";
    CHECK (Json::parse (text, j, err));
    InstrumentMap m;
    CHECK (InstrumentMap::fromJson (j, m));
    CHECK_EQ (m.keyswitchFor (m.directed ("palm_mute", 0.0)), 12);  // on the beat
    CHECK_EQ (m.keyswitchFor (m.directed ("palm_mute", 1.5)), 12);  // on the "&"
    CHECK_EQ (m.keyswitchFor (m.directed ("palm_mute", 1.25)), 15); // 16th in between = up
    CHECK_EQ (m.keyswitchFor (m.directed ("palm_mute", 3.75)), 15);
    CHECK_EQ (m.keyswitchFor (m.directed ("palm_mute", 0.49)), 12); // humanized: still down
    CHECK_EQ (m.limitVelocity ("palm_mute", 127), 126);   // 127 would be Odin's pinch harmonic
    CHECK_EQ (m.limitVelocity ("palm_mute", 10), 35);     // under 35 would be a mute
    CHECK_EQ (m.limitVelocity ("up_open_palm_mute", 127), 127);
}

TEST_CASE (instrument_map_merges_held_keyswitches)
{
    InstrumentMaps maps;
    maps.add (guitarMap ());
    Composition c;
    Track t;
    t.vst = "odin-iii";
    for (int i = 0; i < 4; ++i)
    {
        Note n;
        n.pitch = 40;
        n.start = i * 240;
        n.length = 240;
        n.articulation = i < 3 ? "palm_mute" : "sustain";
        t.notes.push_back (n);
    }
    c.tracks.push_back (t);
    Composition out = applyInstrumentMaps (c, maps);
    int ks12 = 0, ks13 = 0;
    for (const Note& n : out.tracks[0].notes)
        if (n.articulation == kKeyswitchTag)
        {
            if (n.pitch == 12)
            {
                ++ks12;
                CHECK (n.end () <= 720); // released before sustain engages
            }
            if (n.pitch == 13)
                ++ks13;
        }
    CHECK_EQ (ks12, 1);
    CHECK_EQ (ks13, 1);
    CHECK_EQ (out.tracks[0].notes.size (), static_cast<size_t> (6));
}

TEST_CASE (part_bus_routing)
{
    Track t;
    t.role = "rhythm_guitar";
    t.name = "Rhythm Guitar R";
    CHECK_EQ (partBusForTrack (t), static_cast<int> (kBusGuitarR));
    t.name = "Rhythm Guitar (clean)";
    CHECK_EQ (partBusForTrack (t), static_cast<int> (kBusGuitarClean));
    t.name = "Rhythm Guitar";
    CHECK_EQ (partBusForTrack (t), static_cast<int> (kBusGuitarL));
    t.role = "pad";
    t.name = "Pad (choir)";
    CHECK_EQ (partBusForTrack (t), static_cast<int> (kBusPad2));
    t.role = "drums";
    t.name = "Drums";
    CHECK_EQ (partBusForTrack (t), static_cast<int> (kBusDrums));
}

TEST_CASE (mmc_locate_bytes)
{
    auto m = mmcLocateMessage (3661.5); // 1h 1m 1s + 15 frames
    CHECK_EQ (m.size (), static_cast<size_t> (13));
    CHECK_EQ (static_cast<int> (m[4]), 0x44);
    CHECK_EQ (static_cast<int> (m[7]), (3 << 5) | 1);
    CHECK_EQ (static_cast<int> (m[8]), 1);
    CHECK_EQ (static_cast<int> (m[9]), 1);
    CHECK_EQ (static_cast<int> (m[10]), 15);
}
