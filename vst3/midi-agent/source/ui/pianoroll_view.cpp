#include "source/ui/pianoroll_view.h"
#include "source/ui/theme.h"

#include "vstgui/lib/cframe.h"

#include <algorithm>
#include <cmath>

namespace midiagent {

using namespace VSTGUI;

namespace {
const char* drumName (int pitch)
{
    switch (pitch)
    {
        case 35: return "KICK2";
        case 36: return "KICK";
        case 37: return "STICK";
        case 38: return "SNARE";
        case 39: return "CLAP";
        case 40: return "SNR2";
        case 41: return "TOM-L";
        case 42: return "HH-C";
        case 43: return "TOM-L";
        case 44: return "HH-P";
        case 45: return "TOM-M";
        case 46: return "HH-O";
        case 47: return "TOM-M";
        case 48: return "TOM-H";
        case 49: return "CRASH";
        case 50: return "TOM-H";
        case 51: return "RIDE";
        case 52: return "CHINA";
        case 53: return "BELL";
        case 55: return "SPLSH";
        case 57: return "CRSH2";
        case 59: return "RIDE2";
        default: return nullptr;
    }
}

bool isBlackKey (int pitch)
{
    switch (pitch % 12)
    {
        case 1: case 3: case 6: case 8: case 10: return true;
        default: return false;
    }
}
} // namespace

PianoRollView::PianoRollView (const CRect& size, Session* session, std::function<void ()> onEdit)
: CView (size), s_ (session), onEdit_ (std::move (onEdit))
{
    setMouseEnabled (true);
    setWantsFocus (true);
}

CRect PianoRollView::gridRect () const
{
    CRect r = getViewSize ();
    r.left += kKeyboardW;
    r.top += kRulerH;
    return r;
}

double PianoRollView::tickToX (int64_t tick) const
{
    const Segment* seg = s_->segment ();
    CRect g = gridRect ();
    if (!seg || seg->lengthTicks () <= 0)
        return g.left;
    return g.left + (static_cast<double> (tick - seg->startTick) / seg->lengthTicks ()) * g.getWidth ();
}

int64_t PianoRollView::xToTick (double x) const
{
    const Segment* seg = s_->segment ();
    CRect g = gridRect ();
    if (!seg || g.getWidth () <= 0)
        return 0;
    double f = (x - g.left) / g.getWidth ();
    return seg->startTick + static_cast<int64_t> (std::floor (f * seg->lengthTicks ()));
}

double PianoRollView::pitchToY (int pitch) const
{
    return gridRect ().top + (topPitch_ - pitch) * keyH_;
}

int PianoRollView::yToPitch (double y) const
{
    return topPitch_ - static_cast<int> (std::floor ((y - gridRect ().top) / keyH_));
}

int64_t PianoRollView::snap (int64_t tick) const
{
    int64_t snapT = std::max<int64_t> (1, s_->snapTicks);
    const Segment* seg = s_->segment ();
    int64_t base = seg ? seg->startTick : 0;
    int64_t rel = tick - base;
    int64_t snapped = static_cast<int64_t> (std::floor (static_cast<double> (rel) / snapT + 0.5)) * snapT;
    return base + snapped;
}

CRect PianoRollView::noteRect (const Note& n) const
{
    double x0 = tickToX (n.start);
    double x1 = tickToX (n.end ());
    double y = pitchToY (n.pitch);
    return CRect (x0, y + 1, std::max (x0 + 3, x1 - 1), y + keyH_ - 1);
}

int PianoRollView::hitNote (const CPoint& p, bool& onRightEdge) const
{
    onRightEdge = false;
    const Segment* seg = s_->segment ();
    Track* t = s_->track ();
    if (!seg || !t)
        return -1;
    int found = -1;
    for (size_t i : notesInSegment (*t, *seg))
    {
        CRect r = noteRect (t->notes[i]);
        CRect hit = r;
        hit.right += 2;
        if (hit.pointInside (p))
        {
            found = static_cast<int> (i);
            onRightEdge = p.x >= r.right - std::min (8.0, r.getWidth () * 0.4);
        }
    }
    return found;
}

bool PianoRollView::isSelected (size_t index) const
{
    return std::find (s_->selectedNotes.begin (), s_->selectedNotes.end (), index) != s_->selectedNotes.end ();
}

void PianoRollView::select (size_t index, bool add)
{
    if (!add)
    {
        s_->selectedNotes.clear ();
        s_->selectedNotes.push_back (index);
        return;
    }
    auto it = std::find (s_->selectedNotes.begin (), s_->selectedNotes.end (), index);
    if (it == s_->selectedNotes.end ())
        s_->selectedNotes.push_back (index);
    else
        s_->selectedNotes.erase (it);
}

void PianoRollView::commit ()
{
    s_->touch ();
    if (onEdit_)
        onEdit_ ();
    invalid ();
}

void PianoRollView::deleteSelection ()
{
    Track* t = s_->track ();
    if (!t || s_->selectedNotes.empty ())
        return;
    std::vector<size_t> sel = s_->selectedNotes;
    std::sort (sel.begin (), sel.end (), std::greater<size_t> ());
    for (size_t i : sel)
        if (i < t->notes.size ())
            t->notes.erase (t->notes.begin () + static_cast<std::ptrdiff_t> (i));
    s_->selectedNotes.clear ();
    commit ();
}

void PianoRollView::nudgeSelection (int64_t dTicks, int dPitch)
{
    Track* t = s_->track ();
    const Segment* seg = s_->segment ();
    if (!t || !seg || s_->selectedNotes.empty ())
        return;
    for (size_t i : s_->selectedNotes)
    {
        if (i >= t->notes.size ())
            continue;
        Note& n = t->notes[i];
        n.pitch = std::max (0, std::min (127, n.pitch + dPitch));
        int64_t ns = n.start + dTicks;
        if (ns >= seg->startTick && ns < seg->endTick)
            n.start = ns;
    }
    commit ();
}

void PianoRollView::duplicateSelection ()
{
    Track* t = s_->track ();
    const Segment* seg = s_->segment ();
    if (!t || !seg || s_->selectedNotes.empty ())
        return;
    int64_t minStart = INT64_MAX, maxEnd = 0;
    for (size_t i : s_->selectedNotes)
        if (i < t->notes.size ())
        {
            minStart = std::min (minStart, t->notes[i].start);
            maxEnd = std::max (maxEnd, t->notes[i].end ());
        }
    if (minStart == INT64_MAX)
        return;
    int64_t shift = snap (maxEnd) - minStart;
    if (shift <= 0)
        shift = std::max<int64_t> (1, s_->snapTicks);
    std::vector<size_t> fresh;
    for (size_t i : s_->selectedNotes)
    {
        if (i >= t->notes.size ())
            continue;
        Note n = t->notes[i];
        n.start += shift;
        if (n.start >= seg->endTick)
            continue;
        t->notes.push_back (n);
        fresh.push_back (t->notes.size () - 1);
    }
    s_->selectedNotes = fresh;
    commit ();
}

void PianoRollView::selectAll ()
{
    Track* t = s_->track ();
    const Segment* seg = s_->segment ();
    if (!t || !seg)
        return;
    s_->selectedNotes = notesInSegment (*t, *seg);
    invalid ();
}

void PianoRollView::clampView ()
{
    CRect g = gridRect ();
    int visible = static_cast<int> (g.getHeight () / keyH_);
    topPitch_ = std::max (std::min (127, topPitch_), std::min (127, visible - 1));
}

void PianoRollView::scrollToContent ()
{
    Track* t = s_->track ();
    const Segment* seg = s_->segment ();
    int lo = 127, hi = 0;
    if (t && seg)
        for (size_t i : notesInSegment (*t, *seg))
        {
            lo = std::min (lo, t->notes[i].pitch);
            hi = std::max (hi, t->notes[i].pitch);
        }
    if (lo > hi)
    {
        lo = t && t->isDrum ? 35 : 48;
        hi = t && t->isDrum ? 59 : 72;
    }
    int visible = static_cast<int> (gridRect ().getHeight () / keyH_);
    int center = (lo + hi) / 2;
    topPitch_ = center + visible / 2;
    clampView ();
    invalid ();
}

void PianoRollView::segmentChanged ()
{
    s_->selectedNotes.clear ();
    drag_ = Drag::None;
    scrollToContent ();
}

void PianoRollView::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    CRect g = gridRect ();
    const Segment* seg = s_->segment ();
    Track* track = s_->track ();

