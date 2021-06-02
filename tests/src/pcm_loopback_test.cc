/* pcm_loopback_test.c
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
#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>

#include <gtest/gtest.h>

#include "tinyalsa/pcm.h"

namespace tinyalsa {
namespace testing {

template<int32_t CH, int32_t SR, pcm_format F>
class SilenceGenerator {
public:
    pcm_format GetFormat() {
        return F;
    }

    int32_t GetChannels() {
        return CH;
    };

    int32_t GetSamplingRate() {
        return SR;
    };

    virtual int32_t Read(void *buffer, int32_t size) {
        std::memset(buffer, 0, size);
        return size;
    }
};

template<pcm_format F>
struct PcmFormat {
    using Type = void;
    static constexpr pcm_format kFormat = F;
    static constexpr int32_t kMax = 0;
    static constexpr int32_t kMin = 0;
};

template<>
struct PcmFormat<PCM_FORMAT_S16_LE> {
    using Type = int16_t;
    static constexpr pcm_format kFormat = PCM_FORMAT_S16_LE;
    static constexpr Type kMax = std::numeric_limits<Type>::max();
    static constexpr Type kMin = std::numeric_limits<Type>::min();
};

template<>
struct PcmFormat<PCM_FORMAT_FLOAT_LE> {
    using Type = float;
    static constexpr pcm_format kFormat = PCM_FORMAT_FLOAT_LE;
    static constexpr Type kMax = 1.0;
    static constexpr Type kMin = -1.0;
};

// CH: channels
// SR: sampling rate
// FQ: sine wave frequency
// L: max level
template<int32_t CH, int32_t SR, int32_t FQ, int32_t L, pcm_format F>
class SineToneGenerator : public SilenceGenerator<CH, SR, F> {
private:
    using Type = typename PcmFormat<F>::Type;
    static constexpr double kPi = M_PI;
    static constexpr double kStep = FQ * CH * kPi / SR;

    double channels[CH];
    double gain;

    Type GetSample(double radian) {
        double sine = std::sin(radian) * gain;
        if (sine >= 1.0) {
            return PcmFormat<F>::kMax;
        } else if (sine <= -1.0) {
            return PcmFormat<F>::kMin;
        }
        return static_cast<Type>(sine * PcmFormat<F>::kMax);
    }

public:
    SineToneGenerator() {
        constexpr double phase = (CH == 1) ? 0 : kPi / 2 / (CH - 1);

        channels[0] = 0.0;
        for (int32_t i = 1; i < CH; ++i) {
            channels[i] = channels[i - 1] + phase;
        }

        gain = std::pow(M_E, std::log(10) * static_cast<double>(L) / 20.0);
    }

    ~SineToneGenerator() = default;

    int32_t Read(void *buffer, int32_t size) override {
        Type *pcm_buffer = reinterpret_cast<Type *>(buffer);

        size = (size / (CH * sizeof(Type))) * (CH * sizeof(Type));
        int32_t samples = size / sizeof(Type);
        int32_t s = 0;

        while (s < samples) {
            for (int32_t i = 0; i < CH; ++i) {
                pcm_buffer[s++] = GetSample(channels[i]);
                channels[i] += kStep;
            }
        }
        return size;
    }
};

template<typename T>
static double Energy(T *buffer, size_t samples) {
    double sum = 0.0;
    for (size_t i = 0; i < samples; i++) {
        sum += static_cast<double>(buffer[i]) * static_cast<double>(buffer[i]);
    }
    return sum;
}

template<typename F>
class PcmLoopbackTest : public ::testing::Test {
  protected:
    PcmLoopbackTest() = default;
    virtual ~PcmLoopbackTest() = default;

    void SetUp() override {
        static constexpr pcm_config kInConfig = {
            .channels = kDefaultChannels,
            .rate = kDefaultSamplingRate,
            .period_size = kDefaultPeriodSize,
            .period_count = kDefaultPeriodCount,
            .format = kPcmForamt,
            .start_threshold = 0,
            .stop_threshold = 0,
            .silence_threshold = 0,
            .silence_size = 0,
        };
        pcm_in = pcm_open(kLoopbackCard, kLoopbackCaptureDevice, PCM_IN, &kInConfig);
        ASSERT_TRUE(pcm_is_ready(pcm_in));

        static constexpr pcm_config kOutConfig = {
            .channels = kDefaultChannels,
            .rate = kDefaultSamplingRate,
            .period_size = kDefaultPeriodSize,
            .period_count = kDefaultPeriodCount,
            .format = kPcmForamt,
            .start_threshold = kDefaultPeriodSize,
            .stop_threshold = kDefaultPeriodSize * kDefaultPeriodCount,
            .silence_threshold = 0,
            .silence_size = 0,
        };
        pcm_out = pcm_open(kLoopbackCard, kLoopbackPlaybackDevice, PCM_OUT, &kOutConfig);
        ASSERT_TRUE(pcm_is_ready(pcm_out));
        ASSERT_EQ(pcm_link(pcm_in, pcm_out), 0);
    }

    void TearDown() override {
        ASSERT_EQ(pcm_unlink(pcm_in), 0);
        pcm_close(pcm_in);
        pcm_close(pcm_out);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    static constexpr unsigned int kDefaultPeriodTimeInMs =
            kDefaultPeriodSize * 1000 / kDefaultSamplingRate;
    static constexpr pcm_format kPcmForamt = F::kFormat;
    pcm *pcm_in;
    pcm *pcm_out;
};

using S16bitlePcmFormat = PcmFormat<PCM_FORMAT_S16_LE>;
using FloatPcmFormat = PcmFormat<PCM_FORMAT_FLOAT_LE>;

using Formats = ::testing::Types<S16bitlePcmFormat, FloatPcmFormat>;

TYPED_TEST_SUITE(PcmLoopbackTest, Formats);

TYPED_TEST(PcmLoopbackTest, Loopback) {
    static constexpr unsigned int kDefaultPeriodTimeInMs = this->kDefaultPeriodTimeInMs;
    static constexpr pcm_format kPcmForamt = this->kPcmForamt;
    pcm *pcm_in = this->pcm_in;
    pcm *pcm_out = this->pcm_out;

    bool stopping = false;
    ASSERT_EQ(pcm_get_subdevice(pcm_in), pcm_get_subdevice(pcm_out));

    std::thread capture([pcm_in, &stopping] {
        size_t buffer_size = pcm_frames_to_bytes(pcm_in, kDefaultPeriodSize);
        unsigned int frames = pcm_bytes_to_frames(pcm_in, buffer_size);
        auto buffer = std::make_unique<unsigned char[]>(buffer_size);
        int32_t counter = 0;
        while (!stopping) {
            int res = pcm_readi(pcm_in, buffer.get(), frames);
            if (res == -1) {
                std::cout << pcm_get_error(pcm_in) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(kDefaultPeriodTimeInMs));
                counter++;
                continue;
            }

            // Test the energy of the buffer after the sine tone samples fill in the buffer.
            // Therefore, check the buffer 5 times later.
            if (counter >= 5) {
                double e = Energy(buffer.get(), frames * kDefaultChannels);
                EXPECT_GT(e, 0.0) << counter;
            }
            counter++;
        }
        std::cout << "read count = " << counter << std::endl;
    });

    std::thread playback([pcm_out, &stopping] {
        SineToneGenerator<kDefaultChannels, kDefaultSamplingRate, 1000, 0, kPcmForamt> generator;
        size_t buffer_size = pcm_frames_to_bytes(pcm_out, kDefaultPeriodSize);
        unsigned int frames = pcm_bytes_to_frames(pcm_out, buffer_size);
        std::cout << buffer_size << std::endl;
        auto buffer = std::make_unique<unsigned char[]>(buffer_size);
        int32_t counter = 0;
        while (!stopping) {
            generator.Read(buffer.get(), buffer_size);
            EXPECT_EQ(pcm_writei(pcm_out, buffer.get(), frames), frames) << counter;
            counter++;
        }
        std::cout << "write count = " << counter << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    stopping = true;
    capture.join();
    playback.join();
}

} // namespace testing
} // namespace tinyalsa
