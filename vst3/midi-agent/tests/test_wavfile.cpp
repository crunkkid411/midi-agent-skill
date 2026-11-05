#include "core/wavfile.h"
#include "tests/test_framework.h"

using namespace midiagent;

static uint32_t le32 (const std::vector<uint8_t>& d, size_t at)
{
    return d[at] | (d[at + 1] << 8) | (d[at + 2] << 16) | (uint32_t (d[at + 3]) << 24);
}

TEST_CASE (wav_header_is_consistent)
{
    std::vector<float> l = {0.0f, 0.5f, -0.5f, 1.0f};
    std::vector<float> r = {0.0f, -0.5f, 0.5f, -1.0f};
    std::vector<uint8_t> d = encodeWav (l, r, 48000, 24);
    CHECK_EQ (d.size (), size_t (44 + 4 * 2 * 3));
    CHECK_EQ (std::string (d.begin (), d.begin () + 4), std::string ("RIFF"));
    CHECK_EQ (le32 (d, 4), uint32_t (d.size () - 8));
    CHECK_EQ (le32 (d, 24), 48000u);
    CHECK_EQ (le32 (d, 28), 48000u * 6);
    CHECK_EQ (d[34], 24);
    CHECK_EQ (le32 (d, 40), 24u);
    // sample 3 left = +1.0 -> 0x7FFFFF
    size_t s3 = 44 + 3 * 6;
    CHECK_EQ (d[s3], 0xFF);
    CHECK_EQ (d[s3 + 1], 0xFF);
    CHECK_EQ (d[s3 + 2], 0x7F);
    std::vector<uint8_t> d16 = encodeWav (l, r, 44100, 16);
    CHECK_EQ (d16.size (), size_t (44 + 4 * 2 * 2));
    CHECK_EQ (d16[34], 16);
}
