// RIFF/WAVE PCM writer (16 or 24 bit, stereo).
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace midiagent {

std::vector<uint8_t> encodeWav (const std::vector<float>& left, const std::vector<float>& right, int sampleRate,
                                int bitsPerSample = 24);
bool writeWavFile (const std::string& path, const std::vector<float>& left, const std::vector<float>& right,
                   int sampleRate, int bitsPerSample, std::string& error);

} // namespace midiagent
