#include "source/editor.h"
#include "core/midifile.h"
#include "core/platform.h"
#include "core/renderer.h"
#include "source/controller.h"
#include "source/plugids.h"
#include "source/ui/header_view.h"
#include "source/ui/pianoroll_view.h"
#include "source/ui/segment_strip_view.h"
#include "source/ui/theme.h"
#include "source/ui/track_list_view.h"
#include "source/ui/widgets.h"

#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/lib/controls/cslider.h"
#include "vstgui/lib/controls/ctextedit.h"
#include "vstgui/lib/cvstguitimer.h"

#if SMTG_OS_LINUX
#include "public.sdk/source/vst/vstgui_linux_runloop_support.h"
#include "vstgui/lib/platform/linux/linuxfactory.h"
#include "vstgui/lib/platform/platform_x11.h"
#include "vstgui/lib/platform/platformfactory.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <random>
#include <sstream>

namespace midiagent {

using namespace VSTGUI;
using namespace Steinberg;

namespace {
constexpr double kHeaderH = 72;
constexpr double kStripH = 64;
constexpr double kActionH = 38;
constexpr double kTrackW = 250;
constexpr double kBottomH = 168;

const struct
{
    const char* label;
    int ticks;
} kSnaps[] = {{"1/4", 480}, {"1/8", 240}, {"1/16", 120}, {"1/32", 60}, {"1/8T", 160}, {"1/16T", 80}};

std::vector<std::string> splitList (const std::string& text)
{
    std::vector<std::string> out;
    std::string cur;
    for (char c : text)
    {
        if (c == ',' || c == ' ')
        {
            if (!cur.empty ())
                out.push_back (cur);
            cur.clear ();
        }
        else
            cur += c;
    }
    if (!cur.empty ())
        out.push_back (cur);
    return out;
}

void styleTextEdit (CTextEdit* e)
{
    e->setBackColor (CColor (6, 4, 12));
    e->setFrameColor (theme::kCyanDim);
    e->setFontColor (theme::kText);
    e->setFont (theme::bodyFont ());
    e->setHoriAlign (kLeftText);
    e->setTextInset (CPoint (6, 0));
}

void styleMenu (COptionMenu* m)
{
    m->setBackColor (CColor (6, 4, 12));
    m->setFrameColor (theme::kMagentaDim);
    m->setFontColor (theme::kHotPink);
    m->setFont (theme::labelFont ());
    m->setHoriAlign (kLeftText);
    m->setTextInset (CPoint (6, 0));
}
} // namespace

MidiAgentEditor::MidiAgentEditor (MidiAgentController* controller)
: VSTGUIEditor (controller), ctrl_ (controller), s_ (&controller->session ())
{
    ViewRect vr (0, 0, kWidth, kHeight);
    setRect (vr);
    setIdleRate (40);
}

bool PLUGIN_API MidiAgentEditor::open (void* parent, const PlatformType& platformType)
{
#if SMTG_OS_LINUX
    // On Linux VSTGUI needs the host's run loop (IPlugFrame implements Linux::IRunLoop).
    if (plugFrame)
        Steinberg::Linux::setupVSTGUIRunloop (plugFrame);
#endif
    frame = new CFrame (CRect (0, 0, kWidth, kHeight), this);
    frame->setBackgroundColor (theme::kVoid);

    const double W = kWidth, H = kHeight;
    header_ = new HeaderView (CRect (0, 0, W, kHeaderH), s_);
    frame->addView (header_);
    strip_ = new SegmentStripView (CRect (0, kHeaderH, W, kHeaderH + kStripH), s_, [this] (int i) { selectSegment (i); });
    frame->addView (strip_);

    const double top = kHeaderH + kStripH + kActionH;
    const double bottom = H - kBottomH;
    tracks_ = new TrackListView (CRect (0, top, kTrackW, bottom), s_, [this] (TrackListView::Action a, int i) {
        switch (a)
        {
            case TrackListView::Action::Select: selectTrack (i); break;
            case TrackListView::Action::Mute:
            {
                Track& t = s_->comp.tracks[static_cast<size_t> (i)];
                t.mute = !t.mute;
                ctrl_->sendMute (i, t.mute);
                s_->touch ();
                tracks_->invalid ();
                break;
            }
            case TrackListView::Action::Solo:
            {
                bool solo = !tracks_->isSolo (static_cast<size_t> (i));
                tracks_->setSolo (static_cast<size_t> (i), solo);
                ctrl_->sendSolo (i, solo);
                break;
            }
        }
    });
    frame->addView (tracks_);
    roll_ = new PianoRollView (CRect (kTrackW, top, W, bottom), s_, [this] { onNoteEdit (); });
    frame->addView (roll_);

    logView_ = new LogView (CRect (8, bottom + 8, 600, H - 8), &s_->log);
    frame->addView (logView_);

    buildHeader ();
    buildActionBar ();
    buildGeneratePanel ();

    dirtyEach_ = {header_, strip_, roll_};
#if SMTG_OS_LINUX
    // X11 frames need the run loop in their config, like VST3Editor does.
    X11::FrameConfig x11Config;
    if (auto* linuxFactory = getPlatformFactory ().asLinuxFactory ())
        x11Config.runLoop = linuxFactory->getRunLoop ();
    frame->open (parent, platformType, platformType == PlatformType::kWaylandSurfaceID ? nullptr : &x11Config);
#else
    frame->open (parent, platformType);
#endif
    refreshAll ();
    roll_->scrollToContent ();
    pushedRevision_ = s_->revision;
    return true;
}

void PLUGIN_API MidiAgentEditor::close ()
{
    if (frame)
    {
        frame->removeAll (true);
        frame->close ();
        frame = nullptr;
    }
    header_ = nullptr;
    strip_ = nullptr;
    tracks_ = nullptr;
    roll_ = nullptr;
    logView_ = nullptr;
    previewButton_ = synthButton_ = midiOutButton_ = formButton_ = generateButton_ = nullptr;
    segLabel_ = settingsStatus_ = nullptr;
    gainSlider_ = nullptr;
    snapMenu_ = nullptr;
    for (auto& m : genreMenus_)
        m = nullptr;
    for (auto& e : edits_)
        e = nullptr;
    dirtyEach_.clear ();
}

// --- building ---------------------------------------------------------------

void MidiAgentEditor::buildHeader ()
{
    double x = 580;
    auto button = [&] (const char* title, double w, std::function<void (NeonButton&)> f) {
        auto* b = new NeonButton (CRect (x, 12, x + w, 40), title, std::move (f));
        frame->addView (b);
        x += w + 6;
        return b;
    };
    button ("NEW", 60, [this] (NeonButton&) { actionNew (); });
    button ("OPEN", 66, [this] (NeonButton&) { actionOpen (); });
    button ("SAVE", 62, [this] (NeonButton&) { actionSave (false); });
    button ("SAVE AS", 80, [this] (NeonButton&) { actionSave (true); });
    button ("PANIC", 70, [this] (NeonButton&) {
        ctrl_->sendPanic ();
        if (previewButton_)
            previewButton_->setHot (false);
        log ("panic: all notes off");
    });

    x = 580;
    synthButton_ = new NeonButton (CRect (x, 44, x + 72, 66), "SYNTH", [this] (NeonButton& b) {
        ctrl_->setParam (kParamSynthOn, b.isHot () ? 1.0 : 0.0);
    });
    synthButton_->setToggle (true);
    frame->addView (synthButton_);
    x += 78;
    midiOutButton_ = new NeonButton (CRect (x, 44, x + 86, 66), "MIDI OUT", [this] (NeonButton& b) {
        ctrl_->setParam (kParamMidiOut, b.isHot () ? 1.0 : 0.0);
    });
    midiOutButton_->setToggle (true);
    frame->addView (midiOutButton_);
    x += 92;
    frame->addView (new NeonLabel (CRect (x, 44, x + 44, 66), "LEVEL", theme::tinyFont (), theme::kTextDim));
    x += 44;
    gainSlider_ = new CSlider (CRect (x, 48, x + 150, 62), this, kTagGain, 0, 140, nullptr, nullptr);
    gainSlider_->setDrawStyle (CSlider::kDrawFrame | CSlider::kDrawBack | CSlider::kDrawValue);
    gainSlider_->setFrameColor (theme::kChromeLo);
    gainSlider_->setBackColor (CColor (6, 4, 12));
    gainSlider_->setValueColor (theme::kMagenta);
    frame->addView (gainSlider_);
}

void MidiAgentEditor::buildActionBar ()
{
    const double y0 = kHeaderH + kStripH + 5, y1 = y0 + 28;
    double x = 8;
    segLabel_ = new NeonLabel (CRect (x, y0, x + 140, y1), "SEGMENT", theme::labelFont (), theme::kMagenta);
    segLabel_->setGlow (true);
    frame->addView (segLabel_);
    x += 146;

    auto button = [&] (const char* title, double w, std::function<void (NeonButton&)> f) {
        auto* b = new NeonButton (CRect (x, y0, x + w, y1), title, std::move (f));
        frame->addView (b);
        x += w + 6;
        return b;
    };
    auto chip = [&] (const char* title, double w, std::function<std::string ()> prep) {
        auto* c = new DragChip (CRect (x, y0, x + w, y1), title, std::move (prep));
        frame->addView (c);
        x += w + 6;
        return c;
    };

    previewButton_ = button ("PREVIEW LOOP", 100, [this] (NeonButton&) { togglePreview (); });
    previewButton_->setToggle (true);
    button ("REGEN SEG", 86, [this] (NeonButton&) { actionRegenerate (false); });
    button ("REGEN TRACK", 98, [this] (NeonButton&) { actionRegenerate (true); });
    button ("CLEAR", 58, [this] (NeonButton&) { actionClearSegment (); });

    frame->addView (new NeonLabel (CRect (x, y0, x + 36, y1), "SNAP", theme::tinyFont (), theme::kTextDim));
    x += 36;
    snapMenu_ = new COptionMenu (CRect (x, y0 + 2, x + 64, y1 - 2), this, kTagSnap);
    styleMenu (snapMenu_);
    for (const auto& sn : kSnaps)
        snapMenu_->addEntry (sn.label);
    frame->addView (snapMenu_);
    x += 76;

    chip ("DRAG MIDI \xC2\xB7 SEG", 118, [this] { return prepareSegmentMidi (false); });
    chip ("DRAG MIDI \xC2\xB7 TRACK", 126, [this] { return prepareSegmentMidi (true); });
    chip ("DRAG WAV \xC2\xB7 SEG", 116, [this] { return prepareSegmentWav (); });
    button ("RENDER WAV...", 104, [this] (NeonButton&) { actionRenderWav (false); });
    button ("RENDER SONG...", 108, [this] (NeonButton&) { actionRenderWav (true); });
}

void MidiAgentEditor::buildGeneratePanel ()
{
    const double x0 = 612, top = kHeight - kBottomH + 8;
    auto label = [&] (double x, double y, double w, const char* text) {
        frame->addView (new NeonLabel (CRect (x, y, x + w, y + 24), text, theme::tinyFont (), theme::kTextDim));
    };
    auto edit = [&] (Tag tag, double x, double y, double w, const char* placeholder) {
        auto* e = new CTextEdit (CRect (x, y + 2, x + w, y + 24), this, tag);
        styleTextEdit (e);
        e->setPlaceholderString (placeholder);
        frame->addView (e);
        edits_[tag] = e;
        return e;
    };

    double y = top;
    auto* title = new NeonLabel (CRect (x0, y, x0 + 90, y + 24), "GENERATE", theme::labelFont (), theme::kCyan);
    title->setGlow (true);
    frame->addView (title);
    for (int i = 0; i < 3; ++i)
    {
        double x = x0 + 96 + i * 170;
        genreMenus_[i] = new COptionMenu (CRect (x, y + 2, x + 164, y + 24), this, static_cast<int32_t> (kTagGenreA + i));
        styleMenu (genreMenus_[i]);
        frame->addView (genreMenus_[i]);
    }

    y += 32;
    label (x0, y, 60, "WEIGHTS");
    edit (kTagWeights, x0 + 62, y, 110, "0.6,0.4");
    label (x0 + 182, y, 40, "BARS");
    edit (kTagBars, x0 + 218, y, 48, "8");
    label (x0 + 276, y, 36, "BPM");
    edit (kTagBpm, x0 + 306, y, 52, "auto");
    label (x0 + 368, y, 40, "SEED");
    edit (kTagSeed, x0 + 404, y, 60, "random");
    label (x0 + 474, y, 32, "KEY");
    edit (kTagKey, x0 + 500, y, 70, "auto");
    formButton_ = new NeonButton (CRect (x0 + 576, y + 1, x0 + 622, y + 25), "FORM", [this] (NeonButton& b) {
        s_->request.useForm = b.isHot ();
    });
    formButton_->setToggle (true);
    frame->addView (formButton_);

    y += 32;
    label (x0, y, 40, "TITLE");
    edit (kTagTitle, x0 + 42, y, 170, "midi-agent");
    generateButton_ = new NeonButton (CRect (x0 + 222, y - 1, x0 + 360, y + 27), "\xE2\x9C\xA6 GENERATE \xE2\x9C\xA6",
                                      [this] (NeonButton&) { actionGenerate (); });
    generateButton_->setHot (true);
    frame->addView (generateButton_);
    settingsStatus_ = new NeonLabel (CRect (x0 + 370, y, kWidth - 8, y + 24), "", theme::tinyFont (), theme::kTextDim);
    frame->addView (settingsStatus_);

    y += 32;
    label (x0, y, 50, "PYTHON");
    edit (kTagPython, x0 + 52, y, 120, "python");
    label (x0 + 182, y, 62, "SKILL DIR");
    edit (kTagSkillRoot, x0 + 244, y, 290, "folder with SKILL.md");
    frame->addView (new NeonButton (CRect (x0 + 540, y + 1, x0 + 624, y + 25), "FIND...", [this] (NeonButton&) { actionFindSkill (); }));

    y += 32;
    frame->addView (new NeonLabel (CRect (x0, y, kWidth - 8, y + 20),
                                   "piano roll: click = note, drag = move, edge = length, alt = velocity, right click = delete, ctrl+drag = select, ctrl+A / ctrl+D, arrows",
                                   theme::tinyFont (), theme::kViolet));
}

// --- refresh ---------------------------------------------------------------

void MidiAgentEditor::refreshGenreMenus ()
{
    for (int i = 0; i < 3; ++i)
    {
        COptionMenu* m = genreMenus_[i];
        if (!m)
            continue;
        m->removeAllEntry ();
        m->addEntry ("(none)");
        for (const auto& g : s_->genres)
            m->addEntry (g.c_str ());
        int current = 0;
        if (static_cast<size_t> (i) < s_->request.genres.size ())
            for (size_t k = 0; k < s_->genres.size (); ++k)
                if (s_->genres[k] == s_->request.genres[static_cast<size_t> (i)])
                    current = static_cast<int> (k) + 1;
        m->setCurrent (current);
        m->invalid ();
    }
}

void MidiAgentEditor::refreshSettingsText ()
{
    auto set = [&] (Tag t, const std::string& v) {
        if (edits_[t])
            edits_[t]->setText (v.c_str ());
    };
    std::string weights;
    for (size_t i = 0; i < s_->request.weights.size (); ++i)
    {
        char buf[32];
        std::snprintf (buf, sizeof (buf), "%s%g", i ? "," : "", s_->request.weights[i]);
        weights += buf;
    }
    set (kTagWeights, weights);
    set (kTagBars, std::to_string (s_->request.bars));
    set (kTagBpm, s_->request.bpm > 0 ? std::to_string (s_->request.bpm) : "");
    set (kTagSeed, s_->request.seed >= 0 ? std::to_string (s_->request.seed) : "");
    set (kTagKey, s_->request.key);
    set (kTagTitle, s_->request.title);
    set (kTagPython, s_->agent.pythonExe);
    set (kTagSkillRoot, s_->agent.skillRoot);
    if (formButton_)
        formButton_->setHot (s_->request.useForm);
    if (snapMenu_)
    {
        int idx = 2;
        for (int i = 0; i < static_cast<int> (sizeof (kSnaps) / sizeof (kSnaps[0])); ++i)
            if (kSnaps[i].ticks == s_->snapTicks)
                idx = i;
        snapMenu_->setCurrent (idx);
    }
    if (settingsStatus_)
    {
        if (!s_->agent.valid ())
            settingsStatus_->setText ("set SKILL DIR to the midi-agent-skill folder");
        else if (s_->genres.empty ())
            settingsStatus_->setText ("no genres found in SKILL DIR/genres");
        else
            settingsStatus_->setText (std::to_string (s_->genres.size ()) + " genres  \xE2\x80\xA2  plans/<title>.composition.json");
    }
}

void MidiAgentEditor::refreshAll ()
{
    if (!frame)
        return;
    if (s_->agent.valid () && s_->genres.empty ())
        s_->genres = listGenres (s_->agent);
    refreshGenreMenus ();
    refreshSettingsText ();
    if (segLabel_)
    {
        const Segment* seg = s_->segment ();
        Track* t = s_->track ();
        std::string text = seg ? seg->name : std::string ("no segment");
        if (t)
            text += "  /  " + t->displayName (static_cast<size_t> (s_->selectedTrack));
        segLabel_->setText (text);
    }
    if (synthButton_)
        synthButton_->setHot (ctrl_->getParamNormalized (kParamSynthOn) > 0.5);
    if (midiOutButton_)
        midiOutButton_->setHot (ctrl_->getParamNormalized (kParamMidiOut) > 0.5);
    if (gainSlider_)
        gainSlider_->setValueNormalized (static_cast<float> (ctrl_->getParamNormalized (kParamGain)));
    if (previewButton_)
        previewButton_->setHot (s_->loopPreview);
    frame->invalid ();
}

void MidiAgentEditor::compositionReplaced ()
{
    if (!frame)
        return;
    refreshAll ();
    if (roll_)
        roll_->segmentChanged ();
    pushedRevision_ = s_->revision;
}

void MidiAgentEditor::transportUpdated ()
{
    // drawn on the next idle tick
}

void MidiAgentEditor::selectSegment (int index)
{
    if (index < 0 || index >= static_cast<int> (s_->segments.size ()))
        return;
    s_->selectedSegment = index;
    s_->selectedNotes.clear ();
    if (s_->loopPreview)
        ctrl_->sendPreview (true); // move the loop to the new segment
    refreshAll ();
    if (roll_)
        roll_->segmentChanged ();
}

void MidiAgentEditor::selectTrack (int index)
{
    if (index < 0 || index >= static_cast<int> (s_->comp.tracks.size ()))
        return;
    s_->selectedTrack = index;
    s_->selectedNotes.clear ();
    refreshAll ();
    if (roll_)
        roll_->scrollToContent ();
}

void MidiAgentEditor::onNoteEdit ()
{
    lastEditMillis_ = platform::nowMillis ();
    if (tracks_)
        tracks_->invalid ();
    if (header_)
        header_->invalid ();
}

void MidiAgentEditor::log (const std::string& line)
{
    s_->addLog (line);
    if (logView_)
        logView_->invalid ();
}

// --- idle ------------------------------------------------------------------

CMessageResult MidiAgentEditor::notify (CBaseObject* sender, const char* message)
{
    if (message == CVSTGUITimer::kMsgTimer && frame)
    {
        ++tick_;
        ctrl_->pumpJobs ();
        ctrl_->sendPoll ();
        // push edits to the processor once the mouse has rested for a moment
        if (s_->revision != pushedRevision_ && platform::nowMillis () - lastEditMillis_ > 120)
        {
            ctrl_->pushComposition ();
            pushedRevision_ = s_->revision;
            if (s_->loopPreview)
                ctrl_->sendPreview (true);
        }
        if (s_->transport.playing || s_->transport.previewing || s_->busy || (tick_ % 8) == 0)
            for (CView* v : dirtyEach_)
                v->invalid ();
        if (logView_ && (tick_ % 5) == 0)
            logView_->invalid ();
        if ((tick_ % 10) == 0)
        {
            if (synthButton_)
                synthButton_->setHot (ctrl_->getParamNormalized (kParamSynthOn) > 0.5);
            if (midiOutButton_)
                midiOutButton_->setHot (ctrl_->getParamNormalized (kParamMidiOut) > 0.5);
            if (gainSlider_ && !gainSlider_->isEditing ())
                gainSlider_->setValueNormalized (static_cast<float> (ctrl_->getParamNormalized (kParamGain)));
        }
    }
    return VSTGUIEditor::notify (sender, message);
}

void MidiAgentEditor::valueChanged (CControl* control)
{
    if (!control)
        return;
    int32_t tag = control->getTag ();
    if (tag == kTagGain)
    {
        ctrl_->setParam (kParamGain, control->getValueNormalized ());
        return;
    }
    if (tag == kTagSnap && snapMenu_)
    {
        int idx = snapMenu_->getCurrentIndex ();
        if (idx >= 0 && idx < static_cast<int> (sizeof (kSnaps) / sizeof (kSnaps[0])))
            s_->snapTicks = kSnaps[idx].ticks;
        if (roll_)
            roll_->invalid ();
        return;
    }
    if (tag >= kTagGenreA && tag <= kTagGenreC)
    {
        readGenerateControls ();
        return;
    }
    if (tag == kTagSkillRoot || tag == kTagPython)
    {
        readGenerateControls ();
        s_->genres = s_->agent.valid () ? listGenres (s_->agent) : std::vector<std::string> ();
        refreshGenreMenus ();
        refreshSettingsText ();
        return;
    }
    readGenerateControls ();
}

void MidiAgentEditor::readGenerateControls ()
{
    auto text = [&] (Tag t) { return edits_[t] ? std::string (edits_[t]->getText ().getString ()) : std::string (); };
    s_->request.genres.clear ();
    for (auto* m : genreMenus_)
    {
        if (!m)
            continue;
        int idx = m->getCurrentIndex ();
        if (idx > 0 && static_cast<size_t> (idx - 1) < s_->genres.size ())
            s_->request.genres.push_back (s_->genres[static_cast<size_t> (idx - 1)]);
    }
    s_->request.weights.clear ();
    for (const std::string& w : splitList (text (kTagWeights)))
        s_->request.weights.push_back (std::atof (w.c_str ()));
    s_->request.bars = std::max (1, std::atoi (text (kTagBars).c_str ()));
    s_->request.bpm = std::atoi (text (kTagBpm).c_str ());
    std::string seed = text (kTagSeed);
    s_->request.seed = seed.empty () ? -1 : std::atoi (seed.c_str ());
    s_->request.key = text (kTagKey);
    std::string title = text (kTagTitle);
    s_->request.title = title.empty () ? "midi-agent" : title;
    std::string py = text (kTagPython);
    if (!py.empty ())
        s_->agent.pythonExe = py;
    s_->agent.skillRoot = text (kTagSkillRoot);
    if (formButton_)
        s_->request.useForm = formButton_->isHot ();
}

// --- actions ----------------------------------------------------------------

void MidiAgentEditor::actionNew ()
{
    readGenerateControls ();
    Composition c = Composition::makeEmpty (s_->request.title, s_->request.bpm > 0 ? s_->request.bpm : 140,
                                            s_->request.bars > 0 ? s_->request.bars : 8);
    ctrl_->setComposition (std::move (c), "", "new empty composition");
}

void MidiAgentEditor::loadCompositionFile (const std::string& path, const std::string& why, bool keepSelection)
{
    Composition c;
    std::string err;
    if (!Composition::loadFile (path, c, err))
    {
        log ("ERROR " + err);
        return;
    }
    int seg = s_->selectedSegment, trk = s_->selectedTrack;
    ctrl_->setComposition (std::move (c), path, why + ": " + path);
    if (keepSelection)
    {
        s_->selectedSegment = std::min (seg, static_cast<int> (s_->segments.size ()) - 1);
        s_->selectedTrack = std::min (trk, static_cast<int> (s_->comp.tracks.size ()) - 1);
        if (s_->selectedSegment < 0)
            s_->selectedSegment = 0;
        if (s_->selectedTrack < 0)
            s_->selectedTrack = 0;
        compositionReplaced ();
    }
}

void MidiAgentEditor::actionOpen ()
{
    auto* selector = CNewFileSelector::create (frame, CNewFileSelector::kSelectFile);
    if (!selector)
        return;
    selector->setTitle ("Open a composition JSON");
    selector->addFileExtension (CFileExtension ("Composition JSON", "json", "application/json"));
    if (s_->agent.valid ())
        selector->setInitialDirectory (platform::joinPath (s_->agent.skillRoot, "plans").c_str ());
    selector->run ([this] (CNewFileSelector* sel) {
        if (sel->getNumSelectedFiles () > 0)
            loadCompositionFile (sel->getSelectedFile (0), "opened", false);
    });
    selector->forget ();
}

void MidiAgentEditor::actionSave (bool saveAs)
{
    if (!saveAs && !s_->compositionPath.empty ())
    {
        std::string err;
        if (s_->comp.saveFile (s_->compositionPath, err))
        {
            s_->dirty = false;
            log ("saved " + s_->compositionPath);
        }
        else
            log ("ERROR " + err);
        return;
    }
    auto* selector = CNewFileSelector::create (frame, CNewFileSelector::kSelectSaveFile);
    if (!selector)
        return;
    selector->setTitle ("Save composition JSON");
    selector->setDefaultExtension (CFileExtension ("Composition JSON", "json", "application/json"));
    selector->setDefaultSaveName ((platform::safeName (s_->comp.title) + ".composition.json").c_str ());
    if (s_->agent.valid ())
        selector->setInitialDirectory (platform::joinPath (s_->agent.skillRoot, "plans").c_str ());
    selector->run ([this] (CNewFileSelector* sel) {
        if (sel->getNumSelectedFiles () == 0)
            return;
        std::string path = sel->getSelectedFile (0);
        std::string err;
        if (s_->comp.saveFile (path, err))
        {
            s_->compositionPath = path;
            s_->dirty = false;
            log ("saved " + path);
        }
        else
            log ("ERROR " + err);
    });
    selector->forget ();
}

void MidiAgentEditor::actionGenerate ()
{
    readGenerateControls ();
    if (!s_->agent.valid ())
    {
        log ("ERROR set SKILL DIR first (the folder that holds SKILL.md)");
        return;
    }
    if (s_->request.genres.empty ())
    {
        log ("ERROR pick at least one genre");
        return;
    }
    AgentConfig cfg = s_->agent;
    ComposeRequest req = s_->request;
    std::string out = platform::joinPath (platform::joinPath (cfg.skillRoot, "plans"),
                                          platform::safeName (req.title) + ".composition.json");
    log ("$ " + joinList (buildComposeArgs (cfg, req, out), " "));
    ctrl_->startJob ("generating " + joinList (req.genres, "+"), [this, cfg, req, out] () -> std::function<void ()> {
        AgentResult r = runCompose (cfg, req, out);
        return [this, r] () {
            std::istringstream ss (r.log);
            std::string line;
            int shown = 0;
            while (std::getline (ss, line) && shown < 12)
                if (!line.empty ())
                {
                    log ("  " + line);
                    ++shown;
                }
            if (!r.ok)
            {
                log ("ERROR " + r.error);
                return;
            }
            loadCompositionFile (r.compositionPath, "generated", false);
        };
    });
}

bool MidiAgentEditor::writeCompositionForPython (std::string& path)
{
    path = s_->compositionPath;
    if (path.empty ())
    {
        if (!s_->agent.valid ())
            return false;
        path = platform::joinPath (platform::joinPath (s_->agent.skillRoot, "plans"),
                                   platform::safeName (s_->comp.title) + ".composition.json");
        platform::makeDirs (platform::parentPath (path));
    }
    std::string err;
    if (!s_->comp.saveFile (path, err))
    {
        log ("ERROR " + err);
        return false;
    }
    s_->compositionPath = path;
    s_->dirty = false;
    return true;
}

void MidiAgentEditor::actionRegenerate (bool trackOnly)
{
    readGenerateControls ();
    const Segment* seg = s_->segment ();
    Track* track = s_->track ();
    if (!seg || !track)
        return;
    if (!s_->agent.valid ())
    {
        log ("ERROR set SKILL DIR first");
        return;
    }
    if (seg->sectionIndex < 0)
    {
        log ("ERROR this song has no named sections; --regenerate needs one (generate with the skill first)");
        return;
    }
    if (trackOnly && track->role.empty ())
    {
        log ("ERROR the track has no role, so the pipeline cannot regenerate it");
        return;
    }
    std::string path;
    if (!writeCompositionForPython (path))
        return;
    RegenerateRequest req;
    req.resumePath = path;
    req.section = seg->name;
    req.genres = s_->request.genres;
    if (trackOnly)
        req.layers = {track->role};
    std::random_device rd;
    req.seed = static_cast<int> (rd () % 100000);
    AgentConfig cfg = s_->agent;
    log ("$ " + joinList (buildRegenerateArgs (cfg, req, path), " "));
    ctrl_->startJob ("regenerating " + seg->name, [this, cfg, req, path] () -> std::function<void ()> {
        AgentResult r = runRegenerate (cfg, req, path);
        return [this, r] () {
            if (!r.ok)
            {
                std::istringstream ss (r.log);
                std::string line;
                while (std::getline (ss, line))
                    if (!line.empty ())
                        log ("  " + line);
                log ("ERROR " + r.error);
                return;
            }
            loadCompositionFile (r.compositionPath, "regenerated", true);
        };
    });
}

void MidiAgentEditor::actionClearSegment ()
{
    const Segment* seg = s_->segment ();
    Track* track = s_->track ();
    if (!seg || !track)
        return;
    replaceSegmentNotes (*track, *seg, {});
    s_->selectedNotes.clear ();
    s_->touch ();
    onNoteEdit ();
    log ("cleared " + track->displayName (static_cast<size_t> (s_->selectedTrack)) + " in " + seg->name);
    if (roll_)
        roll_->invalid ();
}

void MidiAgentEditor::togglePreview ()
{
    bool on = previewButton_ && previewButton_->isHot ();
    if (on && !s_->segment ())
    {
        previewButton_->setHot (false);
        return;
    }
    ctrl_->sendPreview (on);
    log (on ? "preview loop on: " + s_->segment ()->name : "preview loop off");
}

std::string MidiAgentEditor::autoSavePath () const
{
    return platform::joinPath (ctrl_->workDir (), platform::safeName (s_->comp.title));
}

std::string MidiAgentEditor::prepareSegmentMidi (bool trackOnly)
{
    const Segment* seg = s_->segment ();
    if (!seg)
        return "";
    Composition clip = extractSegment (s_->comp, *seg, trackOnly ? s_->selectedTrack : -1);
    std::string name = platform::safeName (clip.title);
    if (trackOnly && s_->track ())
        name += "-" + platform::safeName (s_->track ()->displayName (static_cast<size_t> (s_->selectedTrack)));
    std::string path = platform::joinPath (ctrl_->workDir (), name + ".mid");
    std::string err;
    if (!writeMidiFile (path, clip, {}, err))
    {
        log ("ERROR " + err);
        return "";
    }
    log ("drag: " + path);
    return path;
}

std::string MidiAgentEditor::prepareSegmentWav ()
{
    const Segment* seg = s_->segment ();
    if (!seg)
        return "";
    std::string path = platform::joinPath (ctrl_->workDir (), platform::safeName (s_->comp.title + "-" + seg->name) + ".wav");
    RenderOptions o;
    std::string err;
    if (!renderToWav (s_->comp, seg, o, path, err))
    {
        log ("ERROR " + err);
        return "";
    }
    log ("drag: " + path);
    return path;
}

void MidiAgentEditor::actionRenderWav (bool wholeSong)
{
    const Segment* seg = s_->segment ();
    if (!wholeSong && !seg)
        return;
    auto* selector = CNewFileSelector::create (frame, CNewFileSelector::kSelectSaveFile);
    if (!selector)
        return;
    selector->setTitle (wholeSong ? "Render the song to WAV" : "Render the segment to WAV");
    selector->setDefaultExtension (CFileExtension ("WAVE", "wav", "audio/wav"));
    std::string base = platform::safeName (wholeSong ? s_->comp.title : s_->comp.title + "-" + seg->name);
    selector->setDefaultSaveName ((base + ".wav").c_str ());
    Segment segCopy = seg ? *seg : Segment ();
    selector->run ([this, wholeSong, segCopy] (CNewFileSelector* sel) {
        if (sel->getNumSelectedFiles () == 0)
            return;
        std::string path = sel->getSelectedFile (0);
        Composition comp = s_->comp;
        ctrl_->startJob ("rendering " + platform::fileName (path), [this, comp, path, wholeSong, segCopy] () -> std::function<void ()> {
            RenderOptions o;
            std::string err;
            bool ok = renderToWav (comp, wholeSong ? nullptr : &segCopy, o, path, err);
            return [this, ok, err, path] () { log (ok ? "rendered " + path : "ERROR " + err); };
        });
    });
    selector->forget ();
}

void MidiAgentEditor::actionFindSkill ()
{
    auto* selector = CNewFileSelector::create (frame, CNewFileSelector::kSelectFile);
    if (!selector)
        return;
    selector->setTitle ("Pick SKILL.md inside the midi-agent-skill folder");
    selector->addFileExtension (CFileExtension ("Markdown", "md", "text/markdown"));
    selector->run ([this] (CNewFileSelector* sel) {
        if (sel->getNumSelectedFiles () == 0)
            return;
        std::string root = findSkillRoot (platform::parentPath (sel->getSelectedFile (0)));
        if (root.empty ())
        {
            log ("ERROR that folder has no SKILL.md with skills/compose_from_profile.py");
            return;
        }
        s_->agent.skillRoot = root;
        s_->genres = listGenres (s_->agent);
        refreshGenreMenus ();
        refreshSettingsText ();
        log ("skill folder: " + root + " (" + std::to_string (s_->genres.size ()) + " genres)");
    });
    selector->forget ();
}

} // namespace midiagent
