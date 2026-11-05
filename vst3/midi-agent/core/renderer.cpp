#include "core/renderer.h"
#include "core/sequencer.h"
#include "core/synth.h"
#include "core/wavfile.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace midiagent {

RenderResult renderComposition (const Composition& comp, const RenderOptions& options)
{
    RenderResult r;
    const int sr = options.sampleRate > 1000 ? options.sampleRate : 44100;
    Synth synth;
    synth.setSampleRate (sr);
    synth.setGain (0.5f);
    Sequencer seq;
    auto shared = std::make_shared<const Composition> (comp);
    seq.setComposition (shared);
    if (options.trackIndex >= 0)
        for (size_t i = 0; i < comp.tracks.size (); ++i)
            seq.setTrackMute (i, static_cast<int> (i) != options.trackIndex);

    const int64_t lengthTicks = comp.lengthTicks ();
    const int block = 256;
    int64_t tick = 0;
    std::vector<SeqEvent> events;
    std::vector<float> l (block), rr (block);
    double fracTick = 0.0;
    std::vector<Timbre> timbres;
    std::vector<float> pans;
    for (const Track& t : comp.tracks)
    {
        timbres.push_back (timbreForTrack (t));
        pans.push_back (t.hasPan ? static_cast<float> (t.pan) : 0.0f);
    }

    double tailLeft = options.tailSeconds;
    while (tick < lengthTicks || (tailLeft > 0.0 && !synth.isSilent ()))
    {
        double bpm = comp.tempoAt (tick);
        double ticksPerSample = bpm * kPPQ / 60.0 / sr;
        double samplesPerTick = 1.0 / ticksPerSample;
        double span = block * ticksPerSample + fracTick;
        int64_t spanTicks = static_cast<int64_t> (span);
        fracTick = span - spanTicks;
        int64_t to = tick + spanTicks;
        events.clear ();
        if (tick < lengthTicks)
            seq.collect (tick, std::max (to, tick + 1), samplesPerTick, 0, events);
        std::fill (l.begin (), l.end (), 0.0f);
        std::fill (rr.begin (), rr.end (), 0.0f);
        int pos = 0;
        for (const SeqEvent& e : events)
        {
            int at = std::min (block, std::max (pos, e.sampleOffset));
            if (at > pos)
            {
                synth.render (l.data () + pos, rr.data () + pos, at - pos);
                pos = at;
            }
            size_t ti = static_cast<size_t> (e.track);
            if (e.noteOn)
                synth.noteOn (e.track, e.pitch, e.velocity, ti < timbres.size () ? timbres[ti] : Timbre::Saw,
                              ti < pans.size () ? pans[ti] : 0.0f);
            else
                synth.noteOff (e.track, e.pitch);
        }
        if (pos < block)
            synth.render (l.data () + pos, rr.data () + pos, block - pos);
        r.left.insert (r.left.end (), l.begin (), l.end ());
        r.right.insert (r.right.end (), rr.begin (), rr.end ());
        tick = std::max (to, tick + 1);
        if (tick >= lengthTicks)
        {
            if (seq.anyHeld ())
            {
                events.clear ();
                seq.flush (0, events);
                for (const SeqEvent& e : events)
                    synth.noteOff (e.track, e.pitch);
            }
            tailLeft -= static_cast<double> (block) / sr;
        }
    }
    if (options.normalizePeak > 0.0)
    {
        float peak = 0.0f;
        for (size_t i = 0; i < r.left.size (); ++i)
            peak = std::max (peak, std::max (std::fabs (r.left[i]), std::fabs (r.right[i])));
        if (peak > 1e-6f)
        {
            float g = static_cast<float> (options.normalizePeak) / peak;
            if (g < 1.0f || peak < 0.3f)
                for (size_t i = 0; i < r.left.size (); ++i)
                {
                    r.left[i] *= g;
                    r.right[i] *= g;
                }
        }
    }
    r.seconds = static_cast<double> (r.left.size ()) / sr;
    return r;
}

bool renderToWav (const Composition& comp, const Segment* segment, const RenderOptions& options,
                  const std::string& path, std::string& error)
{
    RenderResult r = segment ? renderComposition (extractSegment (comp, *segment, options.trackIndex < 0 ? -1 : options.trackIndex),
                                                  [&] { RenderOptions o = options; o.trackIndex = -1; return o; } ())
                             : renderComposition (comp, options);
    if (r.left.empty ())
    {
        error = "nothing to render";
        return false;
    }
    return writeWavFile (path, r.left, r.right, options.sampleRate, options.bitsPerSample, error);
}

} // namespace midiagent
