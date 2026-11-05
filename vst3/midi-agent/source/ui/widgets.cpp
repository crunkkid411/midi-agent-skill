#include "source/ui/widgets.h"
#include "source/ui/theme.h"

#include "vstgui/lib/cdropsource.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/dragging.h"

namespace midiagent {

using namespace VSTGUI;

// --- NeonButton ---------------------------------------------------------------

NeonButton::NeonButton (const CRect& size, const std::string& title, std::function<void (NeonButton&)> onClick)
: CControl (size, nullptr, -1), title_ (title), onClick_ (std::move (onClick))
{
    setMouseEnabled (true);
}

void NeonButton::setTitle (const std::string& t)
{
    title_ = t;
    invalid ();
}

void NeonButton::setHot (bool hot)
{
    hot_ = hot;
    invalid ();
}

void NeonButton::setEnabledLook (bool enabled)
{
    enabled_ = enabled;
    invalid ();
}

void NeonButton::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    r.inset (1, 1);
    if (!enabled_)
        ctx->setGlobalAlpha (0.45f);
    theme::drawNeonButton (ctx, r, hot_, pressed_, hover_);
    CRect tr = r;
    tr.offset (0, pressed_ ? 1 : 0);
    ctx->setFont (theme::labelFont ());
    ctx->setFontColor (hot_ ? theme::kChromeHi : theme::kText);
    ctx->drawString (title_.c_str (), tr, kCenterText);
    if (!enabled_)
        ctx->setGlobalAlpha (1.0f);
    setDirty (false);
}

void NeonButton::onMouseDownEvent (MouseDownEvent& event)
{
    if (!event.buttonState.isLeft () || !enabled_)
        return;
    pressed_ = true;
    invalid ();
    event.consumed = true;
}

void NeonButton::onMouseUpEvent (MouseUpEvent& event)
{
    if (!pressed_)
        return;
    pressed_ = false;
    bool inside = getViewSize ().pointInside (event.mousePosition);
    if (inside)
    {
        if (toggle_)
        {
            hot_ = !hot_;
            setValue (hot_ ? 1.0f : 0.0f);
        }
        if (onClick_)
            onClick_ (*this);
    }
    invalid ();
    event.consumed = true;
}

void NeonButton::onMouseEnterEvent (MouseEnterEvent& event)
{
    hover_ = true;
    invalid ();
    event.consumed = true;
}

void NeonButton::onMouseExitEvent (MouseExitEvent& event)
{
    hover_ = false;
    invalid ();
    event.consumed = true;
}

void NeonButton::onMouseCancelEvent (MouseCancelEvent& event)
{
    pressed_ = false;
    invalid ();
    event.consumed = true;
}

// --- DragChip -----------------------------------------------------------------

DragChip::DragChip (const CRect& size, const std::string& title, std::function<std::string ()> prepare)
: CView (size), title_ (title), prepare_ (std::move (prepare))
{
    setMouseEnabled (true);
}

void DragChip::setTitle (const std::string& t)
{
    title_ = t;
    invalid ();
}

void DragChip::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    r.inset (1, 1);
    theme::fillGradient (ctx, r, 6, hover_ ? CColor (70, 50, 110) : theme::kPanelLight, theme::kPanel);
    theme::strokeRoundRect (ctx, r, 6, hover_ ? theme::kCyan : theme::kCyanDim, 1.0);
    // grip dots on the left
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 2; ++col)
        {
            CRect dot (r.left + 7 + col * 4, r.top + r.getHeight () / 2 - 5 + row * 4, 0, 0);
            dot.setWidth (2);
            dot.setHeight (2);
            ctx->setFillColor (theme::kCyan);
            ctx->drawRect (dot, kDrawFilled);
        }
    CRect tr = r;
    tr.left += 18;
    ctx->setFont (theme::labelFont ());
    ctx->setFontColor (theme::kCyan);
    ctx->drawString (title_.c_str (), tr, kCenterText);
    setDirty (false);
}

void DragChip::onMouseDownEvent (MouseDownEvent& event)
{
    if (!event.buttonState.isLeft ())
        return;
    armed_ = true;
    downPos_ = event.mousePosition;
    event.consumed = true;
}

void DragChip::onMouseMoveEvent (MouseMoveEvent& event)
{
    if (!armed_ || !event.buttonState.isLeft ())
        return;
    CPoint d = event.mousePosition;
    d.offset (-downPos_.x, -downPos_.y);
    if (std::abs (d.x) < 5 && std::abs (d.y) < 5)
        return;
    armed_ = false;
    std::string path = prepare_ ? prepare_ () : std::string ();
    if (path.empty ())
        return;
    auto source = CDropSource::create (path.c_str (), static_cast<uint32_t> (path.size () + 1), IDataPackage::kFilePath);
    DragDescription desc (source);
    doDrag (desc);
    event.consumed = true;
}

void DragChip::onMouseUpEvent (MouseUpEvent& event)
{
    armed_ = false;
    event.consumed = true;
}

void DragChip::onMouseEnterEvent (MouseEnterEvent& event)
{
    hover_ = true;
    invalid ();
    event.consumed = true;
}

void DragChip::onMouseExitEvent (MouseExitEvent& event)
{
    hover_ = false;
    invalid ();
    event.consumed = true;
}

// --- NeonLabel ----------------------------------------------------------------

NeonLabel::NeonLabel (const CRect& size, const std::string& text, CFontRef font, const CColor& color,
                      CHoriTxtAlign align)
: CView (size), text_ (text), font_ (font), color_ (color), align_ (align)
{
    setMouseEnabled (false);
}

void NeonLabel::setText (const std::string& t)
{
    if (t == text_)
        return;
    text_ = t;
    invalid ();
}

void NeonLabel::setColor (const CColor& c)
{
    color_ = c;
    invalid ();
}

void NeonLabel::draw (CDrawContext* ctx)
{
    if (glow_)
        theme::drawGlowText (ctx, text_, getViewSize (), font_, color_, color_, align_);
    else
    {
        ctx->setFont (font_);
        ctx->setFontColor (color_);
        ctx->drawString (text_.c_str (), getViewSize (), align_);
    }
    setDirty (false);
}

// --- LogView -----------------------------------------------------------------

LogView::LogView (const CRect& size, const std::vector<std::string>* lines) : CView (size), lines_ (lines)
{
    setMouseEnabled (false);
}

void LogView::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    theme::fillRoundRect (ctx, r, 6, CColor (4, 3, 9));
    theme::strokeRoundRect (ctx, r, 6, theme::kGrid, 1.0);
    if (!lines_)
        return;
    const double lineH = 14.0;
    int fit = static_cast<int> ((r.getHeight () - 8) / lineH);
    int n = static_cast<int> (lines_->size ());
    int first = std::max (0, n - fit);
    ctx->setFont (theme::tinyFont ());
    double y = r.top + 4;
    for (int i = first; i < n; ++i)
    {
        const std::string& line = (*lines_)[static_cast<size_t> (i)];
        bool err = line.rfind ("ERROR", 0) == 0;
        ctx->setFontColor (err ? theme::kDanger : (i == n - 1 ? theme::kLime : theme::kTextDim));
        CRect lr (r.left + 8, y, r.right - 8, y + lineH);
        ctx->drawString (line.c_str (), lr, kLeftText);
        y += lineH;
    }
    setDirty (false);
}

} // namespace midiagent
