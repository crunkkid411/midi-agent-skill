#include "core/midifile.h"
#include "core/platform.h"
#include "tests/test_framework.h"

using namespace midiagent;

static uint32_t be32 (const std::vector<uint8_t>& d, size_t at)
{
    return (uint32_t (d[at]) << 24) | (uint32_t (d[at + 1]) << 16) | (uint32_t (d[at + 2]) << 8) | d[at + 3];
}

TEST_CASE (midi_channels_match_python)
{
    CHECK_EQ (midiChannelForTrack (0, false), 0);
    CHECK_EQ (midiChannelForTrack (3, true), 9);
    CHECK_EQ (midiChannelForTrack (9, false), 10);
    CHECK_EQ (midiChannelForTrack (20, false), 15);
}

TEST_CASE (midi_file_has_header_and_one_chunk_per_track)
{
    Composition c;
    std::string err;
    CHECK (Composition::loadFile (platform::joinPath (MIDIAGENT_FIXTURE_DIR, "scenecore-8bar.composition.json"), c, err));
    std::vector<uint8_t> d = writeMidi (c);
    CHECK (d.size () > 100);
    CHECK_EQ (std::string (d.begin (), d.begin () + 4), std::string ("MThd"));
    CHECK_EQ (be32 (d, 4), 6u);
    CHECK_EQ (d[9], 1);                 // format 1
    CHECK_EQ (d[11], 6);                // 1 tempo track + 5 tracks
    CHECK_EQ ((d[12] << 8) | d[13], kPPQ);
    // walk the chunks
    size_t pos = 14;
    int chunks = 0;
    while (pos + 8 <= d.size ())
    {
        CHECK_EQ (std::string (d.begin () + pos, d.begin () + pos + 4), std::string ("MTrk"));
        uint32_t len = be32 (d, pos + 4);
        pos += 8 + len;
        ++chunks;
    }
    CHECK_EQ (pos, d.size ());
    CHECK_EQ (chunks, 6);

    std::vector<uint8_t> one = writeMidi (c, {2});
    CHECK_EQ (one[11], 2);
}

TEST_CASE (midi_tempo_meta_is_right)
{
    Composition c;
    c.bpm = 120;
    Track t;
    t.name = "x";
    Note n;
    t.notes.push_back (n);
    c.tracks.push_back (t);
    std::vector<uint8_t> d = writeMidi (c);
    // first event of track 0: delta 0, FF 51 03 07 A1 20 (500000 us)
    size_t p = 14 + 8;
    CHECK_EQ (d[p], 0);
    CHECK_EQ (d[p + 1], 0xFF);
    CHECK_EQ (d[p + 2], 0x51);
    CHECK_EQ (d[p + 3], 3);
    CHECK_EQ (d[p + 4], 0x07);
    CHECK_EQ (d[p + 5], 0xA1);
    CHECK_EQ (d[p + 6], 0x20);
}

TEST_CASE (overlapping_same_pitch_notes_are_shortened)
{
    Composition c;
    Track t;
    Note a;
    a.start = 0;
    a.length = 960;
    Note b = a;
    b.start = 480;
    t.notes = {a, b};
    c.tracks.push_back (t);
    std::vector<uint8_t> d = writeMidi (c);
    // find the second chunk and count note on/off order: on(0) off(480) on(0) off(960)
    size_t pos = 14;
    uint32_t len0 = be32 (d, pos + 4);
    pos += 8 + len0 + 8;
    std::vector<std::pair<int, int>> events; // (delta, status)
    size_t end = d.size ();
    while (pos < end)
    {
        uint32_t delta = 0;
        while (d[pos] & 0x80)
            delta = (delta << 7) | (d[pos++] & 0x7F);
        delta = (delta << 7) | (d[pos++] & 0x7F);
        uint8_t status = d[pos];
        if (status == 0xFF)
        {
            uint8_t type = d[pos + 1];
            uint32_t l = d[pos + 2];
            pos += 3 + l;
            if (type == 0x2F)
                break;
            continue;
        }
        events.push_back ({static_cast<int> (delta), status & 0xF0});
        pos += ((status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0) ? 2 : 3;
    }
    // program change, then on, off, on, off
    CHECK_EQ (events.size (), size_t (5));
    CHECK_EQ (events[1].second, 0x90);
    CHECK_EQ (events[2].second, 0x80);
    CHECK_EQ (events[2].first, 480);
    CHECK_EQ (events[3].second, 0x90);
    CHECK_EQ (events[3].first, 0);
    CHECK_EQ (events[4].first, 960);
}
