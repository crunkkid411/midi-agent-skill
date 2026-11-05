#include "source/editor.h"
#include "core/instrument_map.h"
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

void tip (CView* v, const char* text)
{
    v->setTooltipText (text);
}

CSlider* makeSlider (const CRect& r, IControlListener* l, int32_t tag, const CColor& color)
{
    auto* sl = new CSlider (r, l, tag, 0, static_cast<int32_t> (r.getWidth ()) - 10, nullptr, nullptr);
    sl->setDrawStyle (CSlider::kDrawFrame | CSlider::kDrawBack | CSlider::kDrawValue);
    sl->setFrameColor (theme::kChromeLo);
    sl->setBackColor (CColor (6, 4, 12));
    sl->setValueColor (color);
    return sl;
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
    strip_ = new SegmentStripView (CRect (0, kHeaderH, W, kHeaderH + kStripH), s_,
                                   [this] (int i, bool ctrl, bool shift) { selectSegment (i, ctrl, shift); });
    tip (strip_, "Click a section to jump Cubase there. Ctrl+click adds or removes a section, Shift+click selects every section up to the one you click.");
    frame->addView (strip_);

    const double top = kHeaderH + kStripH + kActionH;
    const double bottom = H - kBottomH;
    tracks_ = new TrackListView (CRect (0, top, kTrackW, bottom), s_, [this] (TrackListView::Action a, int i) {
        switch (a)
        {
            case TrackListView::Action::Select:
            {
                selectTrack (i);
                // sound the part: its first note in this section, else a middle note
                const Track& t = s_->comp.tracks[static_cast<size_t> (i)];
                const Segment* seg = s_->segment ();
                const Note* first = nullptr;
                for (const Note& n : t.notes)
                    if (!seg || (n.start >= seg->startTick && n.start < seg->endTick))
                    {
                        first = &n;
                        break;
                    }
                ctrl_->sendAudition (i, first ? first->pitch : (t.isDrum ? 38 : 60), first ? first->velocity : 0.8,
                                     first ? first->drum : std::string (t.isDrum ? "snare" : ""));
                break;
            }
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
    roll_->setOnAudition ([this] (int pitch, double velocity, const std::string& lane) {
        ctrl_->sendAudition (s_->selectedTrack, pitch, velocity, lane);
    });
    frame->addView (roll_);

    logView_ = new LogView (CRect (8, bottom + 8, 600, H - 8), &s_->log);
    frame->addView (logView_);

    buildHeader ();
    buildActionBar ();
    buildGeneratePanel ();
    frame->enableTooltips (true);

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
    loopButton_ = synthButton_ = midiOutButton_ = formButton_ = generateButton_ = nullptr;
    segLabel_ = settingsStatus_ = surpriseLabel_ = nullptr;
    gainSlider_ = surpriseSlider_ = nullptr;
    for (auto& m : mixSliders_)
        m = nullptr;
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
    auto button = [&] (const char* title, double w, const char* help, std::function<void (NeonButton&)> f) {
        auto* b = new NeonButton (CRect (x, 12, x + w, 40), title, std::move (f));
        tip (b, help);
        frame->addView (b);
        x += w + 6;
        return b;
    };
    button ("NEW", 60, "Start an empty song (keeps the generate settings).", [this] (NeonButton&) { actionNew (); });
    button ("OPEN", 66, "Open a song you saved before.", [this] (NeonButton&) { actionOpen (); });
    button ("SAVE", 62, "Save the song (it is also saved inside the Cubase project).", [this] (NeonButton&) { actionSave (false); });
    button ("SAVE AS", 80, "Save the song under a new name.", [this] (NeonButton&) { actionSave (true); });
    button ("PANIC", 70, "Stuck notes? This stops every note right now.", [this] (NeonButton&) {
        ctrl_->sendPanic ();
        log ("panic: all notes off");
    });

    x = 580;
    synthButton_ = new NeonButton (CRect (x, 44, x + 72, 66), "SYNTH", [this] (NeonButton& b) {
        ctrl_->setParam (kParamSynthOn, b.isHot () ? 1.0 : 0.0);
    });
    synthButton_->setToggle (true);
    tip (synthButton_, "The plug-in's own sketch sounds. Turn off when your real instruments (Odin, SD3, Serum...) play the parts.");
    frame->addView (synthButton_);
    x += 78;
    midiOutButton_ = new NeonButton (CRect (x, 44, x + 86, 66), "MIDI OUT", [this] (NeonButton& b) {
        ctrl_->setParam (kParamMidiOut, b.isHot () ? 1.0 : 0.0);
    });
    midiOutButton_->setToggle (true);
    tip (midiOutButton_, "Send every part to its own MIDI output so the instrument tracks in Cubase play it.");
    frame->addView (midiOutButton_);
    x += 92;
    frame->addView (new NeonLabel (CRect (x, 44, x + 44, 66), "LEVEL", theme::tinyFont (), theme::kTextDim));
    x += 44;
    gainSlider_ = makeSlider (CRect (x, 48, x + 150, 62), this, kTagGain, theme::kMagenta);
    tip (gainSlider_, "Volume of the plug-in's own sketch sounds.");
    frame->addView (gainSlider_);
}

void MidiAgentEditor::buildActionBar ()
{
    const double y0 = kHeaderH + kStripH + 5, y1 = y0 + 28;
    double x = 8;
    segLabel_ = new NeonLabel (CRect (x, y0, x + 150, y1), "SEGMENT", theme::labelFont (), theme::kMagenta);
    segLabel_->setGlow (true);
    frame->addView (segLabel_);
    x += 156;

    auto button = [&] (const char* title, double w, const char* help, std::function<void (NeonButton&)> f) {
        auto* b = new NeonButton (CRect (x, y0, x + w, y1), title, std::move (f));
        tip (b, help);
        frame->addView (b);
        x += w + 6;
        return b;
    };
    auto chip = [&] (const char* title, double w, const char* help, std::function<std::string ()> prep) {
        auto* c = new DragChip (CRect (x, y0, x + w, y1), title, std::move (prep));
        tip (c, help);
        frame->addView (c);
        x += w + 6;
        return c;
    };

    loopButton_ = button ("LOOP", 58, "Loop the selected sections. Press play in Cubase: it jumps back to the start of the selection at the end.",
                          [this] (NeonButton&) { toggleLoop (); });
    loopButton_->setToggle (true);
    button ("REGEN SEG", 86, "New idea for the selected sections, every part. Each press gives something different (see SURPRISE).",
            [this] (NeonButton&) { actionRegenerate (false); });
    button ("REGEN TRACK", 98, "New idea for the selected part only, in the selected sections.", [this] (NeonButton&) { actionRegenerate (true); });
    button ("CLEAR", 58, "Delete the selected part's notes in the selected sections.", [this] (NeonButton&) { actionClearSegment (); });

    frame->addView (new NeonLabel (CRect (x, y0, x + 36, y1), "SNAP", theme::tinyFont (), theme::kTextDim));
    x += 36;
    snapMenu_ = new COptionMenu (CRect (x, y0 + 2, x + 64, y1 - 2), this, kTagSnap);
    styleMenu (snapMenu_);
    for (const auto& sn : kSnaps)
        snapMenu_->addEntry (sn.label);
    tip (snapMenu_, "Grid for drawing and moving notes in the piano roll (1/16 = sixteenth notes).");
    frame->addView (snapMenu_);
    x += 76;

    chip ("DRAG MIDI \xC2\xB7 SEG", 118, "Drag onto Cubase: every part of the selected sections as MIDI, keyswitches included.",
          [this] { return prepareSegmentMidi (false); });
    chip ("DRAG MIDI \xC2\xB7 TRACK", 126, "Drag onto a Cubase track: only the selected part, keyswitches included.",
          [this] { return prepareSegmentMidi (true); });
    chip ("DRAG WAV \xC2\xB7 SEG", 116, "Drag onto Cubase: the selected sections as audio (sketch sounds).", [this] { return prepareSegmentWav (); });
    button ("RENDER WAV...", 104, "Save the selected sections as a WAV file (sketch sounds).", [this] (NeonButton&) { actionRenderWav (false); });
    button ("RENDER SONG...", 108, "Save the whole song as a WAV file (sketch sounds).", [this] (NeonButton&) { actionRenderWav (true); });
}

void MidiAgentEditor::buildGeneratePanel ()
{
    const double x0 = 612, top = kHeight - kBottomH + 8;
    auto label = [&] (double x, double y, double w, const char* text, const char* help = nullptr) {
        auto* l = new NeonLabel (CRect (x, y, x + w, y + 24), text, theme::tinyFont (), theme::kTextDim);
        if (help)
            tip (l, help);
        frame->addView (l);
        return l;
    };
    auto edit = [&] (Tag tag, double x, double y, double w, const char* placeholder, const char* help) {
        auto* e = new CTextEdit (CRect (x, y + 2, x + w, y + 24), this, tag);
        styleTextEdit (e);
        e->setPlaceholderString (placeholder);
        tip (e, help);
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
        tip (genreMenus_[i], "A style for the song. Pick up to three to blend them.");
        frame->addView (genreMenus_[i]);
    }

    y += 28;
    label (x0, y, 90, "HOW MUCH", "How much of each style goes into the blend. Slide right = more of that style.");
    for (int i = 0; i < 3; ++i)
    {
        double x = x0 + 96 + i * 170;
        mixSliders_[i] = makeSlider (CRect (x, y + 6, x + 164, y + 20), this, static_cast<int32_t> (kTagMixA + i), theme::kCyan);
        mixSliders_[i]->setValueNormalized (1.0f);
        tip (mixSliders_[i], "How much of this style goes into the blend. Slide right = more of it.");
        frame->addView (mixSliders_[i]);
    }

    y += 26;
    surpriseLabel_ = label (x0, y, 210, "SURPRISE", "How far each GENERATE / REGEN may wander from the style's usual grooves, chords and riffs.");
    surpriseSlider_ = makeSlider (CRect (x0 + 214, y + 6, x0 + 400, y + 20), this, kTagSurprise, theme::kLime);
    tip (surpriseSlider_, "Left = stay close to the style. Right = new grooves, chords and riffs every time.");
    frame->addView (surpriseSlider_);
    label (x0 + 410, y, 36, "BARS");
    edit (kTagBars, x0 + 446, y, 40, "8", "Song length in bars (ignored with FULL SONG).");
    label (x0 + 494, y, 30, "BPM");
    edit (kTagBpm, x0 + 524, y, 48, "auto", "Tempo. Empty = the style's usual tempo.");

    y += 30;
    label (x0, y, 30, "KEY");
    edit (kTagKey, x0 + 30, y, 70, "auto", "Key, like A minor. Empty = the style's usual key.");
    formButton_ = new NeonButton (CRect (x0 + 108, y + 1, x0 + 200, y + 25), "FULL SONG", [this] (NeonButton& b) {
        s_->request.useForm = b.isHot ();
    });
    formButton_->setToggle (true);
    tip (formButton_, "On: a whole song with intro, verses, choruses and so on. Off: just BARS bars of one idea.");
    frame->addView (formButton_);
    label (x0 + 208, y, 36, "TITLE");
    edit (kTagTitle, x0 + 244, y, 130, "midi-agent", "Song name, used for the saved file.");
    generateButton_ = new NeonButton (CRect (x0 + 382, y - 1, x0 + 520, y + 27), "\xE2\x9C\xA6 GENERATE \xE2\x9C\xA6",
                                      [this] (NeonButton&) { actionGenerate (); });
    generateButton_->setHot (true);
    tip (generateButton_, "Write a new song from the styles above. Every press is a new song.");
    frame->addView (generateButton_);

    y += 32;
    label (x0, y, 62, "SKILL DIR", "Where the midi-agent-skill folder is (set once).");
    edit (kTagSkillRoot, x0 + 64, y, 360, "folder with SKILL.md", "Where the midi-agent-skill folder is (set once).");
    auto* find = new NeonButton (CRect (x0 + 430, y + 1, x0 + 514, y + 25), "FIND...", [this] (NeonButton&) { actionFindSkill (); });
    tip (find, "Pick the SKILL.md file inside the midi-agent-skill folder.");
    frame->addView (find);
    settingsStatus_ = new NeonLabel (CRect (x0 + 520, y, kWidth - 8, y + 24), "", theme::tinyFont (), theme::kTextDim);
    frame->addView (settingsStatus_);
}

void MidiAgentEditor::refreshSurpriseLabel ()
{
    if (!surpriseLabel_)
        return;
    double v = s_->variation;
    const char* words = v < 0.15 ? "stay on style" : v < 0.4 ? "a few new ideas" : v < 0.7 ? "fresh take" : "surprise me";
    surpriseLabel_->setText (std::string ("SURPRISE: ") + words);
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
    for (int i = 0; i < 3; ++i)
        if (mixSliders_[i])
        {
            double w = static_cast<size_t> (i) < s_->request.weights.size () ? s_->request.weights[static_cast<size_t> (i)] : 1.0;
            mixSliders_[i]->setValueNormalized (static_cast<float> (std::max (0.0, std::min (1.0, w))));
        }
    if (surpriseSlider_)
        surpriseSlider_->setValueNormalized (static_cast<float> (s_->variation));
    refreshSurpriseLabel ();
    set (kTagBars, std::to_string (s_->request.bars));
    set (kTagBpm, s_->request.bpm > 0 ? std::to_string (s_->request.bpm) : "");
    set (kTagKey, s_->request.key);
    set (kTagTitle, s_->request.title);
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
            settingsStatus_->setText (std::to_string (s_->genres.size ()) + " styles");
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
        if (seg && s_->segmentSelection.size () > 1)
            text = std::to_string (s_->segmentSelection.size ()) + " sections";
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
    if (loopButton_)
        loopButton_->setHot (s_->loopOn);
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

void MidiAgentEditor::selectSegment (int index, bool ctrl, bool shift)
{
    if (index < 0 || index >= static_cast<int> (s_->segments.size ()))
        return;
    s_->clickSegment (index, ctrl, shift);
    s_->selectedNotes.clear ();
    if (s_->loopOn)
        ctrl_->sendLoop (); // the loop follows the selection
    // move Cubase's playhead to the clicked section (not when ctrl only toggles one)
    if (!ctrl || shift)
    {
        const Segment* seg = s_->segment ();
        if (seg && ctrl_->locateTick (seg->startTick))
            lastLocateMillis_ = platform::nowMillis ();
    }
    refreshAll ();
    if (roll_)
        roll_->segmentChanged ();
}

void MidiAgentEditor::toggleLoop ()
{
    s_->loopOn = loopButton_ && loopButton_->isHot ();
    ctrl_->sendLoop ();
    if (s_->loopOn)
    {
        Segment span = s_->selectionSpan ();
        if (!s_->transport.playing || s_->transport.tick () < span.startTick || s_->transport.tick () >= span.endTick)
            ctrl_->locateTick (span.startTick);
        lastLocateMillis_ = platform::nowMillis ();
        log ("loop on: " + span.name + " (press play in Cubase)");
    }
    else
        log ("loop off");
}

void MidiAgentEditor::checkLoop ()
{
    // ponytail: polled on the editor tick; notes past the end are already silenced by the processor
    if (!s_->loopOn || !s_->transport.playing)
        return;
    Segment span = s_->selectionSpan ();
    int64_t now = s_->transport.tick ();
    if (span.endTick > span.startTick && now >= span.endTick - kPPQ / 32 && platform::nowMillis () - lastLocateMillis_ > 250)
    {
        ctrl_->locateTick (span.startTick);
        lastLocateMillis_ = platform::nowMillis ();
    }
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
        checkLoop ();
        // push edits to the processor once the mouse has rested for a moment
        if (s_->revision != pushedRevision_ && platform::nowMillis () - lastEditMillis_ > 120)
        {
            ctrl_->pushComposition ();
            pushedRevision_ = s_->revision;
        }
        if (s_->transport.playing || s_->busy || (tick_ % 8) == 0)
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
    if (tag == kTagSurprise)
    {
        s_->variation = control->getValueNormalized ();
        refreshSurpriseLabel ();
        return;
    }
    if (tag >= kTagGenreA && tag <= kTagGenreC)
    {
        readGenerateControls ();
        return;
    }
    if (tag == kTagSkillRoot)
    {
        readGenerateControls ();
        ctrl_->refreshSkill ();
        ctrl_->pushComposition ();
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
    s_->request.weights.clear ();
    for (int i = 0; i < 3; ++i)
    {
        COptionMenu* m = genreMenus_[i];
        if (!m)
            continue;
        int idx = m->getCurrentIndex ();
        if (idx > 0 && static_cast<size_t> (idx - 1) < s_->genres.size ())
        {
            s_->request.genres.push_back (s_->genres[static_cast<size_t> (idx - 1)]);
            // a style slid all the way left still counts a little, else it would vanish silently
            double w = mixSliders_[i] ? mixSliders_[i]->getValueNormalized () : 1.0;
            s_->request.weights.push_back (std::max (0.05, w));
        }
    }
    s_->request.variation = s_->variation;
    s_->request.seed = -1; // every GENERATE is a new song
    s_->request.bars = std::max (1, std::atoi (text (kTagBars).c_str ()));
    s_->request.bpm = std::atoi (text (kTagBpm).c_str ());
    s_->request.key = text (kTagKey);
    std::string title = text (kTagTitle);
    s_->request.title = title.empty () ? "midi-agent" : title;
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
    std::vector<int> sel = s_->segmentSelection;
    ctrl_->setComposition (std::move (c), path, why + ": " + path);
    if (keepSelection)
    {
        s_->segmentSelection = sel;
        s_->selectedSegment = std::min (seg, static_cast<int> (s_->segments.size ()) - 1);
        s_->selectedTrack = std::min (trk, static_cast<int> (s_->comp.tracks.size ()) - 1);
        if (s_->selectedSegment < 0)
            s_->selectedSegment = 0;
        if (s_->selectedTrack < 0)
            s_->selectedTrack = 0;
        s_->rebuildSegments (); // drops selection entries past the end
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
    Track* track = s_->track ();
    if (s_->segments.empty () || !track)
        return;
    if (!s_->agent.valid ())
    {
        log ("ERROR set SKILL DIR first");
        return;
    }
    if (trackOnly && track->role.empty ())
    {
        log ("ERROR the track has no role, so the pipeline cannot regenerate it");
        return;
    }
    std::vector<std::string> sections;
    for (int i : s_->segmentSelection)
        if (s_->segments[static_cast<size_t> (i)].sectionIndex >= 0)
            sections.push_back (s_->segments[static_cast<size_t> (i)].name);
    if (sections.empty ())
    {
        log ("ERROR this song has no named sections; REGEN needs one (generate with the skill first)");
        return;
    }
    std::string path;
    if (!writeCompositionForPython (path))
        return;
    RegenerateRequest req;
    req.resumePath = path;
    req.genres = s_->request.genres;
    req.weights = s_->request.weights;
    req.variation = s_->variation;
    if (trackOnly)
        req.layers = {track->role};
    AgentConfig cfg = s_->agent;
    std::string label = sections.size () == 1 ? sections[0] : std::to_string (sections.size ()) + " sections";
    ctrl_->startJob ("regenerating " + label, [this, cfg, req, path, sections] () -> std::function<void ()> {
        AgentResult r;
        std::random_device rd;
        for (const std::string& section : sections) // one call per section, each on the result of the last
        {
            RegenerateRequest one = req;
            one.section = section;
            one.seed = static_cast<int> (rd () % 1000000); // a new idea on every press
            r = runRegenerate (cfg, one, path);
            if (!r.ok)
                break;
        }
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
    Track* track = s_->track ();
    if (!track)
        return;
    for (int i : s_->segmentSelection)
        replaceSegmentNotes (*track, s_->segments[static_cast<size_t> (i)], {});
    s_->selectedNotes.clear ();
    s_->touch ();
    onNoteEdit ();
    log ("cleared " + track->displayName (static_cast<size_t> (s_->selectedTrack)) + " in " + s_->selectionSpan ().name);
    if (roll_)
        roll_->invalid ();
}

std::string MidiAgentEditor::autoSavePath () const
{
    return platform::joinPath (ctrl_->workDir (), platform::safeName (s_->comp.title));
}

Composition MidiAgentEditor::selectionClip (int trackIndex) const
{
    // the span from the first to the last selected section; sections left out
    // of a ctrl+click selection stay silent, so every part keeps its place
    Composition src = s_->comp;
    if (s_->segmentSelection.size () > 1)
        for (int i = s_->segmentSelection.front () + 1; i < s_->segmentSelection.back (); ++i)
            if (!s_->isSegmentSelected (i))
                for (Track& t : src.tracks)
                    replaceSegmentNotes (t, s_->segments[static_cast<size_t> (i)], {});
    return extractSegment (applyInstrumentMaps (src, s_->maps), s_->selectionSpan (), trackIndex);
}

std::string MidiAgentEditor::prepareSegmentMidi (bool trackOnly)
{
    if (!s_->segment ())
        return "";
    Composition clip = selectionClip (trackOnly ? s_->selectedTrack : -1);
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
    if (!s_->segment ())
        return "";
    Segment span = s_->selectionSpan ();
    std::string path = platform::joinPath (ctrl_->workDir (), platform::safeName (s_->comp.title + "-" + span.name) + ".wav");
    RenderOptions o;
    std::string err;
    if (!renderToWav (s_->comp, &span, o, path, err))
    {
        log ("ERROR " + err);
        return "";
    }
    log ("drag: " + path);
    return path;
}

void MidiAgentEditor::actionRenderWav (bool wholeSong)
{
    Segment span = s_->selectionSpan ();
    const Segment* seg = s_->segment () ? &span : nullptr;
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
        ctrl_->refreshSkill ();
        ctrl_->pushComposition ();
        refreshGenreMenus ();
        refreshSettingsText ();
        log ("skill folder: " + root + " (" + std::to_string (s_->genres.size ()) + " genres)");
    });
    selector->forget ();
}

} // namespace midiagent
