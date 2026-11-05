#include "source/ui/track_list_view.h"
#include "core/synth.h"
#include "source/ui/theme.h"

namespace midiagent {

using namespace VSTGUI;

TrackListView::TrackListView (const CRect& size, Session* session, std::function<void (Action, int)> onAction)
: CView (size), s_ (session), onAction_ (std::move (onAction))
{
    setMouseEnabled (true);
}

CRect TrackListView::rowRect (size_t index) const
{
    CRect r = getViewSize ();
    double top = r.top + 30 + index * kRowH;
    return CRect (r.left + 6, top, r.right - 6, top + kRowH - 4);
}

CRect TrackListView::muteRect (size_t index) const
{
    CRect row = rowRect (index);
    return CRect (row.right - 50, row.top + 7, row.right - 30, row.top + 27);
}

CRect TrackListView::soloRect (size_t index) const
{
    CRect row = rowRect (index);
    return CRect (row.right - 26, row.top + 7, row.right - 6, row.top + 27);
}

void TrackListView::setSolo (size_t track, bool solo)
{
    if (solo_.size () < s_->comp.tracks.size ())
        solo_.resize (s_->comp.tracks.size (), false);
    if (track < solo_.size ())
        solo_[track] = solo;
    invalid ();
}

bool TrackListView::isSolo (size_t track) const
{
    return track < solo_.size () && solo_[track];
}

void TrackListView::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    ctx->setFillColor (theme::kPanel);
    ctx->drawRect (r, kDrawFilled);
    CRect head (r.left + 6, r.top + 6, r.right - 6, r.top + 26);
    theme::drawGlowText (ctx, "TRACKS", head, theme::labelFont (), theme::kCyan, theme::kCyanDim, kLeftText);
    theme::drawSparkle (ctx, CPoint (r.right - 16, r.top + 16), 5, theme::kMagenta);

    if (solo_.size () < s_->comp.tracks.size ())
        solo_.resize (s_->comp.tracks.size (), false);

    for (size_t i = 0; i < s_->comp.tracks.size (); ++i)
    {
        const Track& t = s_->comp.tracks[i];
        CRect row = rowRect (i);
        if (row.top > r.bottom)
            break;
        bool selected = static_cast<int> (i) == s_->selectedTrack;
        bool hover = static_cast<int> (i) == hover_;
        if (selected)
            theme::fillGradient (ctx, row, 6, CColor (70, 36, 110), CColor (40, 20, 70));
        else
            theme::fillRoundRect (ctx, row, 6, hover ? CColor (36, 22, 60) : theme::kPanelLight);
        theme::strokeRoundRect (ctx, row, 6, selected ? theme::kMagenta : theme::kChromeLo, 1.0);

        CRect swatch (row.left + 6, row.top + 8, row.left + 14, row.bottom - 8);
        CColor c = theme::trackColor (i);
        if (t.mute)
            c.alpha = 90;
        theme::fillRoundRect (ctx, swatch, 2, c);

        CRect nameR (row.left + 20, row.top + 3, row.right - 56, row.top + 17);
        ctx->setFont (theme::labelFont ());
        ctx->setFontColor (t.mute ? theme::kTextDim : theme::kText);
        ctx->drawString (t.displayName (i).c_str (), nameR, kLeftText);

        std::string tag = std::string (timbreName (timbreForTrack (t))) + "  " + std::to_string (t.notes.size ()) + " notes";
        if (!t.vst.empty ())
            tag += "  " + t.vst;
        CRect tagR (row.left + 20, row.top + 17, row.right - 56, row.bottom - 2);
        ctx->setFont (theme::tinyFont ());
        ctx->setFontColor (theme::kTextDim);
        ctx->drawString (tag.c_str (), tagR, kLeftText);

        CRect m = muteRect (i), so = soloRect (i);
        theme::fillRoundRect (ctx, m, 4, t.mute ? theme::kDanger : CColor (30, 18, 50));
        theme::strokeRoundRect (ctx, m, 4, t.mute ? theme::kChromeHi : theme::kChromeLo, 1.0);
        theme::fillRoundRect (ctx, so, 4, solo_[i] ? theme::kLime : CColor (30, 18, 50));
        theme::strokeRoundRect (ctx, so, 4, solo_[i] ? theme::kChromeHi : theme::kChromeLo, 1.0);
        ctx->setFont (theme::tinyFont ());
        ctx->setFontColor (t.mute ? theme::kVoid : theme::kText);
        ctx->drawString ("M", m, kCenterText);
        ctx->setFontColor (solo_[i] ? theme::kVoid : theme::kText);
        ctx->drawString ("S", so, kCenterText);
    }
    setDirty (false);
}

void TrackListView::onMouseDownEvent (MouseDownEvent& event)
{
    if (!event.buttonState.isLeft ())
        return;
    for (size_t i = 0; i < s_->comp.tracks.size (); ++i)
    {
        if (muteRect (i).pointInside (event.mousePosition))
        {
            if (onAction_)
                onAction_ (Action::Mute, static_cast<int> (i));
            event.consumed = true;
            return;
        }
        if (soloRect (i).pointInside (event.mousePosition))
        {
            if (onAction_)
                onAction_ (Action::Solo, static_cast<int> (i));
            event.consumed = true;
            return;
        }
        if (rowRect (i).pointInside (event.mousePosition))
        {
            if (onAction_)
                onAction_ (Action::Select, static_cast<int> (i));
            event.consumed = true;
            return;
        }
    }
    event.consumed = true;
}

void TrackListView::onMouseMoveEvent (MouseMoveEvent& event)
{
    int h = -1;
    for (size_t i = 0; i < s_->comp.tracks.size (); ++i)
        if (rowRect (i).pointInside (event.mousePosition))
            h = static_cast<int> (i);
    if (h != hover_)
    {
        hover_ = h;
        invalid ();
    }
}

void TrackListView::onMouseExitEvent (MouseExitEvent& event)
{
    hover_ = -1;
    invalid ();
    event.consumed = true;
}

} // namespace midiagent
