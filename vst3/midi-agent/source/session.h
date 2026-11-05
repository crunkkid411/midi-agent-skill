// The editor's state: the composition being edited, the selection, the
// bridge settings and what the processor last reported. Owned by the
// controller, shared with the views by pointer. UI thread only.
#pragma once

#include "core/agent_bridge.h"
#include "core/composition.h"
#include "core/instrument_map.h"
#include "core/segments.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace midiagent {

struct TransportInfo
{
    double ppq {0.0};      // host position in quarter notes
    bool playing {false};
    double tempo {120.0};
    int voices {0};
    int64_t tick () const { return static_cast<int64_t> (ppq * kPPQ); }
};

struct Session
{
    Composition comp;
    std::vector<Segment> segments;
    int selectedSegment {0};           // the one the piano roll shows (the last clicked)
    std::vector<int> segmentSelection; // every selected segment, sorted; always holds selectedSegment
    int selectedTrack {0};
    std::vector<size_t> selectedNotes; // indices into the selected track's notes
    int snapTicks {kPPQ / 4};          // 1/16 note
    std::string compositionPath;       // where the JSON lives on disk ("" = unsaved)
    bool dirty {false};
    uint32_t revision {0};             // bumps on every edit, the editor pushes to the processor
    bool loopOn {false};               // LOOP: the host playhead jumps back to the selection start
    double variation {0.5};            // the SURPRISE slider, 0..1
    InstrumentMaps maps;               // <skill>/instruments/*.json

    AgentConfig agent;
    ComposeRequest request;            // the last / current generate settings
    std::vector<std::string> genres;   // slugs found in the skill folder
    std::vector<std::string> log;      // last messages, newest last
    bool busy {false};
    std::string busyLabel;

    TransportInfo transport;

    void rebuildSegments ()
    {
        segments = buildSegments (comp);
        if (selectedSegment >= static_cast<int> (segments.size ()))
            selectedSegment = segments.empty () ? 0 : static_cast<int> (segments.size ()) - 1;
        if (selectedSegment < 0)
            selectedSegment = 0;
        segmentSelection.erase (std::remove_if (segmentSelection.begin (), segmentSelection.end (),
                                                [this] (int i) { return i < 0 || i >= static_cast<int> (segments.size ()); }),
                                segmentSelection.end ());
        if (std::find (segmentSelection.begin (), segmentSelection.end (), selectedSegment) == segmentSelection.end ())
            segmentSelection = {selectedSegment};
        if (selectedTrack >= static_cast<int> (comp.tracks.size ()))
            selectedTrack = comp.tracks.empty () ? 0 : static_cast<int> (comp.tracks.size ()) - 1;
        if (selectedTrack < 0)
            selectedTrack = 0;
        selectedNotes.clear ();
    }

    const Segment* segment () const
    {
        if (segments.empty () || selectedSegment < 0 || selectedSegment >= static_cast<int> (segments.size ()))
            return nullptr;
        return &segments[static_cast<size_t> (selectedSegment)];
    }

    bool isSegmentSelected (int i) const
    {
        return std::find (segmentSelection.begin (), segmentSelection.end (), i) != segmentSelection.end ();
    }

    /// Click on segment i: plain = only it, ctrl = add/remove it, shift = range
    /// from the last clicked one. The last clicked one is what the piano roll shows.
    void clickSegment (int i, bool ctrl, bool shift)
    {
        if (i < 0 || i >= static_cast<int> (segments.size ()))
            return;
        if (shift)
        {
            int a = std::min (selectedSegment, i), b = std::max (selectedSegment, i);
            if (!ctrl)
                segmentSelection.clear ();
            for (int k = a; k <= b; ++k)
                if (!isSegmentSelected (k))
                    segmentSelection.push_back (k);
        }
        else if (ctrl)
        {
            auto it = std::find (segmentSelection.begin (), segmentSelection.end (), i);
            if (it != segmentSelection.end () && segmentSelection.size () > 1)
            {
                segmentSelection.erase (it);
                if (selectedSegment == i)
                    selectedSegment = segmentSelection.back ();
                std::sort (segmentSelection.begin (), segmentSelection.end ());
                return;
            }
            if (it == segmentSelection.end ())
                segmentSelection.push_back (i);
        }
        else
            segmentSelection = {i};
        selectedSegment = i;
        std::sort (segmentSelection.begin (), segmentSelection.end ());
    }

    /// One segment from the first selected start to the last selected end.
    Segment selectionSpan () const
    {
        Segment span;
        if (segmentSelection.empty () || segments.empty ())
            return segment () ? *segment () : span;
        const Segment& a = segments[static_cast<size_t> (segmentSelection.front ())];
        const Segment& b = segments[static_cast<size_t> (segmentSelection.back ())];
        span = a;
        span.endBar = b.endBar;
        span.endTick = b.endTick;
        if (segmentSelection.size () > 1)
        {
            span.name = a.name + "-" + b.name;
            span.sectionIndex = -1;
        }
        return span;
    }

    Track* track ()
    {
        if (comp.tracks.empty () || selectedTrack < 0 || selectedTrack >= static_cast<int> (comp.tracks.size ()))
            return nullptr;
        return &comp.tracks[static_cast<size_t> (selectedTrack)];
    }

    void touch ()
    {
        dirty = true;
        ++revision;
    }

    void addLog (const std::string& line)
    {
        log.push_back (line);
        if (log.size () > 200)
            log.erase (log.begin (), log.begin () + static_cast<std::ptrdiff_t> (log.size () - 200));
    }
};

} // namespace midiagent
