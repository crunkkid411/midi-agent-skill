#include "source/ui/theme.h"

#include "vstgui/lib/cgradient.h"
#include "vstgui/lib/cgraphicspath.h"

#include <cmath>

namespace midiagent {
namespace theme {

using namespace VSTGUI;

CColor trackColor (size_t trackIndex)
{
    static const CColor colors[] = {
        CColor (255, 43, 214), CColor (57, 240, 255), CColor (182, 255, 59), CColor (255, 170, 60),
        CColor (150, 92, 255), CColor (255, 110, 199), CColor (90, 255, 200), CColor (255, 236, 90),
    };
    return colors[trackIndex % (sizeof (colors) / sizeof (colors[0]))];
}

namespace {
CFontRef makeFont (const char* name, double size, int style)
{
    // Fonts are shared and never freed: the editor can open and close many times.
    auto* f = new CFontDesc (name, size, style);
    return f;
}
} // namespace

CFontRef titleFont ()
{
    static CFontRef f = makeFont ("Verdana", 24, kBoldFace);
    return f;
}

CFontRef labelFont ()
{
    static CFontRef f = makeFont ("Verdana", 10, kBoldFace);
    return f;
}

CFontRef bodyFont ()
{
    static CFontRef f = makeFont ("Verdana", 11, kNormalFace);
    return f;
}

CFontRef monoFont ()
{
    static CFontRef f = makeFont ("Courier New", 12, kBoldFace);
    return f;
}

CFontRef tinyFont ()
{
    static CFontRef f = makeFont ("Verdana", 8, kNormalFace);
    return f;
}

void fillRoundRect (CDrawContext* ctx, const CRect& r, double radius, const CColor& color)
{
    if (auto path = owned (ctx->createGraphicsPath ()))
    {
        path->addRoundRect (r, radius);
        ctx->setFillColor (color);
        ctx->drawGraphicsPath (path, CDrawContext::kPathFilled);
    }
}

void strokeRoundRect (CDrawContext* ctx, const CRect& r, double radius, const CColor& color, double width)
{
    if (auto path = owned (ctx->createGraphicsPath ()))
    {
        path->addRoundRect (r, radius);
        ctx->setFrameColor (color);
        ctx->setLineWidth (width);
        ctx->setLineStyle (kLineSolid);
        ctx->drawGraphicsPath (path, CDrawContext::kPathStroked);
    }
}

void fillGradient (CDrawContext* ctx, const CRect& r, double radius, const CColor& top, const CColor& bottom)
{
    auto path = owned (ctx->createGraphicsPath ());
    auto gradient = owned (CGradient::create (0.0, 1.0, top, bottom));
    if (!path || !gradient)
        return;
    path->addRoundRect (r, radius);
    ctx->fillLinearGradient (path, *gradient, r.getTopLeft (), r.getBottomLeft ());
}

void drawChromePanel (CDrawContext* ctx, const CRect& r, double radius)
{
    fillGradient (ctx, r, radius, kChromeHi, kChromeLo);
    CRect inner = r;
    inner.inset (1.5, 1.5);
    fillGradient (ctx, inner, radius - 1, kChromeMid, kChromeLo);
    CRect shine = inner;
    shine.bottom = shine.top + inner.getHeight () * 0.45;
    fillGradient (ctx, shine, radius - 1, CColor (255, 255, 255, 110), CColor (255, 255, 255, 0));
    strokeRoundRect (ctx, r, radius, CColor (255, 255, 255, 90), 1.0);
}

void drawNeonButton (CDrawContext* ctx, const CRect& r, bool hot, bool pressed, bool hover)
{
    const double radius = r.getHeight () * 0.5;
    CRect body = r;
    if (pressed)
        body.offset (0, 1);
    // glow
    if (hot || hover)
    {
        CRect glow = body;
        glow.extend (2, 2);
        CColor g = hot ? kMagenta : kCyan;
        g.alpha = hover ? 90 : 60;
        fillRoundRect (ctx, glow, radius + 2, g);
    }
    CColor top = hot ? CColor (255, 120, 230) : CColor (60, 40, 96);
    CColor bottom = hot ? CColor (160, 20, 130) : CColor (24, 14, 42);
    if (pressed)
        std::swap (top, bottom);
    fillGradient (ctx, body, radius, top, bottom);
    CRect shine = body;
    shine.inset (2, 2);
    shine.bottom = shine.top + body.getHeight () * 0.42;
    fillGradient (ctx, shine, radius, CColor (255, 255, 255, hot ? 120 : 70), CColor (255, 255, 255, 0));
    strokeRoundRect (ctx, body, radius, hot ? kHotPink : (hover ? kCyan : kChromeMid), 1.0);
}

void drawGlowText (CDrawContext* ctx, const std::string& text, const CRect& r, CFontRef font, const CColor& color,
                   const CColor& glow, CHoriTxtAlign align)
{
    ctx->setFont (font);
    CColor g = glow;
    g.alpha = 70;
    ctx->setFontColor (g);
    const double offsets[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (const auto& o : offsets)
    {
        CRect rr = r;
        rr.offset (o[0], o[1]);
        ctx->drawString (text.c_str (), rr, align);
    }
    ctx->setFontColor (color);
    ctx->drawString (text.c_str (), r, align);
}

void drawSparkle (CDrawContext* ctx, const CPoint& c, double size, const CColor& color)
{
    auto path = owned (ctx->createGraphicsPath ());
    if (!path)
        return;
    const double s = size, w = size * 0.22;
    path->beginSubpath (CPoint (c.x, c.y - s));
    path->addLine (CPoint (c.x + w, c.y - w));
    path->addLine (CPoint (c.x + s, c.y));
    path->addLine (CPoint (c.x + w, c.y + w));
    path->addLine (CPoint (c.x, c.y + s));
    path->addLine (CPoint (c.x - w, c.y + w));
    path->addLine (CPoint (c.x - s, c.y));
    path->addLine (CPoint (c.x - w, c.y - w));
    path->closeSubpath ();
    ctx->setFillColor (color);
    ctx->drawGraphicsPath (path, CDrawContext::kPathFilled);
}

void drawScanlines (CDrawContext* ctx, const CRect& r)
{
    ctx->setFrameColor (CColor (0, 0, 0, 34));
    ctx->setLineWidth (1.0);
    ctx->setLineStyle (kLineSolid);
    ctx->setDrawMode (kAliasing);
    for (double y = std::floor (r.top) + 0.5; y < r.bottom; y += 3.0)
        ctx->drawLine (CPoint (r.left, y), CPoint (r.right, y));
    ctx->setDrawMode (kAntiAliasing);
}

void drawCheckerStrip (CDrawContext* ctx, const CRect& r)
{
    const double cell = r.getHeight () / 2.0;
    int i = 0;
    for (double x = r.left; x < r.right; x += cell, ++i)
    {
        for (int row = 0; row < 2; ++row)
        {
            bool on = ((i + row) % 2) == 0;
            CRect c (x, r.top + row * cell, std::min (r.right, x + cell), r.top + (row + 1) * cell);
            ctx->setFillColor (on ? kMagentaDim : kPanel);
            ctx->drawRect (c, kDrawFilled);
        }
    }
}

} // namespace theme
} // namespace midiagent
