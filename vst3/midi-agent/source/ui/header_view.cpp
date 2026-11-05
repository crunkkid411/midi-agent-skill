#include "source/ui/header_view.h"
#include "source/ui/theme.h"

#include <cmath>
#include <cstdio>

namespace midiagent {

using namespace VSTGUI;

HeaderView::HeaderView (const CRect& size, Session* session) : CView (size), s_ (session)
{
    setMouseEnabled (false);
}

void HeaderView::draw (CDrawContext* ctx)
{
    CRect r = getViewSize ();
    theme::fillGradient (ctx, r, 0, CColor (28, 14, 48), theme::kVoid);
    // horizon lines, the y2k grid floor
    ctx->setLineWidth (1.0);
    for (int i = 0; i < 5; ++i)
    {
        double y = r.bottom - 2 - i * i * 2.2;
        CColor c = theme::kMagentaDim;
        c.alpha = static_cast<uint8_t> (120 - i * 20);
        ctx->setFrameColor (c);
        ctx->drawLine (CPoint (r.left, y), CPoint (r.right, y));
    }

    // wordmark
    CRect wm (r.left + 18, r.top + 8, r.left + 300, r.top + 44);
    theme::drawGlowText (ctx, "MIDI AGENT", wm, theme::titleFont (), theme::kChromeHi, theme::kMagenta, kLeftText);
    CRect sub (r.left + 20, r.top + 42, r.left + 300, r.top + 56);
    ctx->setFont (theme::tinyFont ());
    ctx->setFontColor (theme::kCyan);
    ctx->drawString ("\xE2\x98\x85 scenecore edition \xE2\x98\x85  midi-agent-skill inside cubase", sub, kLeftText);
    theme::drawSparkle (ctx, CPoint (r.left + 236, r.top + 14), 7, theme::kCyan);
    theme::drawSparkle (ctx, CPoint (r.left + 250, r.top + 30), 4, theme::kHotPink);

    // transport readout: bar.beat.tick like Cubase
    const TransportInfo& t = s_->transport;
    double qpb = s_->comp.timeSigNum * 4.0 / std::max (1, s_->comp.timeSigDen);
    double ppq = std::max (0.0, t.ppq);
    int bar = static_cast<int> (std::floor (ppq / qpb)) + 1;
    double inBar = ppq - (bar - 1) * qpb;
    int beat = static_cast<int> (std::floor (inBar)) + 1;
    int tick = static_cast<int> ((inBar - (beat - 1)) * kPPQ);
    char buf[96];
    std::snprintf (buf, sizeof (buf), "%03d.%02d.%03d", bar, beat, tick);
    CRect ro (r.left + 330, r.top + 10, r.left + 470, r.top + 40);
    theme::drawChromePanel (ctx, ro, 6);
    CRect inner = ro;
    inner.inset (4, 4);
    ctx->setFillColor (CColor (6, 4, 12));
    ctx->drawRect (inner, kDrawFilled);
    ctx->setFont (theme::monoFont ());
    ctx->setFontColor (t.playing ? theme::kLime : theme::kHotPink);
    ctx->drawString (buf, inner, kCenterText);
    CRect lab (ro.left, ro.bottom + 2, ro.right, ro.bottom + 14);
    ctx->setFont (theme::tinyFont ());
    ctx->setFontColor (theme::kTextDim);
    ctx->drawString (t.playing ? (s_->loopOn ? "PLAYING - LOOP" : "PLAYING") : "STOPPED", lab, kCenterText);

    // song facts
    std::snprintf (buf, sizeof (buf), "%.0f BPM", t.playing ? t.tempo : s_->comp.bpm);
    CRect bpmR (r.left + 480, r.top + 10, r.left + 560, r.top + 40);
    theme::drawChromePanel (ctx, bpmR, 6);
    CRect bi = bpmR;
    bi.inset (4, 4);
    ctx->setFillColor (CColor (6, 4, 12));
    ctx->drawRect (bi, kDrawFilled);
    ctx->setFont (theme::monoFont ());
    ctx->setFontColor (theme::kCyan);
    ctx->drawString (buf, bi, kCenterText);

    std::string facts = s_->comp.title;
    if (!s_->comp.key.empty ())
        facts += "  \xE2\x80\xA2  " + s_->comp.key;
    facts += "  \xE2\x80\xA2  " + std::to_string (s_->comp.barCount ()) + " bars  \xE2\x80\xA2  " +
             std::to_string (s_->comp.tracks.size ()) + " tracks";
    if (s_->dirty)
        facts += "  \xE2\x80\xA2  unsaved";
    std::snprintf (buf, sizeof (buf), "  \xE2\x80\xA2  %d voices", t.voices);
    facts += buf;
    CRect fr (r.right - 420, r.bottom - 22, r.right - 16, r.bottom - 8);
    ctx->setFont (theme::tinyFont ());
    ctx->setFontColor (theme::kTextDim);
    ctx->drawString (facts.c_str (), fr, kRightText);

    if (s_->busy)
    {
        CRect busy (r.left + 330, r.bottom - 22, r.left + 470, r.bottom - 8);
        ctx->setFontColor (theme::kLime);
        ctx->drawString (("\xE2\x9C\xA6 " + s_->busyLabel).c_str (), busy, kLeftText);
    }
    setDirty (false);
}

} // namespace midiagent
