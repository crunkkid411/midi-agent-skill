// The segment strip: every segment of the song as a chrome tile, the parent
// sections above, the playhead across them. Click selects a segment.
#pragma once

#include "source/session.h"

#include "vstgui/lib/cview.h"
#include "vstgui/lib/events.h"

#include <functional>

namespace midiagent {

class SegmentStripView : public VSTGUI::CView
{
public:
    SegmentStripView (const VSTGUI::CRect& size, Session* session, std::function<void (int)> onSelect);

    void draw (VSTGUI::CDrawContext* ctx) override;
    void onMouseDownEvent (VSTGUI::MouseDownEvent& event) override;
    void onMouseMoveEvent (VSTGUI::MouseMoveEvent& event) override;
    void onMouseExitEvent (VSTGUI::MouseExitEvent& event) override;

    CLASS_METHODS (SegmentStripView, CView)
private:
    VSTGUI::CRect tileRect (size_t index) const;
    int tileAt (const VSTGUI::CPoint& p) const;

    Session* s_;
    std::function<void (int)> onSelect_;
    int hover_ {-1};
};

} // namespace midiagent
