#include "source/ui/segment_strip_view.h"
#include "source/ui/theme.h"

#include <algorithm>
#include <cmath>

namespace midiagent {

using namespace VSTGUI;

namespace {
constexpr double kPad = 8.0;
constexpr double kParentH = 14.0;
} // namespace

SegmentStripView::SegmentStripView (const CRect& size, Session* session, std::function<void (int, bool, bool)> onSelect)
: CView (size), s_ (session), onSelect_ (std::move (onSelect))
{
    setMouseEnabled (true);
}

CRect SegmentStripView::tileRect (size_t index) const
{
    CRect r = getViewSize ();
    int64_t total = s_->comp.lengthTicks ();
    if (index >= s_->segments.size () || total <= 0)
        return CRect ();
    const Segment& seg = s_->segments[index];
    double w = r.getWidth () - 2 * kPad;
    double x0 = r.left + kPad + w * static_cast<double> (seg.startTick) / total;
    double x1 = r.left + kPad + w * static_cast<double> (seg.endTick) / total;
    return CRect (x0 + 1, r.top + kParentH + 4, x1 - 1, r.bottom - 6);
}

int SegmentStripView::tileAt (const CPoint& p) const
{
    for (size_t i = 0; i < s_->segments.size (); ++i)
        if (tileRect (i).pointInside (p))
            return static_cast<int> (i);
    return -1;
}

void SegmentStripView::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    ctx->setFillColor (theme::kPanel);
    ctx->drawRect (r, kDrawFilled);
    theme::drawCheckerStrip (ctx, CRect (r.left, r.top, r.right, r.top + 4));

    if (s_->segments.empty ())
    {
        ctx->setFont (theme::labelFont ());
        ctx->setFontColor (theme::kTextDim);
        ctx->drawString ("NO SEGMENTS", r, kCenterText);
        setDirty (false);
        return;
    }

    // parent section labels
    ctx->setFont (theme::tinyFont ());
    std::string lastParent;
    double parentStart = 0;
    auto flushParent = [&] (double endX) {
        if (lastParent.empty ())
            return;
        CRect pr (parentStart, r.top + 5, endX, r.top + kParentH + 3);
        ctx->setFontColor (theme::kViolet);
        ctx->drawString (lastParent.c_str (), pr, kLeftText);
        ctx->setFrameColor (theme::kViolet);
        ctx->setLineWidth (1.0);
        ctx->drawLine (CPoint (parentStart, pr.bottom), CPoint (endX - 3, pr.bottom));
    };
    for (size_t i = 0; i < s_->segments.size (); ++i)
    {
        const Segment& seg = s_->segments[i];
        std::string parent = seg.name;
        size_t dot = parent.find ('.');
        if (dot != std::string::npos)
            parent = parent.substr (0, dot);
        CRect t = tileRect (i);
        if (parent != lastParent)
        {
            flushParent (t.left);
            lastParent = parent;
            parentStart = t.left + 2;
        }
    }
    flushParent (r.right - kPad);

    for (size_t i = 0; i < s_->segments.size (); ++i)
    {
        const Segment& seg = s_->segments[i];
        CRect t = tileRect (i);
        bool selected = s_->isSegmentSelected (static_cast<int> (i));
        bool focused = static_cast<int> (i) == s_->selectedSegment;
        bool hover = static_cast<int> (i) == hover_;
        if (selected)
        {
            CRect glow = t;
            glow.extend (2, 2);
            theme::fillRoundRect (ctx, glow, 7, CColor (255, 43, 214, 80));
            theme::fillGradient (ctx, t, 5, CColor (255, 120, 230), CColor (150, 20, 120));
        }
        else
            theme::fillGradient (ctx, t, 5, hover ? CColor (64, 44, 100) : theme::kPanelLight, theme::kPanel);
        CRect shine = t;
        shine.inset (1, 1);
        shine.bottom = shine.top + t.getHeight () * 0.4;
        theme::fillGradient (ctx, shine, 4, CColor (255, 255, 255, selected ? 110 : 40), CColor (255, 255, 255, 0));
        theme::strokeRoundRect (ctx, t, 5, focused ? theme::kLime : (selected ? theme::kChromeHi : (hover ? theme::kCyan : theme::kChromeLo)),
                                focused ? 2.0 : 1.0);

        std::string name = seg.name;
        size_t dot = name.find ('.');
        if (dot != std::string::npos)
            name = name.substr (dot + 1);
        std::string label = name + "  " + std::to_string (seg.bars ()) + (seg.bars () == 1 ? " bar" : " bars");
        ctx->setFont (theme::labelFont ());
        ctx->setFontColor (selected ? theme::kChromeHi : theme::kText);
        CRect lr = t;
        lr.inset (4, 0);
        if (t.getWidth () > 40)
            ctx->drawString (t.getWidth () > 110 ? label.c_str () : name.c_str (), lr, kCenterText);
    }

    // playhead
    int64_t total = s_->comp.lengthTicks ();
    if (total > 0)
    {
        int64_t tick = s_->transport.tick ();
        if (tick >= 0 && tick < total)
        {
            double x = r.left + kPad + (r.getWidth () - 2 * kPad) * static_cast<double> (tick) / total;
            ctx->setFrameColor (theme::kLime);
            ctx->setLineWidth (2.0);
            ctx->drawLine (CPoint (x, r.top + kParentH), CPoint (x, r.bottom - 3));
        }
    }
    setDirty (false);
}

void SegmentStripView::onMouseDownEvent (MouseDownEvent& event)
{
    // On Jordan's PC ctrl + left click arrives as a right click with ctrl held.
    bool ctrl = event.modifiers.has (ModifierKey::Control);
    bool shift = event.modifiers.has (ModifierKey::Shift);
    if (!event.buttonState.isLeft () && !(event.buttonState.isRight () && ctrl))
        return;
    int i = tileAt (event.mousePosition);
    if (i >= 0 && onSelect_)
        onSelect_ (i, ctrl, shift);
    event.consumed = true;
}

void SegmentStripView::onMouseMoveEvent (MouseMoveEvent& event)
{
    int i = tileAt (event.mousePosition);
    if (i != hover_)
    {
        hover_ = i;
        invalid ();
    }
}

void SegmentStripView::onMouseExitEvent (MouseExitEvent& event)
{
    hover_ = -1;
    invalid ();
    event.consumed = true;
}

} // namespace midiagent
