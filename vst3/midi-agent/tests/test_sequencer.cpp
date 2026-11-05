#include "core/sequencer.h"
#include "tests/test_framework.h"

#include <memory>

using namespace midiagent;

static std::shared_ptr<const Composition> twoNotes ()
{
    auto c = std::make_shared<Composition> ();
    Track t;
    Note a;
    a.pitch = 60;
    a.start = 0;
    a.length = 480;
    Note b;
    b.pitch = 64;
    b.start = 480;
    b.length = 960;
    t.notes = {a, b};
    c->tracks.push_back (t);
    Track d;
    d.isDrum = true;
    Note k;
    k.pitch = 36;
    k.start = 240;
    k.length = 120;
    d.notes = {k};
    c->tracks.push_back (d);
    return c;
}

TEST_CASE (sequencer_emits_events_with_sample_offsets)
{
    Sequencer s;
    s.setComposition (twoNotes ());
    std::vector<SeqEvent> ev;
    // one block of 480 ticks, 2 samples per tick
    s.collect (0, 480, 2.0, 0, ev);
    CHECK_EQ (ev.size (), size_t (3)); // on C4, on kick, off kick (ends at 360 < 480)
    CHECK (ev[0].noteOn);
    CHECK_EQ (ev[0].pitch, 60);
    CHECK_EQ (ev[1].sampleOffset, 480);
    CHECK_EQ (ev[2].sampleOffset, 720);
    CHECK (!ev[2].noteOn);
    CHECK (s.anyHeld ()); // C4 ends at tick 480, in the next block
    ev.clear ();
    s.collect (480, 960, 2.0, 0, ev);
    CHECK_EQ (ev.size (), size_t (2));
    CHECK (!ev[0].noteOn); // off C4 at offset 0
    CHECK_EQ (ev[0].pitch, 60);
    CHECK (ev[1].noteOn);
    CHECK_EQ (ev[1].pitch, 64);
    ev.clear ();
    s.flush (7, ev);
    CHECK_EQ (ev.size (), size_t (1));
    CHECK_EQ (ev[0].sampleOffset, 7);
    CHECK (!s.anyHeld ());
}

TEST_CASE (sequencer_mute_and_solo)
{
    Sequencer s;
    s.setComposition (twoNotes ());
    s.setTrackMute (0, true);
    std::vector<SeqEvent> ev;
    s.collect (0, 480, 1.0, 0, ev);
    CHECK_EQ (ev.size (), size_t (2)); // only the kick
    CHECK_EQ (ev[0].track, 1);
    s.setTrackMute (0, false);
    s.setTrackSolo (0, true);
    ev.clear ();
    s.collect (0, 480, 1.0, 0, ev);
    CHECK_EQ (ev.size (), size_t (1));
    CHECK_EQ (ev[0].track, 0);
    s.clearSolo ();
    CHECK (s.trackAudible (1));
}

TEST_CASE (sequencer_block_offset_is_added)
{
    Sequencer s;
    s.setComposition (twoNotes ());
    std::vector<SeqEvent> ev;
    s.collect (0, 240, 1.0, 100, ev);
    CHECK_EQ (ev.size (), size_t (1));
    CHECK_EQ (ev[0].sampleOffset, 100);
}