    ctx->setFillColor (theme::kVoid);
    ctx->drawRect (r, kDrawFilled);

    if (!seg || !track)
    {
        theme::drawGlowText (ctx, "NO SEGMENT  -  press GENERATE or OPEN", r, theme::labelFont (), theme::kTextDim,
                             theme::kMagentaDim);
        setDirty (false);
        return;
    }

    ctx->saveGlobalState ();
    ctx->setClipRect (g);

    // rows
    for (int pitch = topPitch_; pitch >= 0; --pitch)
    {
        double y = pitchToY (pitch);
        if (y > g.bottom)
            break;
        CRect row (g.left, y, g.right, y + keyH_);
        bool black = isBlackKey (pitch);
        ctx->setFillColor (black ? CColor (10, 7, 18) : CColor (16, 10, 28));
        ctx->drawRect (row, kDrawFilled);
        if (pitch % 12 == 0)
        {
            ctx->setFrameColor (theme::kGridStrong);
            ctx->setLineWidth (1.0);
            ctx->drawLine (CPoint (g.left, y + keyH_), CPoint (g.right, y + keyH_));
        }
    }

    // vertical grid: snap, beats, bars
    double beatsPerBar = 4.0;
    if (seg->sectionIndex >= 0 && seg->sectionIndex < static_cast<int> (s_->comp.sections.size ()))
        beatsPerBar = s_->comp.sections[static_cast<size_t> (seg->sectionIndex)].beatsPerBar;
    else
        beatsPerBar = s_->comp.timeSigNum * 4.0 / std::max (1, s_->comp.timeSigDen);
    int64_t ticksPerBar = beatsToTicks (beatsPerBar);
    int64_t snapT = std::max<int64_t> (1, s_->snapTicks);
    ctx->setLineWidth (1.0);
    for (int64_t t = seg->startTick; t < seg->endTick; t += snapT)
    {
        double x = std::floor (tickToX (t)) + 0.5;
        int64_t rel = t - seg->startTick;
        if (rel % ticksPerBar == 0)
            ctx->setFrameColor (theme::kGridStrong);
        else if (rel % kPPQ == 0)
            ctx->setFrameColor (theme::kGrid);
        else
            ctx->setFrameColor (CColor (30, 18, 50));
        ctx->drawLine (CPoint (x, g.top), CPoint (x, g.bottom));
    }

