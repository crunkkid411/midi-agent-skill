#include "core/segments.h"

#include <algorithm>

namespace midiagent {

std::vector<Segment> buildSegments (const Composition& comp)
{
    std::vector<Segment> out;
    // Leaf sections: micro-sections, and top-level sections with no children.
    for (size_t i = 0; i < comp.sections.size (); ++i)
    {
        const Section& s = comp.sections[i];
        bool hasChild = false;
        if (!s.isMicro ())
            for (const Section& k : comp.sections)
                if (k.name.size () > s.name.size () + 1 && k.name.compare (0, s.name.size () + 1, s.name + ".") == 0)
                {
                    hasChild = true;
                    break;
                }
        if (hasChild)
            continue;
        Segment seg;
        seg.name = s.name;
        seg.startBar = s.startBar;
        seg.endBar = s.endBar;
        seg.startTick = beatsToTicks (s.startBeat ());
        seg.endTick = beatsToTicks (s.endBeat ());
        seg.isMicro = s.isMicro ();
        seg.sectionIndex = static_cast<int> (i);
        if (seg.endTick > seg.startTick)
            out.push_back (seg);
    }
    std::stable_sort (out.begin (), out.end (), [] (const Segment& a, const Segment& b) { return a.startTick < b.startTick; });
    if (!out.empty ())
        return out;

    // No sections: cut the song into 4 bar segments.
    int64_t tpb = comp.ticksPerBar ();
    int bars = std::max (1, comp.barCount ());
    for (int b = 0; b < bars; b += 4)
    {
        Segment seg;
        int last = std::min (bars, b + 4);
        seg.startBar = b + 1;
        seg.endBar = last;
        seg.startTick = b * tpb;
        seg.endTick = last * tpb;
        seg.name = "bars " + std::to_string (seg.startBar) + "-" + std::to_string (seg.endBar);
        out.push_back (seg);
    }
    return out;
}

std::vector<size_t> notesInSegment (const Track& track, const Segment& segment)
{
    std::vector<size_t> out;
    for (size_t i = 0; i < track.notes.size (); ++i)
    {
        const Note& n = track.notes[i];
        if (n.start >= segment.startTick && n.start < segment.endTick)
            out.push_back (i);
    }
    return out;
}

Composition extractSegment (const Composition& comp, const Segment& segment, int trackIndex)
{
    Composition clip;
    clip.title = comp.title + "-" + segment.name;
    clip.bpm = comp.tempoAt (segment.startTick);
    clip.timeSigNum = comp.timeSigNum;
    clip.timeSigDen = comp.timeSigDen;
    clip.key = comp.key;
    clip.tuning = comp.tuning;
    clip.hasBars = true;
    clip.bars = segment.bars ();
    Section s;
    s.name = segment.name;
    s.startBar = 1;
    s.endBar = segment.bars ();
    if (segment.sectionIndex >= 0 && segment.sectionIndex < static_cast<int> (comp.sections.size ()))
    {
        const Section& src = comp.sections[static_cast<size_t> (segment.sectionIndex)];
        s.beatsPerBar = src.beatsPerBar;
        s.bindings = src.bindings;
        s.hasDensity = src.hasDensity;
        s.density = src.density;
    }
    clip.sections.push_back (s);
    for (size_t ti = 0; ti < comp.tracks.size (); ++ti)
    {
        if (trackIndex >= 0 && static_cast<size_t> (trackIndex) != ti)
            continue;
        const Track& src = comp.tracks[ti];
        Track t = src;
        t.notes.clear ();
        t.mute = false;
        for (size_t ni : notesInSegment (src, segment))
        {
            Note n = src.notes[ni];
            n.start -= segment.startTick;
            n.length = std::min (n.length, segment.lengthTicks () - n.start);
            if (n.length > 0)
                t.notes.push_back (n);
        }
        // automation events are in beats from the song start; keep the ones inside
        if (t.automation.isArray ())
        {
            Json kept = Json::array ();
            double b0 = ticksToBeats (segment.startTick), b1 = ticksToBeats (segment.endTick);
            for (const Json& ev : t.automation.asArray ())
            {
                double time = ev.get ("time").asNumber ();
                if (time >= b0 && time < b1)
                {
                    Json e = ev;
                    e.set ("time", time - b0);
                    kept.push (e);
                }
            }
            t.automation = kept;
        }
        clip.tracks.push_back (std::move (t));
    }
    return clip;
}

void replaceSegmentNotes (Track& track, const Segment& segment, const std::vector<Note>& notes)
{
    std::vector<Note> kept;
    kept.reserve (track.notes.size () + notes.size ());
    for (const Note& n : track.notes)
        if (n.start < segment.startTick || n.start >= segment.endTick)
            kept.push_back (n);
    for (const Note& n : notes)
        if (n.start >= segment.startTick && n.start < segment.endTick)
            kept.push_back (n);
    std::stable_sort (kept.begin (), kept.end (), [] (const Note& a, const Note& b) {
        return a.start != b.start ? a.start < b.start : a.pitch < b.pitch;
    });
    track.notes = std::move (kept);
}

void pasteClip (Composition& dst, const Composition& src, int64_t atTick)
{
    for (size_t si = 0; si < src.tracks.size (); ++si)
    {
        const Track& s = src.tracks[si];
        Track* target = nullptr;
        for (Track& d : dst.tracks)
            if (!s.name.empty () && d.name == s.name)
            {
                target = &d;
                break;
            }
        if (!target)
            for (Track& d : dst.tracks)
                if (!s.role.empty () && d.role == s.role)
                {
                    target = &d;
                    break;
                }
        if (!target && s.name.empty () && s.role.empty () && si < dst.tracks.size ())
            target = &dst.tracks[si];
        if (!target)
        {
            Track t = s;
            t.notes.clear ();
            dst.tracks.push_back (t);
            target = &dst.tracks.back ();
        }
        for (Note n : s.notes)
        {
            n.start += atTick;
            target->notes.push_back (n);
        }
        std::stable_sort (target->notes.begin (), target->notes.end (),
                          [] (const Note& a, const Note& b) { return a.start < b.start; });
    }
}

} // namespace midiagent
