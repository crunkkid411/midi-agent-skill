// The editor's state: the composition being edited, the selection, the
// bridge settings and what the processor last reported. Owned by the
// controller, shared with the views by pointer. UI thread only.
#pragma once

#include "core/agent_bridge.h"
#include "core/composition.h"
#include "core/segments.h"

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
    bool previewing {false};
    int64_t tick () const { return static_cast<int64_t> (ppq * kPPQ); }
};

struct Session
{
    Composition comp;
    std::vector<Segment> segments;
    int selectedSegment {0};
    int selectedTrack {0};
    std::vector<size_t> selectedNotes; // indices into the selected track's notes
    int snapTicks {kPPQ / 4};          // 1/16 note
    std::string compositionPath;       // where the JSON lives on disk ("" = unsaved)
    bool dirty {false};
    uint32_t revision {0};             // bumps on every edit, the editor pushes to the processor
    bool loopPreview {false};

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
