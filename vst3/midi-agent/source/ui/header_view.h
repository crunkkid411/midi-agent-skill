// The header band: the wordmark, the transport readout and the song facts.
// The buttons on it are added by the editor.
#pragma once

#include "source/session.h"

#include "vstgui/lib/cview.h"

namespace midiagent {

class HeaderView : public VSTGUI::CView
{
public:
    HeaderView (const VSTGUI::CRect& size, Session* session);
    void draw (VSTGUI::CDrawContext* ctx) override;

    CLASS_METHODS (HeaderView, CView)
private:
    Session* s_;
};

} // namespace midiagent
