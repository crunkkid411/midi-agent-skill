// The piano roll: one segment of one track, editable with the mouse and the
// keyboard. Other tracks are drawn as ghosts for context.
//
//   left click empty   add a note (drag right to set its length)
//   left drag note     move; drag its right edge to resize; Alt-drag = velocity
//   shift click        add / remove a note from the selection
//   ctrl drag empty    marquee selection
//   right click        delete the note (or the selection)
//   Delete / Backspace delete the selection
//   arrows             transpose (shift = octave) / nudge by the snap value
//   ctrl A / ctrl D    select all / duplicate the selection after itself
//   mouse wheel        scroll pitches; ctrl + wheel zooms
#pragma once

#include "source/session.h"

#include "vstgui/lib/cview.h"
#include "vstgui/lib/events.h"

#include <functional>
#include <vector>

namespace midiagent {

class PianoRollView : public VSTGUI::CView
{
public:
    PianoRollView (const VSTGUI::CRect& size, Session* session, std::function<void ()> onEdit);

    /// Scroll so the selected track's notes are in view.
    void scrollToContent ();
    void segmentChanged ();

    void draw (VSTGUI::CDrawContext* ctx) override;
    void onMouseDownEvent (VSTGUI::MouseDownEvent& event) override;
    void onMouseMoveEvent (VSTGUI::MouseMoveEvent& event) override;
    void onMouseUpEvent (VSTGUI::MouseUpEvent& event) override;
    void onMouseCancelEvent (VSTGUI::MouseCancelEvent& event) override;
    void onMouseWheelEvent (VSTGUI::MouseWheelEvent& event) override;
    void onMouseExitEvent (VSTGUI::MouseExitEvent& event) override;
    void onKeyboardEvent (VSTGUI::KeyboardEvent& event) override;

    CLASS_METHODS (PianoRollView, CView)

private:
    enum class Drag { None, Move, Resize, Velocity, Create, Marquee };

    VSTGUI::CRect gridRect () const;
    double tickToX (int64_t tick) const;
    int64_t xToTick (double x) const;
    double pitchToY (int pitch) const;
    int yToPitch (double y) const;
    int64_t snap (int64_t tick) const;
    VSTGUI::CRect noteRect (const Note& n) const;
    int hitNote (const VSTGUI::CPoint& p, bool& onRightEdge) const;
    bool isSelected (size_t index) const;
    void select (size_t index, bool add);
    void deleteSelection ();
    void nudgeSelection (int64_t dTicks, int dPitch);
    void duplicateSelection ();
    void selectAll ();
    void clampView ();
    void commit ();

    Session* s_;
    std::function<void ()> onEdit_;
    double keyH_ {13.0};
    int topPitch_ {96};
    static constexpr double kKeyboardW = 56.0;
    static constexpr double kRulerH = 20.0;
    static constexpr double kVelocityLaneH = 0.0;

    Drag drag_ {Drag::None};
    int dragNote_ {-1};
    VSTGUI::CPoint dragStart_;
    std::vector<Note> dragOrig_;
    std::vector<size_t> dragIndices_;
    VSTGUI::CRect marquee_;
    int hoverNote_ {-1};
    bool hoverEdge_ {false};
    bool changedDuringDrag_ {false};
    int64_t lastLength_ {kPPQ / 4};
    double lastVelocity_ {0.787};
};

} // namespace midiagent
