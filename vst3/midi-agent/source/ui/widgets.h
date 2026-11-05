// Small custom controls in the theme: neon pill buttons, drag chips and labels.
#pragma once

#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cdrawdefs.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/events.h"

#include <functional>
#include <string>

namespace midiagent {

/// A pill button. Momentary by default; setToggle (true) makes it a 0/1 switch.
class NeonButton : public VSTGUI::CControl
{
public:
    NeonButton (const VSTGUI::CRect& size, const std::string& title, std::function<void (NeonButton&)> onClick = {});

    void setTitle (const std::string& t);
    const std::string& getTitle () const { return title_; }
    void setToggle (bool toggle) { toggle_ = toggle; }
    void setHot (bool hot);
    bool isHot () const { return hot_; }
    void setOnClick (std::function<void (NeonButton&)> f) { onClick_ = std::move (f); }
    void setEnabledLook (bool enabled);

    void draw (VSTGUI::CDrawContext* ctx) override;
    void onMouseDownEvent (VSTGUI::MouseDownEvent& event) override;
    void onMouseUpEvent (VSTGUI::MouseUpEvent& event) override;
    void onMouseEnterEvent (VSTGUI::MouseEnterEvent& event) override;
    void onMouseExitEvent (VSTGUI::MouseExitEvent& event) override;
    void onMouseCancelEvent (VSTGUI::MouseCancelEvent& event) override;

    CLASS_METHODS (NeonButton, CControl)
private:
    std::string title_;
    std::function<void (NeonButton&)> onClick_;
    bool toggle_ {false};
    bool hot_ {false};
    bool pressed_ {false};
    bool hover_ {false};
    bool enabled_ {true};
};

/// Press and drag it onto a DAW track: `prepare` writes the file and returns
/// its path (empty on failure), then a native file drag starts.
class DragChip : public VSTGUI::CView
{
public:
    DragChip (const VSTGUI::CRect& size, const std::string& title, std::function<std::string ()> prepare);

    void setTitle (const std::string& t);
    void draw (VSTGUI::CDrawContext* ctx) override;
    void onMouseDownEvent (VSTGUI::MouseDownEvent& event) override;
    void onMouseMoveEvent (VSTGUI::MouseMoveEvent& event) override;
    void onMouseUpEvent (VSTGUI::MouseUpEvent& event) override;
    void onMouseEnterEvent (VSTGUI::MouseEnterEvent& event) override;
    void onMouseExitEvent (VSTGUI::MouseExitEvent& event) override;

    CLASS_METHODS (DragChip, CView)
private:
    std::string title_;
    std::function<std::string ()> prepare_;
    bool armed_ {false};
    bool hover_ {false};
    VSTGUI::CPoint downPos_;
};

/// Static text in the theme fonts.
class NeonLabel : public VSTGUI::CView
{
public:
    NeonLabel (const VSTGUI::CRect& size, const std::string& text, VSTGUI::CFontRef font, const VSTGUI::CColor& color,
               VSTGUI::CHoriTxtAlign align = VSTGUI::kLeftText);
    void setText (const std::string& t);
    const std::string& getText () const { return text_; }
    void setColor (const VSTGUI::CColor& c);
    void setGlow (bool glow) { glow_ = glow; }
    void draw (VSTGUI::CDrawContext* ctx) override;

    CLASS_METHODS (NeonLabel, CView)
private:
    std::string text_;
    VSTGUI::CFontRef font_;
    VSTGUI::CColor color_;
    VSTGUI::CHoriTxtAlign align_;
    bool glow_ {false};
};

/// The log / status panel: the last lines of the session log.
class LogView : public VSTGUI::CView
{
public:
    LogView (const VSTGUI::CRect& size, const std::vector<std::string>* lines);
    void draw (VSTGUI::CDrawContext* ctx) override;

    CLASS_METHODS (LogView, CView)
private:
    const std::vector<std::string>* lines_;
};

} // namespace midiagent
