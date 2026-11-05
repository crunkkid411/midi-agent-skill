// Turns a composition and a musical time range into note on/off events with
// sample offsets. The processor calls it once per audio block with the host
// position (ProcessContext::projectTimeMusic) or with an internal clock when
// previewing a segment while the transport is stopped.
#pragma once
#include "core/composition.h"

#include <memory>
#include <vector>

namespace midiagent {

struct SeqEvent
{
    int sampleOffset {0};
    int track {0};
    int pitch {60};
    float velocity {1.0f}; // 0..1
    bool noteOn {true};
};

class Sequencer
{
public:
    /// The sequencer keeps a shared pointer, so the composition may be swapped
    /// from another thread by publishing a new shared_ptr.
    void setComposition (std::shared_ptr<const Composition> comp);
    const Composition* composition () const { return comp_.get (); }

    /// Events for notes that start or end in [fromTick, toTick), with sample
    /// offsets computed from samplesPerTick. Mutes: notes of a muted track are
    /// skipped (their note offs still fire). A solo set makes mute apply to all
    /// other tracks.
    void collect (int64_t fromTick, int64_t toTick, double samplesPerTick, int blockOffset,
                  std::vector<SeqEvent>& out);
    /// Note offs for every note currently held, at sampleOffset `at`.
    void flush (int at, std::vector<SeqEvent>& out);
    bool anyHeld () const { return !held_.empty (); }

    void setTrackMute (size_t track, bool mute);
    void setTrackSolo (size_t track, bool solo);
    void clearSolo ();
    bool trackAudible (size_t track) const;

private:
    struct Held
    {
        int track;
        int pitch;
        int64_t end;
    };
    std::shared_ptr<const Composition> comp_;
    std::vector<Held> held_;
    std::vector<bool> mute_;
    std::vector<bool> solo_;
    bool anySolo_ {false};
};

} // namespace midiagent