    // ghost notes of the other tracks
    for (size_t ti = 0; ti < s_->comp.tracks.size (); ++ti)
    {
        if (static_cast<int> (ti) == s_->selectedTrack)
            continue;
        const Track& other = s_->comp.tracks[ti];
        CColor c = theme::trackColor (ti);
        c.alpha = 55;
        ctx->setFillColor (c);
        for (size_t i : notesInSegment (other, *seg))
        {
            CRect nr = noteRect (other.notes[i]);
            if (nr.bottom < g.top || nr.top > g.bottom)
                continue;
            ctx->drawRect (nr, kDrawFilled);
        }
    }

    // the edited track
    CColor base = theme::trackColor (static_cast<size_t> (s_->selectedTrack));
    for (size_t i : notesInSegment (*track, *seg))
    {
        const Note& n = track->notes[i];
        CRect nr = noteRect (n);
        if (nr.bottom < g.top || nr.top > g.bottom)
            continue;
        double vel = std::max (0.15, std::min (1.0, n.velocity > 1.0 ? n.velocity / 127.0 : n.velocity));
        CColor fill = base;
        fill.red = static_cast<uint8_t> (fill.red * (0.45 + 0.55 * vel));
        fill.green = static_cast<uint8_t> (fill.green * (0.45 + 0.55 * vel));
        fill.blue = static_cast<uint8_t> (fill.blue * (0.45 + 0.55 * vel));
        bool sel = isSelected (i);
        if (sel)
        {
            CRect glow = nr;
            glow.extend (2, 2);
            theme::fillRoundRect (ctx, glow, 3, CColor (255, 255, 255, 60));
        }
        theme::fillRoundRect (ctx, nr, 2, fill);
        CRect shine = nr;
        shine.bottom = shine.top + std::max (1.0, nr.getHeight () * 0.35);
        theme::fillRoundRect (ctx, shine, 2, CColor (255, 255, 255, 60));
        theme::strokeRoundRect (ctx, nr, 2, sel ? theme::kChromeHi : (static_cast<int> (i) == hoverNote_ ? theme::kCyan : CColor (0, 0, 0, 120)), 1.0);
        if (nr.getWidth () > 30 && keyH_ >= 11)
        {
            ctx->setFont (theme::tinyFont ());
            ctx->setFontColor (CColor (0, 0, 0, 170));
            CRect tr = nr;
            tr.left += 3;
            ctx->drawString (track->isDrum ? (drumName (n.pitch) ? drumName (n.pitch) : "") : pitchName (n.pitch).c_str (), tr, kLeftText);
        }
    }

