#include "core/composition.h"
#include "core/platform.h"
#include "tests/test_framework.h"

using namespace midiagent;

static std::string fixture (const char* name)
{
    return platform::joinPath (MIDIAGENT_FIXTURE_DIR, name);
}

TEST_CASE (pitch_parsing_matches_python)
{
    int p = 0;
    CHECK (parsePitch ("C4", p));
    CHECK_EQ (p, 60);
    CHECK (parsePitch ("F#5", p));
    CHECK_EQ (p, 78);
    CHECK (parsePitch ("Bb3", p));
    CHECK_EQ (p, 58);
    CHECK (parsePitch ("36", p));
    CHECK_EQ (p, 36);
    CHECK (parsePitch ("C-1", p));
    CHECK_EQ (p, 0);
    CHECK (!parsePitch ("H4", p));
    CHECK (!parsePitch ("", p));
    CHECK_EQ (pitchName (60), std::string ("C4"));
    CHECK_EQ (pitchName (61), std::string ("C#4"));
    CHECK_EQ (pitchName (0), std::string ("C-1"));
    for (int i = 0; i < 128; ++i)
    {
        int back = -1;
        CHECK (parsePitch (pitchName (i), back));
        CHECK_EQ (back, i);
    }
}

TEST_CASE (duration_labels_round_trip)
{
    CHECK_NEAR (parseDurationBeats ("4"), 1.0, 1e-12);
    CHECK_NEAR (parseDurationBeats ("d4"), 1.5, 1e-12);
    CHECK_NEAR (parseDurationBeats ("T8"), 1.0 / 6.0, 1e-12);
    CHECK_NEAR (parseDurationBeats ("3"), 4.0 / 3.0, 1e-12);
    CHECK_NEAR (parseDurationBeats ("bogus"), 1.0, 1e-12);
    CHECK_EQ (durationLabel (1.0), std::string ("4"));
    CHECK_EQ (durationLabel (0.75), std::string ("d8"));
    CHECK_EQ (durationLabel (1.0 / 3.0), std::string ("T4"));
    // an edited length has no label: a division string Python parses back
    std::string odd = durationLabel (1.25);
    CHECK_NEAR (parseDurationBeats (odd), 1.25, 1e-6);
    CHECK_EQ (beatsToTicks (1.0 / 3.0), int64_t (160));
}

TEST_CASE (fixture_loads_with_sections_and_tracks)
{
    Composition c;
    std::string err;
    CHECK (Composition::loadFile (fixture ("scenecore-8bar.composition.json"), c, err));
    CHECK (err.empty ());
    CHECK_EQ (c.title, std::string ("vst-fixture"));
    CHECK_NEAR (c.bpm, 147.0, 1e-9);
    CHECK_EQ (c.tracks.size (), size_t (5));
    CHECK_EQ (c.sections.size (), size_t (3));
    CHECK_EQ (c.key, std::string ("A minor"));
    CHECK (c.hasBars);
    CHECK_EQ (c.bars, 8);
    CHECK_EQ (c.tracks[1].name, std::string ("Drums"));
    CHECK (c.tracks[1].isDrum);
    CHECK_EQ (c.tracks[1].vst, std::string ("ujam-beatmaker"));
    CHECK_EQ (c.tracks[0].notes[0].pitch, 57); // A3
    CHECK_EQ (c.tracks[0].notes[0].length, int64_t (120)); // "16"
    CHECK_EQ (c.sections[2].name, std::string ("verse.B"));
    CHECK_NEAR (c.sections[2].startBeat (), 16.0, 1e-9);
    CHECK_EQ (c.lengthTicks (), int64_t (8 * 4 * kPPQ));
    CHECK_EQ (c.barCount (), 8);
}

TEST_CASE (fixture_round_trips_through_json)
{
    Composition c;
    std::string err;
    CHECK (Composition::loadFile (fixture ("scenecore-8bar.composition.json"), c, err));
    std::string text = c.toString ();
    Composition again;
    CHECK (Composition::fromString (text, again, err));
    CHECK_EQ (again.toString (), text);
    CHECK_EQ (again.tracks.size (), c.tracks.size ());
    for (size_t t = 0; t < c.tracks.size (); ++t)
    {
        CHECK_EQ (again.tracks[t].notes.size (), c.tracks[t].notes.size ());
        for (size_t n = 0; n < c.tracks[t].notes.size (); ++n)
        {
            CHECK_EQ (again.tracks[t].notes[n].pitch, c.tracks[t].notes[n].pitch);
            CHECK_EQ (again.tracks[t].notes[n].start, c.tracks[t].notes[n].start);
            CHECK_EQ (again.tracks[t].notes[n].length, c.tracks[t].notes[n].length);
            CHECK_EQ (again.tracks[t].notes[n].drum, c.tracks[t].notes[n].drum);
        }
    }
    // bindings and the unknown keys survive
    CHECK_EQ (again.sections[1].bindings.get ("drum_pattern").asString (), std::string ("halftime-drop"));
    // the output is still what the Python side expects
    Json j;
    CHECK (Json::parse (text, j, err));
    CHECK (j.get ("tracks").at (0).get ("notes").at (0).has ("start"));
    CHECK_EQ (j.get ("tracks").at (1).get ("notes").at (0).get ("pitch").asString (), std::string ("49"));
    CHECK_EQ (j.get ("tracks").at (0).get ("notes").at (0).get ("pitch").asString (), std::string ("A3"));
}

TEST_CASE (sequential_notes_without_start_are_placed_in_order)
{
    Composition c;
    std::string err;
    CHECK (Composition::fromString (R"({"title":"t","bpm":100,"tracks":[{"instrument":"piano","notes":[
        {"pitch":"C4","duration":"4"},{"pitch":"E4","duration":"8"},{"pitch":"G4","duration":"8","start":4}]}]})", c, err));
    CHECK_EQ (c.tracks[0].notes[0].start, int64_t (0));
    CHECK_EQ (c.tracks[0].notes[1].start, int64_t (480));
    CHECK_EQ (c.tracks[0].notes[2].start, int64_t (1920));
    CHECK_EQ (c.tracks[0].notes[1].velocity127 (), 100);
}

TEST_CASE (bad_input_is_an_error_not_an_empty_song)
{
    Composition c;
    std::string err;
    CHECK (!Composition::fromString ("[1,2]", c, err));
    CHECK (!err.empty ());
    CHECK (!Composition::fromString ("{\"title\": ", c, err));
    CHECK (!Composition::loadFile (fixture ("does-not-exist.json"), c, err));
}

TEST_CASE (make_empty_has_roles_and_a_segment)
{
    Composition c = Composition::makeEmpty ("new", 140, 8);
    CHECK_EQ (c.tracks.size (), size_t (4));
    CHECK_EQ (c.sections.size (), size_t (1));
    CHECK_EQ (c.barCount (), 8);
    CHECK (c.tracks[1].isDrum);
}
