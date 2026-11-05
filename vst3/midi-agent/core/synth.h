// A small polyphonic synth so the plug-in makes sound on its own: the
// Cubase "Render in Place" and "Export Audio Mixdown" paths produce a .wav
// from it, and the offline renderer uses it to write .wav files for dragging.
// It is a sketch instrument, not a sound design tool: one timbre per track
// role, a drum voice per General MIDI drum note.
#pragma once
#include "core/composition.h"

#include <array>
#include <cstdint>
#include <string>

namespace midiagent {

enum class Timbre
{
    Saw,     // chords, rhythm guitar
    Square,  // lead, melody
    Pluck,   // short decaying saw
    SubBass, // bass, synth bass: filtered saw + sine
    Pad,     // slow attack sine + triangle
    Drums,   // General MIDI drum map
};

/// Timbre from a track's role, then its instrument name, then a default.
Timbre timbreForTrack (const Track& track);
const char* timbreName (Timbre t);

class Synth
{
public:
    static constexpr int kMaxVoices = 48;

    Synth ();
    void setSampleRate (double sampleRate);
    double sampleRate () const { return sampleRate_; }
    void reset ();

    void noteOn (int track, int pitch, float velocity, Timbre timbre, float pan);
    void noteOff (int track, int pitch);
    void allNotesOff ();
    /// Render and ADD into the buffers.
    void render (float* left, float* right, int frames);
    bool isSilent () const;
    int activeVoices () const;
    void setGain (float gain) { gain_ = gain; }

private:
    struct Voice
    {
        bool active {false};
        bool released {false};
        int track {-1};
        int pitch {60};
        Timbre timbre {Timbre::Saw};
        double phase {0.0};
        double phase2 {0.0};
        double freq {440.0};
        double env {0.0};
        double age {0.0}; // seconds since note on
        float velocity {1.0f};
        float panL {0.7f};
        float panR {0.7f};
        double lp {0.0}; // one pole filter state
        uint32_t noise {0x12345678u};
        int64_t startOrder {0};
    };

    float nextNoise (Voice& v);
    float renderVoice (Voice& v);
    Voice* allocate ();

    double sampleRate_ {44100.0};
    float gain_ {0.5f};
    int64_t order_ {0};
    std::array<Voice, kMaxVoices> voices_;
};

} // namespace midiagent