    // marquee
    if (drag_ == Drag::Marquee)
    {
        CRect m = marquee_;
        m.normalize ();
        theme::fillRoundRect (ctx, m, 2, CColor (57, 240, 255, 40));
        theme::strokeRoundRect (ctx, m, 2, theme::kCyan, 1.0);
    }

    // playhead
    int64_t ph = s_->transport.tick ();
    if ((s_->transport.playing || s_->transport.previewing) && ph >= seg->startTick && ph < seg->endTick)
    {
        double x = tickToX (ph);
        ctx->setFrameColor (theme::kLime);
        ctx->setLineWidth (2.0);
        ctx->drawLine (CPoint (x, g.top), CPoint (x, g.bottom));
    }

    ctx->restoreGlobalState ();

    // keyboard
    CRect kb (r.left, g.top, g.left, r.bottom);
    ctx->saveGlobalState ();
    ctx->setClipRect (kb);
    for (int pitch = topPitch_; pitch >= 0; --pitch)
    {
        double y = pitchToY (pitch);
        if (y > kb.bottom)
            break;
        CRect key (kb.left, y, kb.right - 2, y + keyH_);
        bool black = isBlackKey (pitch);
        ctx->setFillColor (black ? theme::kBlackKey : theme::kWhiteKey);
        ctx->drawRect (key, kDrawFilled);
        ctx->setFrameColor (CColor (0, 0, 0, 160));
        ctx->setLineWidth (1.0);
        ctx->drawLine (CPoint (key.left, key.bottom), CPoint (key.right, key.bottom));
        const char* dn = track->isDrum ? drumName (pitch) : nullptr;
        if ((dn || (!track->isDrum && pitch % 12 == 0)) && keyH_ >= 9)
        {
            ctx->setFont (theme::tinyFont ());
            ctx->setFontColor (dn ? theme::kCyan : theme::kTextDim);
            CRect tr = key;
            tr.left += 4;
            ctx->drawString (dn ? dn : pitchName (pitch).c_str (), tr, kLeftText);
        }
    }
    ctx->restoreGlobalState ();

    // ruler
    CRect ruler (r.left, r.top, r.right, g.top);
    ctx->setFillColor (theme::kPanel);
    ctx->drawRect (ruler, kDrawFilled);
    ctx->setFont (theme::tinyFont ());
    int bar = seg->startBar;
    for (int64_t t = seg->startTick; t < seg->endTick; t += ticksPerBar, ++bar)
    {
        double x = tickToX (t);
        ctx->setFrameColor (theme::kGridStrong);
        ctx->drawLine (CPoint (std::floor (x) + 0.5, ruler.top), CPoint (std::floor (x) + 0.5, ruler.bottom));
        ctx->setFontColor (theme::kText);
        CRect tr (x + 4, ruler.top, x + 60, ruler.bottom);
        ctx->drawString (std::to_string (bar).c_str (), tr, kLeftText);
        for (int b = 1; b < static_cast<int> (beatsPerBar); ++b)
        {
            double bx = std::floor (tickToX (t + b * kPPQ)) + 0.5;
            ctx->setFrameColor (theme::kGrid);
            ctx->drawLine (CPoint (bx, ruler.bottom - 6), CPoint (bx, ruler.bottom));
        }
    }
    // segment name in the ruler corner
    CRect corner (r.left, r.top, g.left, g.top);
    ctx->setFillColor (theme::kPanelLight);
    ctx->drawRect (corner, kDrawFilled);
    ctx->setFontColor (theme::kMagenta);
    ctx->drawString ("\xE2\x9C\xA6", corner, kCenterText);

    theme::drawScanlines (ctx, g);
    ctx->setFrameColor (theme::kGrid);
    ctx->setLineWidth (1.0);
    ctx->drawRect (r, kDrawStroked);
    setDirty (false);
}

