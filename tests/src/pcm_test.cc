/* pcm_out_test.c
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

#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include "tinyalsa/pcm.h"

namespace tinyalsa {
namespace testing {

TEST(PcmTest, FormatToBits) {
    // FIXME: Should we return 16 bits for INVALID?
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_INVALID), 16);

    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S16_LE), 16);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S32_LE), 32);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S8), 8);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S24_LE), 32);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S24_3LE), 24);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S16_BE), 16);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S24_BE), 32);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S24_3BE), 24);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_S32_BE), 32);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_FLOAT_LE), 32);
    ASSERT_EQ(pcm_format_to_bits(PCM_FORMAT_FLOAT_BE), 32);
}

TEST(PcmTest, OpenAndCloseOutPcm) {
    static constexpr unsigned int kDefaultChannels = 2;
    static constexpr unsigned int kDefaultSamplingRate = 48000;
    static constexpr unsigned int kDefaultPeriodSize = 1024;
    static constexpr unsigned int kDefaultPeriodCount = 3;
    static constexpr pcm_config kDefaultConfig = {
        .channels = kDefaultChannels,
        .rate = kDefaultSamplingRate,
        .period_size = kDefaultPeriodSize,
        .period_count = kDefaultPeriodCount,
        .format = PCM_FORMAT_S16_LE,
        .start_threshold = kDefaultPeriodSize,
        .stop_threshold = kDefaultPeriodSize * kDefaultPeriodCount,
        .silence_threshold = 0,
        .silence_size = 0,
    };

    pcm *pcm_object = pcm_open(1000, 1000, PCM_OUT, &kDefaultConfig);
    ASSERT_FALSE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);

    // assume card 0, device 0 is always available
    pcm_object = pcm_open(0, 0, PCM_OUT, &kDefaultConfig);
    ASSERT_TRUE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);

    pcm_object = pcm_open(0, 0, PCM_OUT | PCM_MMAP, &kDefaultConfig);
    ASSERT_TRUE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);

    pcm_object = pcm_open(0, 0, PCM_OUT | PCM_MMAP | PCM_NOIRQ, &kDefaultConfig);
    ASSERT_TRUE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);

    pcm_object = pcm_open(0, 0, PCM_OUT | PCM_NONBLOCK, &kDefaultConfig);
    ASSERT_TRUE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);

    pcm_object = pcm_open(0, 0, PCM_OUT | PCM_MONOTONIC, &kDefaultConfig);
    ASSERT_TRUE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);

    std::string name = "hw:0,0";
    pcm_object = pcm_open_by_name(name.c_str(), PCM_OUT, &kDefaultConfig);
    ASSERT_TRUE(pcm_is_ready(pcm_object));
    ASSERT_EQ(pcm_close(pcm_object), 0);
}

} // namespace testing
} // namespace tinyalsa
