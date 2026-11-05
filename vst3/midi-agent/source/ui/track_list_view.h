// The track list: one row per track with its colour, name, timbre tag,
// note count, and mute / solo switches. Click a row to edit that track.
#pragma once

#include "source/session.h"

#include "vstgui/lib/cview.h"
#include "vstgui/lib/events.h"

#include <functional>

namespace midiagent {

class TrackListView : public VSTGUI::CView
{
public:
    enum class Action { Select, Mute, Solo };

    TrackListView (const VSTGUI::CRect& size, Session* session, std::function<void (Action, int)> onAction);

    void draw (VSTGUI::CDrawContext* ctx) override;
    void onMouseDownEvent (VSTGUI::MouseDownEvent& event) override;
    void onMouseMoveEvent (VSTGUI::MouseMoveEvent& event) override;
    void onMouseExitEvent (VSTGUI::MouseExitEvent& event) override;

    void setSolo (size_t track, bool solo);
    bool isSolo (size_t track) const;

    CLASS_METHODS (TrackListView, CView)
private:
    static constexpr double kRowH = 34.0;
    VSTGUI::CRect rowRect (size_t index) const;
    VSTGUI::CRect muteRect (size_t index) const;
    VSTGUI::CRect soloRect (size_t index) const;

    Session* s_;
    std::function<void (Action, int)> onAction_;
    std::vector<bool> solo_;
    int hover_ {-1};
};

} // namespace midiagent
