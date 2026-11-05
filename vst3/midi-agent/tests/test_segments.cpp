#include "core/platform.h"
#include "core/segments.h"
#include "tests/test_framework.h"

using namespace midiagent;

static Composition loadFixture ()
{
    Composition c;
    std::string err;
    CHECK (Composition::loadFile (platform::joinPath (MIDIAGENT_FIXTURE_DIR, "scenecore-8bar.composition.json"), c, err));
    return c;
}

TEST_CASE (segments_are_the_leaf_sections)
{
    Composition c = loadFixture ();
    auto segs = buildSegments (c);
    CHECK_EQ (segs.size (), size_t (2));
    CHECK_EQ (segs[0].name, std::string ("verse.A"));
    CHECK_EQ (segs[0].startTick, int64_t (0));
    CHECK_EQ (segs[0].endTick, int64_t (16 * kPPQ));
    CHECK_EQ (segs[1].name, std::string ("verse.B"));
    CHECK_EQ (segs[1].startBar, 5);
    CHECK (segs[1].isMicro);
    CHECK_EQ (segs[1].sectionIndex, 2);
}

TEST_CASE (song_without_sections_is_cut_into_4_bar_segments)
{
    Composition c;
    c.hasBars = true;
    c.bars = 10;
    auto segs = buildSegments (c);
    CHECK_EQ (segs.size (), size_t (3));
    CHECK_EQ (segs[2].startBar, 9);
    CHECK_EQ (segs[2].endBar, 10);
    CHECK_EQ (segs[2].endTick, int64_t (10 * 4 * kPPQ));
    CHECK_EQ (segs[0].sectionIndex, -1);
}

TEST_CASE (extract_segment_shifts_notes_to_zero)
{
    Composition c = loadFixture ();
    auto segs = buildSegments (c);
    Composition clip = extractSegment (c, segs[1]);
    CHECK_EQ (clip.tracks.size (), c.tracks.size ());
    CHECK_EQ (clip.bars, 4);
    CHECK_EQ (clip.sections.size (), size_t (1));
    CHECK_EQ (clip.title, std::string ("vst-fixture-verse.B"));
    size_t expected = notesInSegment (c.tracks[0], segs[1]).size ();
    CHECK_EQ (clip.tracks[0].notes.size (), expected);
    for (const Note& n : clip.tracks[0].notes)
    {
        CHECK (n.start >= 0);
        CHECK (n.end () <= segs[1].lengthTicks ());
    }
    Composition one = extractSegment (c, segs[0], 2);
    CHECK_EQ (one.tracks.size (), size_t (1));
    CHECK_EQ (one.tracks[0].name, std::string ("Bass"));
}

TEST_CASE (replace_segment_notes_keeps_the_rest)
{
    Composition c = loadFixture ();
    auto segs = buildSegments (c);
    Track& bass = c.tracks[2];
    size_t inA = notesInSegment (bass, segs[0]).size ();
    size_t inB = notesInSegment (bass, segs[1]).size ();
    std::vector<Note> fresh;
    Note n;
    n.pitch = 40;
    n.start = segs[0].startTick + 480;
    n.length = 480;
    fresh.push_back (n);
    Note outside = n;
    outside.start = segs[1].startTick; // not inside segment A: dropped
    fresh.push_back (outside);
    replaceSegmentNotes (bass, segs[0], fresh);
    CHECK_EQ (notesInSegment (bass, segs[0]).size (), size_t (1));
    CHECK_EQ (notesInSegment (bass, segs[1]).size (), inB);
    CHECK (inA > 1);
}

TEST_CASE (paste_clip_matches_tracks_by_name)
{
    Composition c = loadFixture ();
    auto segs = buildSegments (c);
    Composition clip = extractSegment (c, segs[0]);
    Composition dst = Composition::makeEmpty ("dst", 120, 8);
    pasteClip (dst, clip, 4 * kPPQ);
    // "Bass" exists in both, "Pad" only in the clip (appended)
    CHECK_EQ (dst.tracks.size (), size_t (5));
    CHECK (dst.tracks[2].notes.size () > 0);
    CHECK_EQ (dst.tracks[2].notes[0].start, int64_t (4 * kPPQ + clip.tracks[2].notes[0].start));
}