void PianoRollView::onMouseDownEvent (MouseDownEvent& event)
{
    const Segment* seg = s_->segment ();
    Track* track = s_->track ();
    if (!seg || !track)
        return;
    if (auto* frame = getFrame ())
        frame->setFocusView (this);
    CPoint p = event.mousePosition;
    if (!gridRect ().pointInside (p))
    {
        event.consumed = true;
        return;
    }
    bool edge = false;
    int hit = hitNote (p, edge);
    bool shift = event.modifiers.has (ModifierKey::Shift);
    bool ctrl = event.modifiers.has (ModifierKey::Control);
    bool alt = event.modifiers.has (ModifierKey::Alt);

    if (event.buttonState.isRight ())
    {
        if (hit >= 0)
        {
            if (!isSelected (static_cast<size_t> (hit)))
                select (static_cast<size_t> (hit), false);
            deleteSelection ();
        }
        event.consumed = true;
        return;
    }
    if (!event.buttonState.isLeft ())
        return;

    dragStart_ = p;
    changedDuringDrag_ = false;
    if (hit >= 0)
    {
        size_t idx = static_cast<size_t> (hit);
        if (shift)
            select (idx, true);
        else if (!isSelected (idx))
            select (idx, false);
        dragNote_ = hit;
        drag_ = alt ? Drag::Velocity : (edge ? Drag::Resize : Drag::Move);
        dragIndices_ = s_->selectedNotes;
        dragOrig_.clear ();
        for (size_t i : dragIndices_)
            dragOrig_.push_back (track->notes[i]);
        lastLength_ = track->notes[idx].length;
        lastVelocity_ = track->notes[idx].velocity;
    }
    else if (ctrl)
    {
        drag_ = Drag::Marquee;
        marquee_ = CRect (p.x, p.y, p.x, p.y);
        if (!shift)
            s_->selectedNotes.clear ();
    }
    else
    {
        Note n;
        n.pitch = std::max (0, std::min (127, yToPitch (p.y)));
        n.start = snap (xToTick (p.x));
        if (n.start >= seg->endTick)
            n.start = seg->endTick - std::max<int64_t> (1, s_->snapTicks);
        if (n.start < seg->startTick)
            n.start = seg->startTick;
        n.length = std::min (lastLength_, seg->endTick - n.start);
        n.velocity = lastVelocity_;
        track->notes.push_back (n);
        dragNote_ = static_cast<int> (track->notes.size () - 1);
        s_->selectedNotes.clear ();
        s_->selectedNotes.push_back (static_cast<size_t> (dragNote_));
        dragIndices_ = s_->selectedNotes;
        dragOrig_ = {n};
        drag_ = Drag::Create;
        changedDuringDrag_ = true;
    }
    invalid ();
    event.consumed = true;
}

void PianoRollView::onMouseMoveEvent (MouseMoveEvent& event)
{
    const Segment* seg = s_->segment ();
    Track* track = s_->track ();
    CPoint p = event.mousePosition;
    if (!seg || !track)
        return;
    if (drag_ == Drag::None)
    {
        bool edge = false;
        int h = hitNote (p, edge);
        if (h != hoverNote_ || edge != hoverEdge_)
        {
            hoverNote_ = h;
            hoverEdge_ = edge;
            invalid ();
        }
        return;
    }
    if (!event.buttonState.isLeft ())
        return;
    double dx = p.x - dragStart_.x;
    double dy = p.y - dragStart_.y;
    int64_t dTicks = static_cast<int64_t> (std::llround (dx / gridRect ().getWidth () * seg->lengthTicks ()));
    int dPitch = -static_cast<int> (std::floor (dy / keyH_ + 0.5));
    int64_t snapT = std::max<int64_t> (1, s_->snapTicks);
    auto snapDelta = [&] (int64_t d) { return static_cast<int64_t> (std::llround (static_cast<double> (d) / snapT)) * snapT; };

    switch (drag_)
    {
        case Drag::Move:
            for (size_t k = 0; k < dragIndices_.size (); ++k)
            {
                Note& n = track->notes[dragIndices_[k]];
                const Note& o = dragOrig_[k];
                int64_t ns = o.start + snapDelta (dTicks);
                ns = std::max (seg->startTick, std::min (seg->endTick - 1, ns));
                n.start = ns;
                n.pitch = std::max (0, std::min (127, o.pitch + dPitch));
            }
            changedDuringDrag_ = true;
            break;
        case Drag::Resize:
        case Drag::Create:
            for (size_t k = 0; k < dragIndices_.size (); ++k)
            {
                Note& n = track->notes[dragIndices_[k]];
                const Note& o = dragOrig_[k];
                int64_t end = drag_ == Drag::Create ? snap (xToTick (p.x)) + snapT : o.end () + snapDelta (dTicks);
                end = std::max (n.start + snapT, std::min (seg->endTick, end));
                n.length = end - n.start;
                lastLength_ = n.length;
            }
            changedDuringDrag_ = true;
            break;
        case Drag::Velocity:
            for (size_t k = 0; k < dragIndices_.size (); ++k)
            {
                Note& n = track->notes[dragIndices_[k]];
                const Note& o = dragOrig_[k];
                double ov = o.velocity > 1.0 ? o.velocity / 127.0 : o.velocity;
                n.velocity = std::max (0.01, std::min (1.0, ov - dy / 100.0));
                lastVelocity_ = n.velocity;
            }
            changedDuringDrag_ = true;
            break;
        case Drag::Marquee:
        {
            marquee_ = CRect (dragStart_.x, dragStart_.y, p.x, p.y);
            CRect m = marquee_;
            m.normalize ();
            std::vector<size_t> sel;
            for (size_t i : notesInSegment (*track, *seg))
                if (m.rectOverlap (noteRect (track->notes[i])))
                    sel.push_back (i);
            s_->selectedNotes = sel;
            break;
        }
        default: break;
    }
    invalid ();
    event.consumed = true;
}

