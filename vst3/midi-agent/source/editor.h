// The plug-in window. Builds the views by hand (no .uidesc): header, segment
// strip, action bar, track list, piano roll, log and the generate panel.
#pragma once

#include "source/session.h"

#include "public.sdk/source/vst/vstguieditor.h"
#include "vstgui/lib/controls/icontrollistener.h"

#include <string>
#include <vector>

namespace VSTGUI {
class CTextEdit;
class COptionMenu;
class CSlider;
} // namespace VSTGUI

namespace midiagent {

class MidiAgentController;
class HeaderView;
class SegmentStripView;
class TrackListView;
class PianoRollView;
class NeonButton;
class NeonLabel;
class LogView;
class DragChip;

class MidiAgentEditor : public Steinberg::Vst::VSTGUIEditor, public VSTGUI::IControlListener
{
public:
    static constexpr int kWidth = 1240;
    static constexpr int kHeight = 800;

    explicit MidiAgentEditor (MidiAgentController* controller);

    bool PLUGIN_API open (void* parent, const VSTGUI::PlatformType& platformType) SMTG_OVERRIDE;
    void PLUGIN_API close () SMTG_OVERRIDE;
    VSTGUI::CMessageResult notify (VSTGUI::CBaseObject* sender, const char* message) SMTG_OVERRIDE;
    void valueChanged (VSTGUI::CControl* control) SMTG_OVERRIDE;

    // called by the controller
    void compositionReplaced ();
    void transportUpdated ();

private:
    enum Tag
    {
        kTagGenreA = 1, kTagGenreB, kTagGenreC, kTagWeights, kTagBars, kTagBpm, kTagSeed, kTagKey, kTagTitle,
        kTagPython, kTagSkillRoot, kTagSnap, kTagGain,
    };

    void buildHeader ();
    void buildActionBar ();
    void buildGeneratePanel ();
    void refreshGenreMenus ();
    void refreshSettingsText ();
    void refreshAll ();
    void selectSegment (int index);
    void selectTrack (int index);
    void onNoteEdit ();

    // actions
    void actionNew ();
    void actionOpen ();
    void actionSave (bool saveAs);
    void actionGenerate ();
    void actionRegenerate (bool trackOnly);
    void actionClearSegment ();
    void actionRenderWav (bool wholeSong);
    void actionFindSkill ();
    void togglePreview ();
    std::string prepareSegmentMidi (bool trackOnly);
    std::string prepareSegmentWav ();
    std::string autoSavePath () const;
    bool writeCompositionForPython (std::string& path);
    void loadCompositionFile (const std::string& path, const std::string& why, bool keepSelection);
    void readGenerateControls ();
    void log (const std::string& line);

    MidiAgentController* ctrl_;
    Session* s_;

    HeaderView* header_ {nullptr};
    SegmentStripView* strip_ {nullptr};
    TrackListView* tracks_ {nullptr};
    PianoRollView* roll_ {nullptr};
    LogView* logView_ {nullptr};
    NeonButton* previewButton_ {nullptr};
    NeonButton* synthButton_ {nullptr};
    NeonButton* midiOutButton_ {nullptr};
    NeonButton* formButton_ {nullptr};
    NeonButton* generateButton_ {nullptr};
    NeonLabel* segLabel_ {nullptr};
    NeonLabel* settingsStatus_ {nullptr};
    VSTGUI::CSlider* gainSlider_ {nullptr};
    VSTGUI::COptionMenu* genreMenus_[3] {nullptr, nullptr, nullptr};
    VSTGUI::COptionMenu* snapMenu_ {nullptr};
    VSTGUI::CTextEdit* edits_[16] {}; // indexed by Tag
    std::vector<VSTGUI::CView*> dirtyEach_; // views redrawn on every tick while playing

    uint32_t pushedRevision_ {0};
    int64_t lastEditMillis_ {0};
    int tick_ {0};
};

} // namespace midiagent
