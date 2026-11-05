#include "core/wavfile.h"
#include "core/platform.h"

#include <algorithm>
#include <cmath>

namespace midiagent {

namespace {
void put16 (std::vector<uint8_t>& o, uint32_t v)
{
    o.push_back (static_cast<uint8_t> (v));
    o.push_back (static_cast<uint8_t> (v >> 8));
}
void put32 (std::vector<uint8_t>& o, uint32_t v)
{
    put16 (o, v & 0xFFFF);
    put16 (o, v >> 16);
}
} // namespace

std::vector<uint8_t> encodeWav (const std::vector<float>& left, const std::vector<float>& right, int sampleRate,
                                int bitsPerSample)
{
    if (bitsPerSample != 16)
        bitsPerSample = 24;
    const uint32_t channels = 2;
    const uint32_t frames = static_cast<uint32_t> (std::min (left.size (), right.size ()));
    const uint32_t bytesPerSample = static_cast<uint32_t> (bitsPerSample / 8);
    const uint32_t dataBytes = frames * channels * bytesPerSample;

    std::vector<uint8_t> o;
    o.reserve (44 + dataBytes);
    o.insert (o.end (), {'R', 'I', 'F', 'F'});
    put32 (o, 36 + dataBytes);
    o.insert (o.end (), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put32 (o, 16);
    put16 (o, 1); // PCM
    put16 (o, channels);
    put32 (o, static_cast<uint32_t> (sampleRate));
    put32 (o, static_cast<uint32_t> (sampleRate) * channels * bytesPerSample);
    put16 (o, channels * bytesPerSample);
    put16 (o, static_cast<uint32_t> (bitsPerSample));
    o.insert (o.end (), {'d', 'a', 't', 'a'});
    put32 (o, dataBytes);

    const double scale = bitsPerSample == 16 ? 32767.0 : 8388607.0;
    for (uint32_t i = 0; i < frames; ++i)
    {
        const float samples[2] = {left[i], right[i]};
        for (float s : samples)
        {
            double v = std::max (-1.0, std::min (1.0, static_cast<double> (s)));
            int32_t q = static_cast<int32_t> (std::lround (v * scale));
            if (bitsPerSample == 16)
                put16 (o, static_cast<uint32_t> (static_cast<uint16_t> (q)));
            else
            {
                uint32_t u = static_cast<uint32_t> (q);
                o.push_back (static_cast<uint8_t> (u));
                o.push_back (static_cast<uint8_t> (u >> 8));
                o.push_back (static_cast<uint8_t> (u >> 16));
            }
        }
    }
    return o;
}

bool writeWavFile (const std::string& path, const std::vector<float>& left, const std::vector<float>& right,
                   int sampleRate, int bitsPerSample, std::string& error)
{
    return platform::writeFile (path, encodeWav (left, right, sampleRate, bitsPerSample), error);
}

} // namespace midiagent
