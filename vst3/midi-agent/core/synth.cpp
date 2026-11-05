#include "core/synth.h"

#include <algorithm>
#include <cmath>

namespace midiagent {

namespace {
constexpr double kPi = 3.14159265358979323846;

bool contains (const std::string& s, const char* needle)
{
    return s.find (needle) != std::string::npos;
}
} // namespace

Timbre timbreForTrack (const Track& track)
{
    if (track.isDrum)
        return Timbre::Drums;
    const std::string& r = track.role;
    if (r == "bass" || r == "synth_bass")
        return Timbre::SubBass;
    if (r == "pad")
        return Timbre::Pad;
    if (r == "lead" || r == "melody")
        return Timbre::Square;
    if (r == "chords" || r == "rhythm_guitar")
        return Timbre::Saw;
    const std::string& i = track.instrument;
    if (contains (i, "bass"))
        return Timbre::SubBass;
    if (contains (i, "pad") || contains (i, "string") || contains (i, "choir") || contains (i, "organ"))
        return Timbre::Pad;
    if (contains (i, "square") || contains (i, "lead") || contains (i, "flute") || contains (i, "sax"))
        return Timbre::Square;
    if (contains (i, "piano") || contains (i, "guitar") || contains (i, "pluck") || contains (i, "harp") ||
        contains (i, "bell") || contains (i, "marimba"))
        return Timbre::Pluck;
    return Timbre::Saw;
}

const char* timbreName (Timbre t)
{
    switch (t)
    {
        case Timbre::Saw: return "SAW";
        case Timbre::Square: return "SQR";
        case Timbre::Pluck: return "PLK";
        case Timbre::SubBass: return "SUB";
        case Timbre::Pad: return "PAD";
        case Timbre::Drums: return "DRM";
    }
    return "?";
}

Synth::Synth ()
{
    reset ();
}

void Synth::setSampleRate (double sampleRate)
{
    sampleRate_ = sampleRate > 1000.0 ? sampleRate : 44100.0;
}

void Synth::reset ()
{
    for (auto& v : voices_)
        v = Voice {};
    order_ = 0;
}

Synth::Voice* Synth::allocate ()
{
    Voice* best = nullptr;
    for (auto& v : voices_)
        if (!v.active)
            return &v;
    // steal the oldest released voice, else the oldest voice
    for (auto& v : voices_)
        if (v.released && (!best || v.startOrder < best->startOrder))
            best = &v;
    if (best)
        return best;
    for (auto& v : voices_)
        if (!best || v.startOrder < best->startOrder)
            best = &v;
    return best;
}

void Synth::noteOn (int track, int pitch, float velocity, Timbre timbre, float pan)
{
    Voice* v = allocate ();
    if (!v)
        return;
    *v = Voice {};
    v->active = true;
    v->track = track;
    v->pitch = pitch;
    v->timbre = timbre;
    v->velocity = std::max (0.05f, std::min (1.0f, velocity));
    v->freq = 440.0 * std::pow (2.0, (pitch - 69) / 12.0);
    v->startOrder = ++order_;
    float p = std::max (-1.0f, std::min (1.0f, pan));
    v->panL = std::cos ((p + 1.0f) * 0.25f * static_cast<float> (kPi));
    v->panR = std::sin ((p + 1.0f) * 0.25f * static_cast<float> (kPi));
    v->noise = 0x9E3779B9u ^ static_cast<uint32_t> (order_ * 2654435761u);
}

void Synth::noteOff (int track, int pitch)
{
    for (auto& v : voices_)
        if (v.active && v.track == track && v.pitch == pitch && !v.released)
            v.released = true;
}

void Synth::allNotesOff ()
{
    for (auto& v : voices_)
        if (v.active)
            v.released = true;
}

bool Synth::isSilent () const
{
    for (const auto& v : voices_)
        if (v.active)
            return false;
    return true;
}

int Synth::activeVoices () const
{
    int n = 0;
    for (const auto& v : voices_)
        if (v.active)
            ++n;
    return n;
}

float Synth::nextNoise (Voice& v)
{
    v.noise ^= v.noise << 13;
    v.noise ^= v.noise >> 17;
    v.noise ^= v.noise << 5;
    return static_cast<float> (static_cast<int32_t> (v.noise)) / 2147483648.0f;
}

float Synth::renderVoice (Voice& v)
{
    const double dt = 1.0 / sampleRate_;
    v.age += dt;

    if (v.timbre == Timbre::Drums)
    {
        // One-shot drum voices: the envelope ignores note off.
        int p = v.pitch;
        float out = 0.0f;
        double decay;
        if (p == 35 || p == 36) // kick: sine sweep 160 -> 45 Hz
        {
            decay = 0.28;
            double f = 45.0 + 115.0 * std::exp (-v.age * 28.0);
            v.phase += f * dt;
            out = static_cast<float> (std::sin (2.0 * kPi * v.phase)) * 1.3f;
        }
        else if (p == 38 || p == 40 || p == 37) // snare / clap: noise + 190 Hz body
        {
            decay = p == 37 ? 0.12 : 0.19;
            v.phase += 190.0 * dt;
            out = nextNoise (v) * 0.8f + static_cast<float> (std::sin (2.0 * kPi * v.phase)) * 0.4f;
        }
        else if (p == 42 || p == 44) // closed hat: bright short noise
        {
            decay = 0.045;
            float n = nextNoise (v);
            out = (n - static_cast<float> (v.lp)) * 0.7f;
            v.lp = n;
        }
        else if (p == 46) // open hat
        {
            decay = 0.32;
            float n = nextNoise (v);
            out = (n - static_cast<float> (v.lp)) * 0.6f;
            v.lp = n;
        }
        else if (p == 49 || p == 57 || p == 52 || p == 55) // crash / china / splash
        {
            decay = 1.1;
            float n = nextNoise (v);
            out = (n - static_cast<float> (v.lp)) * 0.5f;
            v.lp = n;
        }
        else if (p == 51 || p == 59 || p == 53) // ride
        {
            decay = 0.45;
            float n = nextNoise (v);
            v.phase += 2600.0 * dt;
            out = (n - static_cast<float> (v.lp)) * 0.35f + static_cast<float> (std::sin (2.0 * kPi * v.phase)) * 0.15f;
            v.lp = n;
        }
        else if (p >= 41 && p <= 50) // toms
        {
            decay = 0.3;
            double base = 80.0 + (p - 41) * 18.0;
            double f = base + base * std::exp (-v.age * 20.0);
            v.phase += f * dt;
            out = static_cast<float> (std::sin (2.0 * kPi * v.phase)) * 1.0f;
        }
        else // percussion: short click
        {
            decay = 0.08;
            out = nextNoise (v) * 0.6f;
        }
        double env = std::exp (-v.age / (decay * 0.4));
        if (v.age > decay * 2.5)
            v.active = false;
        return out * static_cast<float> (env) * v.velocity;
    }

    // Melodic voices: osc + ADSR + one pole low pass.
    double attack, decay, sustain, release;
    switch (v.timbre)
    {
        case Timbre::Pad: attack = 0.35; decay = 0.4; sustain = 0.85; release = 0.6; break;
        case Timbre::Pluck: attack = 0.002; decay = 0.35; sustain = 0.0; release = 0.12; break;
        case Timbre::SubBass: attack = 0.004; decay = 0.2; sustain = 0.8; release = 0.08; break;
        case Timbre::Square: attack = 0.006; decay = 0.15; sustain = 0.7; release = 0.1; break;
        default: attack = 0.008; decay = 0.2; sustain = 0.75; release = 0.15; break;
    }
    if (!v.released)
    {
        if (v.age < attack)
            v.env = v.age / attack;
        else
        {
            double d = (v.age - attack) / decay;
            v.env = sustain + (1.0 - sustain) * std::exp (-d * 3.0);
        }
    }
    else
    {
        v.env -= dt / release;
        if (v.env <= 0.0)
        {
            v.env = 0.0;
            v.active = false;
            return 0.0f;
        }
    }

    double inc = v.freq * dt;
    v.phase += inc;
    if (v.phase >= 1.0)
        v.phase -= 1.0;
    float osc = 0.0f;
    switch (v.timbre)
    {
        case Timbre::Saw:
        case Timbre::Pluck:
        {
            v.phase2 += inc * 1.004; // slight detune
            if (v.phase2 >= 1.0)
                v.phase2 -= 1.0;
            osc = static_cast<float> ((v.phase * 2.0 - 1.0) * 0.5 + (v.phase2 * 2.0 - 1.0) * 0.5);
            break;
        }
        case Timbre::Square:
            osc = v.phase < 0.5 ? 0.8f : -0.8f;
            break;
        case Timbre::SubBass:
        {
            float saw = static_cast<float> (v.phase * 2.0 - 1.0);
            float sine = static_cast<float> (std::sin (2.0 * kPi * v.phase));
            osc = saw * 0.5f + sine * 0.7f;
            break;
        }
        case Timbre::Pad:
        {
            float sine = static_cast<float> (std::sin (2.0 * kPi * v.phase));
            float tri = static_cast<float> (1.0 - 4.0 * std::fabs (v.phase - 0.5));
            v.phase2 += inc * 0.5015; // sub octave, detuned
            if (v.phase2 >= 1.0)
                v.phase2 -= 1.0;
            float sub = static_cast<float> (std::sin (2.0 * kPi * v.phase2));
            osc = sine * 0.5f + tri * 0.3f + sub * 0.3f;
            break;
        }
        default: break;
    }
    // low pass: brighter with velocity and at the start of the note
    double cutoff;
    switch (v.timbre)
    {
        case Timbre::SubBass: cutoff = 300.0 + 900.0 * v.velocity * std::exp (-v.age * 6.0); break;
        case Timbre::Pluck: cutoff = 600.0 + 5000.0 * v.velocity * std::exp (-v.age * 5.0); break;
        case Timbre::Pad: cutoff = 900.0 + 1400.0 * v.env; break;
        default: cutoff = 1200.0 + 6000.0 * v.velocity; break;
    }
    double a = 1.0 - std::exp (-2.0 * kPi * cutoff * dt);
    v.lp += a * (osc - v.lp);
    return static_cast<float> (v.lp * v.env) * v.velocity;
}

void Synth::render (float* left, float* right, int frames)
{
    for (auto& v : voices_)
    {
        if (!v.active)
            continue;
        for (int i = 0; i < frames && v.active; ++i)
        {
            float s = renderVoice (v) * gain_;
            left[i] += s * v.panL;
            right[i] += s * v.panR;
        }
    }
}

} // namespace midiagent
