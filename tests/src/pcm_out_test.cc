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
#include "pcm_test_device.h"

#include <chrono>
#include <cstring>
#include <iostream>

#include <gtest/gtest.h>

#include "tinyalsa/pcm.h"

namespace tinyalsa {
namespace testing {

class PcmOutTest : public ::testing::Test {
  protected:
    PcmOutTest() : pcm_object(nullptr) {}
    virtual ~PcmOutTest() = default;

    virtual void SetUp() override {
        pcm_object = pcm_open(kLoopbackCard, kLoopbackPlaybackDevice, PCM_OUT, &kDefaultConfig);
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
        .start_threshold = kDefaultPeriodSize,
        .stop_threshold = kDefaultPeriodSize * kDefaultPeriodCount,
        .silence_threshold = 0,
        .silence_size = 0,
    };

    pcm* pcm_object;
};

TEST_F(PcmOutTest, GetFileDescriptor) {
    ASSERT_GT(pcm_get_file_descriptor(pcm_object), 0);
}

TEST_F(PcmOutTest, GetChannels) {
    ASSERT_EQ(pcm_get_channels(pcm_object), kDefaultConfig.channels);
}

TEST_F(PcmOutTest, GetSamplingRate) {
    ASSERT_EQ(pcm_get_rate(pcm_object), kDefaultConfig.rate);
}

TEST_F(PcmOutTest, GetFormat) {
    ASSERT_EQ(pcm_get_format(pcm_object), kDefaultConfig.format);

}

TEST_F(PcmOutTest, GetErrorMessage) {
    ASSERT_STREQ(pcm_get_error(pcm_object), "");
}

TEST_F(PcmOutTest, GetConfig) {
    ASSERT_EQ(pcm_get_config(nullptr), nullptr);
    ASSERT_EQ(std::memcmp(pcm_get_config(pcm_object), &kDefaultConfig, sizeof(pcm_config)), 0);
}

TEST_F(PcmOutTest, SetConfig) {
    ASSERT_EQ(pcm_set_config(nullptr, nullptr), -EFAULT);
    ASSERT_EQ(pcm_set_config(pcm_object, nullptr), 0);
}

TEST_F(PcmOutTest, GetBufferSize) {
    unsigned int buffer_size = pcm_get_buffer_size(pcm_object);
    ASSERT_EQ(buffer_size, kDefaultConfig.period_count * kDefaultConfig.period_size);
}

TEST_F(PcmOutTest, FramesBytesConvert) {
    unsigned int bytes = pcm_frames_to_bytes(pcm_object, 1);
    ASSERT_EQ(bytes, pcm_format_to_bits(kDefaultConfig.format) / 8 * kDefaultConfig.channels);

    unsigned int frames = pcm_bytes_to_frames(pcm_object, bytes + 1);
    ASSERT_EQ(frames, 1);
}

TEST_F(PcmOutTest, GetAvailableAndTimestamp) {
    unsigned int available = 0;
    timespec time = { 0 };

    ASSERT_LT(pcm_get_htimestamp(nullptr, nullptr, nullptr), 0);

    ASSERT_EQ(pcm_get_htimestamp(pcm_object, &available, &time), 0);
    ASSERT_NE(available, 0);
    // ASSERT_NE(time.tv_nsec | time.tv_sec, 0);
}

TEST_F(PcmOutTest, GetSubdevice) {
    ASSERT_EQ(pcm_get_subdevice(pcm_object), 0);
}

TEST_F(PcmOutTest, Readi) {
    size_t buffer_size = pcm_frames_to_bytes(pcm_object, kDefaultConfig.period_size);
    auto buffer = std::make_unique<char[]>(buffer_size);

    unsigned int frames = pcm_bytes_to_frames(pcm_object, buffer_size);
    ASSERT_EQ(pcm_readi(pcm_object, buffer.get(), frames), -EINVAL);
}

TEST_F(PcmOutTest, Writei) {
    constexpr uint32_t write_count = 20;

    size_t buffer_size = pcm_frames_to_bytes(pcm_object, kDefaultConfig.period_size);
    auto buffer = std::make_unique<char[]>(buffer_size);
    for (uint32_t i = 0; i < buffer_size; ++i) {
        buffer[i] = static_cast<char>(i);
    }

    int written_frames = 0;
    unsigned int frames = pcm_bytes_to_frames(pcm_object, buffer_size);
    auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < write_count; ++i) {
        written_frames = pcm_writei(pcm_object, buffer.get(), frames);
        ASSERT_EQ(written_frames, frames);
    }

    std::chrono::duration<double> difference = std::chrono::steady_clock::now() - start;
    std::chrono::milliseconds expected_elapsed_time_ms(frames *
            (write_count - kDefaultConfig.period_count) / (kDefaultConfig.rate / 1000));

    std::cout << difference.count() << std::endl;
    std::cout << expected_elapsed_time_ms.count() << std::endl;

    ASSERT_NEAR(difference.count() * 1000, expected_elapsed_time_ms.count(), 100);
}

class PcmOutMmapTest : public PcmOutTest {
  protected:
    PcmOutMmapTest() = default;
    ~PcmOutMmapTest() = default;

    virtual void SetUp() override {
        pcm_object = pcm_open(kLoopbackCard, kLoopbackPlaybackDevice, PCM_OUT | PCM_MMAP,
                &kDefaultConfig);
        ASSERT_NE(pcm_object, nullptr);
        ASSERT_TRUE(pcm_is_ready(pcm_object));
    }

    virtual void TearDown() override {
        ASSERT_EQ(pcm_close(pcm_object), 0);
    }
};

TEST_F(PcmOutMmapTest, Write) {
    constexpr uint32_t write_count = 20;

    size_t buffer_size = pcm_frames_to_bytes(pcm_object, kDefaultConfig.period_size);
    auto buffer = std::make_unique<char[]>(buffer_size);
    for (uint32_t i = 0; i < buffer_size; ++i) {
        buffer[i] = static_cast<char>(i);
    }

    int res = 0;
    unsigned int frames = pcm_bytes_to_frames(pcm_object, buffer_size);
    pcm_start(pcm_object);
    auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < write_count; ++i) {
        res = pcm_mmap_write(pcm_object, buffer.get(), buffer_size);
        ASSERT_EQ(res, 0);
    }
    pcm_stop(pcm_object);

    std::chrono::duration<double> difference = std::chrono::steady_clock::now() - start;
    std::chrono::milliseconds expected_elapsed_time_ms(frames *
            (write_count - kDefaultConfig.period_count) / (kDefaultConfig.rate / 1000));

    std::cout << difference.count() << std::endl;
    std::cout << expected_elapsed_time_ms.count() << std::endl;

    ASSERT_NEAR(difference.count() * 1000, expected_elapsed_time_ms.count(), 100);
}

} // namespace testing
} // namespace tinyalsa
