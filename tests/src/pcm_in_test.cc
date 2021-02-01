/* pcm_in_test.c
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

#include <chrono>
#include <cstring>
#include <iostream>

#include <gtest/gtest.h>

#include "tinyalsa/pcm.h"

namespace tinyalsa {
namespace testing {

class PcmInTest : public ::testing::Test {
  protected:
    PcmInTest() : pcm_object(nullptr) {}
    virtual ~PcmInTest() = default;

    virtual void SetUp() override {
        pcm_object = pcm_open(kLoopbackCard, kLoopbackCaptureDevice, PCM_IN, &kDefaultConfig);
        ASSERT_NE(pcm_object, nullptr);
        ASSERT_TRUE(pcm_is_ready(pcm_object));
    }

    virtual void TearDown() override {
        ASSERT_EQ(pcm_close(pcm_object), 0);
    }

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
        .start_threshold = 0,
        .stop_threshold = 0,
        .silence_threshold = 0,
        .silence_size = 0,
    };

    pcm* pcm_object;
};

TEST_F(PcmInTest, GetDelay) {
    pcm_prepare(pcm_object);
    long delay = pcm_get_delay(pcm_object);
    std::cout << delay << std::endl;
    ASSERT_GE(delay, 0);
}

TEST_F(PcmInTest, Readi) {
    constexpr uint32_t read_count = 20;

    size_t buffer_size = pcm_frames_to_bytes(pcm_object, kDefaultConfig.period_size);
    auto buffer = std::make_unique<char[]>(buffer_size);

    int read_frames = 0;
    unsigned int frames = pcm_bytes_to_frames(pcm_object, buffer_size);
    auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < read_count; ++i) {
        read_frames = pcm_readi(pcm_object, buffer.get(), frames);
        ASSERT_EQ(read_frames, frames);
    }

    std::chrono::duration<double> difference = std::chrono::steady_clock::now() - start;
    std::chrono::milliseconds expected_elapsed_time_ms(frames * read_count /
            (kDefaultConfig.rate / 1000));

    std::cout << difference.count() << std::endl;
    std::cout << expected_elapsed_time_ms.count() << std::endl;

    ASSERT_NEAR(difference.count() * 1000, expected_elapsed_time_ms.count(), 100);
}

TEST_F(PcmInTest, Writei) {
    size_t buffer_size = pcm_frames_to_bytes(pcm_object, kDefaultConfig.period_size);
    auto buffer = std::make_unique<char[]>(buffer_size);

    unsigned int frames = pcm_bytes_to_frames(pcm_object, buffer_size);
    ASSERT_EQ(pcm_writei(pcm_object, buffer.get(), frames), -EINVAL);
}

} // namespace testing
} // namespace tinyalsa
