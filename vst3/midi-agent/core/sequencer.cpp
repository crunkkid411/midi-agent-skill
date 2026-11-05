#include "core/sequencer.h"

#include <algorithm>

namespace midiagent {

void Sequencer::setComposition (std::shared_ptr<const Composition> comp)
{
    comp_ = std::move (comp);
    size_t n = comp_ ? comp_->tracks.size () : 0;
    mute_.resize (n, false);
    solo_.resize (n, false);
    if (comp_)
        for (size_t i = 0; i < n; ++i)
            mute_[i] = comp_->tracks[i].mute;
    anySolo_ = std::any_of (solo_.begin (), solo_.end (), [] (bool b) { return b; });
}

void Sequencer::setTrackMute (size_t track, bool mute)
{
    if (track < mute_.size ())
        mute_[track] = mute;
}

void Sequencer::setTrackSolo (size_t track, bool solo)
{
    if (track < solo_.size ())
        solo_[track] = solo;
    anySolo_ = std::any_of (solo_.begin (), solo_.end (), [] (bool b) { return b; });
}

void Sequencer::clearSolo ()
{
    std::fill (solo_.begin (), solo_.end (), false);
    anySolo_ = false;
}

bool Sequencer::trackAudible (size_t track) const
{
    if (track >= mute_.size ())
        return true;
    if (anySolo_)
        return solo_[track];
    return !mute_[track];
}

void Sequencer::collect (int64_t fromTick, int64_t toTick, double samplesPerTick, int blockOffset,
                         std::vector<SeqEvent>& out)
{
    if (!comp_ || toTick <= fromTick)
        return;
    auto offsetFor = [&] (int64_t tick) {
        double s = static_cast<double> (tick - fromTick) * samplesPerTick;
        return blockOffset + static_cast<int> (std::max (0.0, s));
    };
    // note offs first
    for (size_t i = 0; i < held_.size ();)
    {
        if (held_[i].end >= fromTick && held_[i].end < toTick)
        {
            out.push_back ({offsetFor (held_[i].end), held_[i].track, held_[i].pitch, 0.0f, false});
            held_.erase (held_.begin () + static_cast<std::ptrdiff_t> (i));
        }
        else
            ++i;
    }
    for (size_t ti = 0; ti < comp_->tracks.size (); ++ti)
    {
        if (!trackAudible (ti))
            continue;
        const Track& t = comp_->tracks[ti];
        for (const Note& n : t.notes)
        {
            if (n.start < fromTick || n.start >= toTick)
                continue;
            int64_t end = n.end ();
            // a note that would end inside this block too: keep it at least one sample
            // retrigger: if the same pitch is already held on this track, release it first
            for (size_t i = 0; i < held_.size ();)
            {
                if (held_[i].track == static_cast<int> (ti) && held_[i].pitch == n.pitch)
                {
                    out.push_back ({offsetFor (n.start), held_[i].track, held_[i].pitch, 0.0f, false});
                    held_.erase (held_.begin () + static_cast<std::ptrdiff_t> (i));
                }
                else
                    ++i;
            }
            float vel = static_cast<float> (n.velocity > 1.0 ? n.velocity / 127.0 : n.velocity);
            out.push_back ({offsetFor (n.start), static_cast<int> (ti), n.pitch, std::max (0.01f, std::min (1.0f, vel)), true});
            if (end < toTick)
                out.push_back ({std::max (offsetFor (n.start) + 1, offsetFor (end)), static_cast<int> (ti), n.pitch, 0.0f, false});
            else
                held_.push_back ({static_cast<int> (ti), n.pitch, end});
        }
    }
    std::stable_sort (out.begin (), out.end (), [] (const SeqEvent& a, const SeqEvent& b) {
        if (a.sampleOffset != b.sampleOffset)
            return a.sampleOffset < b.sampleOffset;
        return !a.noteOn && b.noteOn; // offs before ons at the same sample
    });
}

void Sequencer::flush (int at, std::vector<SeqEvent>& out)
{
    for (const Held& h : held_)
        out.push_back ({at, h.track, h.pitch, 0.0f, false});
    held_.clear ();
}

} // namespace midiagent