void PianoRollView::onMouseUpEvent (MouseUpEvent& event)
{
    if (drag_ == Drag::None)
        return;
    bool changed = changedDuringDrag_;
    drag_ = Drag::None;
    dragNote_ = -1;
    if (changed)
        commit ();
    else
        invalid ();
    event.consumed = true;
}

void PianoRollView::onMouseCancelEvent (MouseCancelEvent& event)
{
    Track* track = s_->track ();
    if (drag_ != Drag::None && track)
    {
        if (drag_ == Drag::Create && dragNote_ >= 0 && static_cast<size_t> (dragNote_) < track->notes.size ())
            track->notes.erase (track->notes.begin () + dragNote_);
        else
            for (size_t k = 0; k < dragIndices_.size () && k < dragOrig_.size (); ++k)
                if (dragIndices_[k] < track->notes.size ())
                    track->notes[dragIndices_[k]] = dragOrig_[k];
        s_->selectedNotes.clear ();
    }
    drag_ = Drag::None;
    invalid ();
    event.consumed = true;
}

void PianoRollView::onMouseWheelEvent (MouseWheelEvent& event)
{
    double d = event.deltaY;
    if (event.flags & MouseWheelEvent::DirectionInvertedFromDevice)
        d = -d;
    if (event.modifiers.has (ModifierKey::Control))
    {
        keyH_ = std::max (6.0, std::min (28.0, keyH_ + (d > 0 ? 1.0 : -1.0)));
    }
    else
        topPitch_ += d > 0 ? 3 : -3;
    clampView ();
    invalid ();
    event.consumed = true;
}

void PianoRollView::onMouseExitEvent (MouseExitEvent& event)
{
    if (hoverNote_ != -1)
    {
        hoverNote_ = -1;
        invalid ();
    }
    event.consumed = true;
}

void PianoRollView::onKeyboardEvent (KeyboardEvent& event)
{
    if (event.type != EventType::KeyDown)
        return;
    bool shift = event.modifiers.has (ModifierKey::Shift);
    bool ctrl = event.modifiers.has (ModifierKey::Control);
    int64_t snapT = std::max<int64_t> (1, s_->snapTicks);
    switch (event.virt)
    {
        case VirtualKey::Delete:
        case VirtualKey::Back: deleteSelection (); event.consumed = true; return;
        case VirtualKey::Up: nudgeSelection (0, shift ? 12 : 1); event.consumed = true; return;
        case VirtualKey::Down: nudgeSelection (0, shift ? -12 : -1); event.consumed = true; return;
        case VirtualKey::Left: nudgeSelection (-snapT, 0); event.consumed = true; return;
        case VirtualKey::Right: nudgeSelection (snapT, 0); event.consumed = true; return;
        case VirtualKey::Escape:
            s_->selectedNotes.clear ();
            invalid ();
            event.consumed = true;
            return;
        default: break;
    }
    if (ctrl && (event.character == 'a' || event.character == 'A'))
    {
        selectAll ();
        event.consumed = true;
    }
    else if (ctrl && (event.character == 'd' || event.character == 'D'))
    {
        duplicateSelection ();
        event.consumed = true;
    }
}

} // namespace midiagent
