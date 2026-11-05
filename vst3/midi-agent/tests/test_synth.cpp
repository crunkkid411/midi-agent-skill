#include "core/renderer.h"
#include "core/synth.h"
#include "core/platform.h"
#include "tests/test_framework.h"

#include <cmath>

using namespace midiagent;

TEST_CASE (timbre_follows_role_then_instrument)
{
    Track t;
    t.role = "bass";
    CHECK (timbreForTrack (t) == Timbre::SubBass);
    t.role = "";
    t.instrument = "pad-3-polysynth";
    CHECK (timbreForTrack (t) == Timbre::Pad);
    t.isDrum = true;
    CHECK (timbreForTrack (t) == Timbre::Drums);
}

TEST_CASE (synth_makes_sound_and_goes_silent)
{
    Synth s;
    s.setSampleRate (44100);
    CHECK (s.isSilent ());
    s.noteOn (0, 60, 0.8f, Timbre::Saw, 0.0f);
    std::vector<float> l (4410, 0.0f), r (4410, 0.0f);
    s.render (l.data (), r.data (), 4410);
    float peak = 0.0f;
    for (float v : l)
        peak = std::max (peak, std::fabs (v));
    CHECK (peak > 0.05f);
    CHECK (!s.isSilent ());
    s.noteOff (0, 60);
    for (int i = 0; i < 20; ++i)
        s.render (l.data (), r.data (), 4410);
    CHECK (s.isSilent ());
    // drums are one-shot: no note off needed
    s.noteOn (1, 36, 1.0f, Timbre::Drums, 0.0f);
    for (int i = 0; i < 20; ++i)
        s.render (l.data (), r.data (), 4410);
    CHECK (s.isSilent ());
}

TEST_CASE (synth_voice_stealing_never_exceeds_the_pool)
{
    Synth s;
    for (int i = 0; i < 200; ++i)
        s.noteOn (0, 40 + (i % 40), 0.5f, Timbre::Pad, 0.0f);
    CHECK (s.activeVoices () <= Synth::kMaxVoices);
}

TEST_CASE (render_fixture_segment_to_wav)
{
    Composition c;
    std::string err;
    CHECK (Composition::loadFile (platform::joinPath (MIDIAGENT_FIXTURE_DIR, "scenecore-8bar.composition.json"), c, err));
    auto segs = buildSegments (c);
    RenderOptions o;
    o.sampleRate = 22050;
    o.tailSeconds = 0.2;
    RenderResult r = renderComposition (extractSegment (c, segs[0]), o);
    // 4 bars at 147 bpm = 16 beats = 6.53 s, plus tail
    CHECK (r.seconds > 6.5);
    CHECK (r.seconds < 9.0);
    float peak = 0.0f;
    for (float v : r.left)
        peak = std::max (peak, std::fabs (v));
    CHECK_NEAR (peak, 0.89, 0.02);
    std::string path = platform::joinPath (platform::tempDir (), "midiagent-test-segment.wav");
    CHECK (renderToWav (c, &segs[0], o, path, err));
    CHECK (platform::fileExists (path));
}
