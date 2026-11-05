// The Y2K / scenecore look: deep violet black, hot magenta and cyan neon,
// chrome bevels, sparkle glyphs and a scanline overlay. Every colour and font
// the views use is defined here, so the look can change in one file.
// See docs/UI-STYLE.md.
#pragma once

#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/crect.h"

#include <string>

namespace midiagent {
namespace theme {

// --- palette --------------------------------------------------------------
const VSTGUI::CColor kVoid (7, 5, 13);             // window background
const VSTGUI::CColor kPanel (18, 10, 31);          // panels
const VSTGUI::CColor kPanelLight (31, 18, 52);     // raised panels
const VSTGUI::CColor kGrid (46, 28, 76);           // piano roll grid
const VSTGUI::CColor kGridStrong (84, 48, 128);    // bar lines
const VSTGUI::CColor kMagenta (255, 43, 214);      // primary neon
const VSTGUI::CColor kMagentaDim (140, 24, 118);
const VSTGUI::CColor kCyan (57, 240, 255);         // secondary neon
const VSTGUI::CColor kCyanDim (26, 110, 120);
const VSTGUI::CColor kLime (182, 255, 59);         // playhead / ok
const VSTGUI::CColor kHotPink (255, 110, 199);
const VSTGUI::CColor kViolet (150, 92, 255);
const VSTGUI::CColor kChromeHi (246, 246, 255);
const VSTGUI::CColor kChromeMid (170, 172, 190);
const VSTGUI::CColor kChromeLo (72, 74, 92);
const VSTGUI::CColor kText (236, 230, 255);
const VSTGUI::CColor kTextDim (150, 140, 180);
const VSTGUI::CColor kBlackKey (12, 8, 20);
const VSTGUI::CColor kWhiteKey (40, 28, 62);
const VSTGUI::CColor kDanger (255, 72, 72);

// Note colours per track index, cycling.
VSTGUI::CColor trackColor (size_t trackIndex);

// --- fonts ----------------------------------------------------------------
VSTGUI::CFontRef titleFont ();   // big chrome wordmark
VSTGUI::CFontRef labelFont ();   // bold small caps
VSTGUI::CFontRef bodyFont ();    // normal text
VSTGUI::CFontRef monoFont ();    // readouts
VSTGUI::CFontRef tinyFont ();

// --- drawing helpers --------------------------------------------------------
void fillRoundRect (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r, double radius, const VSTGUI::CColor& color);
void strokeRoundRect (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r, double radius, const VSTGUI::CColor& color,
                      double width = 1.0);
/// Vertical gradient fill.
void fillGradient (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r, double radius, const VSTGUI::CColor& top,
                   const VSTGUI::CColor& bottom);
/// Chrome bevel: light top, dark bottom, with a highlight line.
void drawChromePanel (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r, double radius);
/// Neon pill button body. `hot` draws the active (magenta) state.
void drawNeonButton (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r, bool hot, bool pressed, bool hover);
/// Text with a soft glow underneath.
void drawGlowText (VSTGUI::CDrawContext* ctx, const std::string& text, const VSTGUI::CRect& r,
                   VSTGUI::CFontRef font, const VSTGUI::CColor& color, const VSTGUI::CColor& glow,
                   VSTGUI::CHoriTxtAlign align = VSTGUI::kCenterText);
/// A four-point sparkle star.
void drawSparkle (VSTGUI::CDrawContext* ctx, const VSTGUI::CPoint& center, double size, const VSTGUI::CColor& color);
/// CRT scanlines over a rect (draw last).
void drawScanlines (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r);
/// Dotted "checker" strip used as a divider.
void drawCheckerStrip (VSTGUI::CDrawContext* ctx, const VSTGUI::CRect& r);

} // namespace theme
} // namespace midiagent
