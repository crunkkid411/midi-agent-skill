// Segments: the units the plug-in edits, renders and drags. A segment is a
// leaf section of the composition (a micro-section such as "verse1.A" when
// the song has them, else a top-level section). A song without sections is
// cut into 4 bar segments so every composition has a piano roll per segment.
#pragma once
#include "core/composition.h"

#include <string>
#include <vector>

namespace midiagent {

struct Segment
{
    std::string name;
    int startBar {1};   // 1-indexed inclusive
    int endBar {1};
    int64_t startTick {0};
    int64_t endTick {0};
    bool isMicro {false};
    int sectionIndex {-1}; // index into Composition::sections, -1 for synthetic 4 bar segments

    int bars () const { return endBar - startBar + 1; }
    int64_t lengthTicks () const { return endTick - startTick; }
};

std::vector<Segment> buildSegments (const Composition& comp);

/// Indices of the notes of `track` that start inside the segment.
std::vector<size_t> notesInSegment (const Track& track, const Segment& segment);

/// A stand-alone composition holding only the segment: notes shifted so the
/// segment starts at tick 0, one section covering it, title "<title>-<segment>".
/// trackIndex -1 keeps every track, otherwise only that one.
Composition extractSegment (const Composition& comp, const Segment& segment, int trackIndex = -1);

/// Replace the notes of `track` inside the segment with `notes` (absolute ticks).
void replaceSegmentNotes (Track& track, const Segment& segment, const std::vector<Note>& notes);

/// Copy the notes of every track in `src` (a segment extract or any clip)
/// into `dst` at `atTick`. Tracks are matched by name, then role, then index.
void pasteClip (Composition& dst, const Composition& src, int64_t atTick);

} // namespace midiagent
