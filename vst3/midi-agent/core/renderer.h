// Offline render: composition (or one segment of it) -> stereo PCM through
// the built-in synth, then a .wav file. Runs on a worker thread in the plug-in
// and in the tests.
#pragma once
#include "core/composition.h"
#include "core/segments.h"

#include <string>
#include <vector>

namespace midiagent {

struct RenderOptions
{
    int sampleRate {44100};
    int bitsPerSample {24};
    double tailSeconds {1.5};
    double normalizePeak {0.89}; // -1 dBFS; 0 disables normalization
    int trackIndex {-1};         // -1 = every audible track
};

struct RenderResult
{
    std::vector<float> left;
    std::vector<float> right;
    double seconds {0.0};
};

RenderResult renderComposition (const Composition& comp, const RenderOptions& options);
/// Render a segment (notes shifted to start at 0) or, with segment == nullptr, the whole song.
bool renderToWav (const Composition& comp, const Segment* segment, const RenderOptions& options,
                  const std::string& path, std::string& error);

} // namespace midiagent
