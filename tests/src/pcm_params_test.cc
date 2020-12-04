/* pcm_params_test.c
**
** Copyright 2020, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#include "pcm_test_device.h"

#include <cstring>
#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include "tinyalsa/pcm.h"

namespace tinyalsa {
namespace testing {

static inline unsigned int OrAllBits(const pcm_mask *mask) {
    static constexpr size_t kTotalMaskBytes = 32;
    unsigned int res = 0;
    for (uint32_t i = 0; i < kTotalMaskBytes / sizeof(pcm_mask::bits[0]); ++i) {
        res |= mask->bits[i];
    }
    return res;
}

TEST(PcmParamsTest, GetAndFreeParams) {
    pcm_params *params = nullptr;

    // test to get nonexistent card and device.
    params = pcm_params_get(1000, 1000, PCM_IN);
    ASSERT_EQ(params, nullptr);

    // test free null params.
    pcm_params_free(params);

    // assume that card 0, device 0 is always available.
    params = pcm_params_get(0, 0, PCM_OUT);
    ASSERT_NE(params, nullptr);
    pcm_params_free(params);
}

TEST(PcmParamsTest, GetParamsBitMask) {
    // test to get mask with null params
    ASSERT_EQ(pcm_params_get_mask(nullptr, PCM_PARAM_ACCESS), nullptr);

    // assume that card 0, device 0 is always available.
    pcm_params *params = pcm_params_get(0, 0, PCM_OUT);
    ASSERT_NE(params, nullptr);

    // test to get param which is not described in bit mask format
    ASSERT_EQ(pcm_params_get_mask(params, PCM_PARAM_SAMPLE_BITS), nullptr);

    // test to get mask out of pcm_param enum
    ASSERT_EQ(pcm_params_get_mask(params, static_cast<pcm_param>(100)), nullptr);

    const pcm_mask *mask = pcm_params_get_mask(params, PCM_PARAM_ACCESS);
    ASSERT_NE(mask, nullptr);

    pcm_params_free(params);
}

TEST(PcmParamsTest, GetParamsInterval) {
    // test to get interval with null params
    ASSERT_EQ(pcm_params_get_min(nullptr, PCM_PARAM_SAMPLE_BITS), 0);
    ASSERT_EQ(pcm_params_get_max(nullptr, PCM_PARAM_SAMPLE_BITS), 0);

    // assume that card 0, device 0 is always available.
    pcm_params *params = pcm_params_get(0, 0, PCM_OUT);
    ASSERT_NE(params, nullptr);

    // test to get param which is not described in interval format
    ASSERT_EQ(pcm_params_get_min(params, PCM_PARAM_ACCESS), 0);
    ASSERT_EQ(pcm_params_get_max(params, PCM_PARAM_ACCESS), 0);

    // test to get interval out of pcm_param enum
    ASSERT_EQ(pcm_params_get_min(params, static_cast<pcm_param>(100)), 0);
    ASSERT_EQ(pcm_params_get_max(params, static_cast<pcm_param>(100)), 0);

    pcm_params_free(params);
}

TEST(PcmParamsTest, ParamsToString) {
    // assume that card 0, device 0 is always available.
    pcm_params *params = pcm_params_get(0, 0, PCM_OUT);
    ASSERT_NE(params, nullptr);

    char long_string[1024] = { 0 };
    int count = pcm_params_to_string(params, long_string, sizeof(long_string));
    ASSERT_LE(static_cast<size_t>(count), sizeof(long_string));
    ASSERT_GT(static_cast<size_t>(count), 0);

    char short_string[1] = { 0 };
    count = pcm_params_to_string(params, short_string, sizeof(short_string));
    ASSERT_GT(static_cast<size_t>(count), sizeof(short_string));

    int proper_string_len = count;
    int proper_string_size = proper_string_len + 1;
    auto proper_string = std::make_unique<char[]>(proper_string_size);
    count = pcm_params_to_string(params, proper_string.get(), proper_string_size);
    ASSERT_GT(static_cast<size_t>(count), 0);
    ASSERT_EQ(static_cast<size_t>(count), proper_string_len);
    ASSERT_EQ(std::strlen(proper_string.get()), proper_string_len);
    pcm_params_free(params);
}

TEST(PcmParamsTest, GetPlaybackDeviceParams) {
    pcm_params *params = pcm_params_get(kLoopbackCard, kLoopbackPlaybackDevice, PCM_OUT);
    ASSERT_NE(params, nullptr);

    const pcm_mask *access_mask = pcm_params_get_mask(params, PCM_PARAM_ACCESS);
    ASSERT_NE(access_mask, nullptr);
    ASSERT_NE(OrAllBits(access_mask), 0);

    const pcm_mask *format_mask = pcm_params_get_mask(params, PCM_PARAM_FORMAT);
    ASSERT_NE(format_mask, nullptr);
    ASSERT_NE(OrAllBits(format_mask), 0);

    const pcm_mask *subformat_mask = pcm_params_get_mask(params, PCM_PARAM_SUBFORMAT);
    ASSERT_NE(subformat_mask, nullptr);
    ASSERT_NE(OrAllBits(subformat_mask), 0);

    unsigned int sample_bits_min = pcm_params_get_min(params, PCM_PARAM_SAMPLE_BITS);
    unsigned int sample_bits_max = pcm_params_get_max(params, PCM_PARAM_SAMPLE_BITS);
    std::cout << "sample_bits: " << sample_bits_min << " - " << sample_bits_max << std::endl;
    ASSERT_GT(sample_bits_min, 0);
    ASSERT_GT(sample_bits_max, 0);

    unsigned int frame_bits_min = pcm_params_get_min(params, PCM_PARAM_FRAME_BITS);
    unsigned int frame_bits_max = pcm_params_get_max(params, PCM_PARAM_FRAME_BITS);
    std::cout << "frame_bits: " << frame_bits_min << " - " << frame_bits_max << std::endl;
    ASSERT_GT(frame_bits_min, 0);
    ASSERT_GT(frame_bits_max, 0);

    unsigned int channels_min = pcm_params_get_min(params, PCM_PARAM_CHANNELS);
    unsigned int channels_max = pcm_params_get_max(params, PCM_PARAM_CHANNELS);
    std::cout << "channels: " << channels_min << " - " << channels_max << std::endl;
    ASSERT_GT(channels_min, 0);
    ASSERT_GT(channels_max, 0);

    unsigned int sampling_rate_min = pcm_params_get_min(params, PCM_PARAM_RATE);
    unsigned int sampling_rate_max = pcm_params_get_max(params, PCM_PARAM_RATE);
    std::cout << "sampling_rate: " << sampling_rate_min << " - " << sampling_rate_max << std::endl;
    ASSERT_GT(sampling_rate_min, 0);
    ASSERT_GT(sampling_rate_max, 0);

    unsigned int period_time_min = pcm_params_get_min(params, PCM_PARAM_PERIOD_TIME);
    unsigned int period_time_max = pcm_params_get_max(params, PCM_PARAM_PERIOD_TIME);
    std::cout << "period_time: " << period_time_min << " - " << period_time_max << std::endl;
    ASSERT_GT(period_time_min, 0);
    ASSERT_GT(period_time_max, 0);

    unsigned int period_size_min = pcm_params_get_min(params, PCM_PARAM_PERIOD_SIZE);
    unsigned int period_size_max = pcm_params_get_max(params, PCM_PARAM_PERIOD_SIZE);
    std::cout << "period_size: " << period_size_min << " - " << period_size_max << std::endl;
    ASSERT_GT(period_size_min, 0);
    ASSERT_GT(period_size_max, 0);

    unsigned int period_bytes_min = pcm_params_get_min(params, PCM_PARAM_PERIOD_BYTES);
    unsigned int period_bytes_max = pcm_params_get_max(params, PCM_PARAM_PERIOD_BYTES);
    std::cout << "period_bytes: " << period_bytes_min << " - " << period_bytes_max << std::endl;
    ASSERT_GT(period_bytes_min, 0);
    ASSERT_GT(period_bytes_max, 0);

    unsigned int period_count_min = pcm_params_get_min(params, PCM_PARAM_PERIODS);
    unsigned int period_count_max = pcm_params_get_max(params, PCM_PARAM_PERIODS);
    std::cout << "period_count: " << period_count_min << " - " << period_count_max << std::endl;
    ASSERT_GT(period_count_min, 0);
    ASSERT_GT(period_count_max, 0);

    unsigned int buffer_time_min = pcm_params_get_min(params, PCM_PARAM_BUFFER_TIME);
    unsigned int buffer_time_max = pcm_params_get_max(params, PCM_PARAM_BUFFER_TIME);
    std::cout << "buffer_time: " << buffer_time_min << " - " << buffer_time_max << std::endl;
    ASSERT_GT(buffer_time_min, 0);
    ASSERT_GT(buffer_time_max, 0);

    unsigned int buffer_size_min = pcm_params_get_min(params, PCM_PARAM_BUFFER_SIZE);
    unsigned int buffer_size_max = pcm_params_get_max(params, PCM_PARAM_BUFFER_SIZE);
    std::cout << "buffer_size: " << buffer_size_min << " - " << buffer_size_max << std::endl;
    ASSERT_GT(buffer_size_min, 0);
    ASSERT_GT(buffer_size_max, 0);

    unsigned int buffer_bytes_min = pcm_params_get_min(params, PCM_PARAM_BUFFER_BYTES);
    unsigned int buffer_bytes_max = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    std::cout << "buffer_bytes: " << buffer_bytes_min << " - " << buffer_bytes_max << std::endl;
    ASSERT_GT(buffer_bytes_min, 0);
    ASSERT_GT(buffer_bytes_max, 0);

    unsigned int tick_in_us_min = pcm_params_get_min(params, PCM_PARAM_TICK_TIME);
    unsigned int tick_in_us_max = pcm_params_get_max(params, PCM_PARAM_TICK_TIME);
    ASSERT_GT(tick_in_us_max, 0);
    std::cout << "tick_in_us: " << tick_in_us_min << " - " << tick_in_us_max << std::endl;

    pcm_params_free(params);
}

} // namespace testing
} // namespace tinyalsa
