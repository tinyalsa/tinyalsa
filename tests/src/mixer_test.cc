/* mixer_test.c
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

#include <limits>
#include <string_view>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>

#include "tinyalsa/mixer.h"

namespace tinyalsa {
namespace testing {

#ifndef MAX_CARD_INDEX
#define MAX_CARD_INDEX 2
#endif

static constexpr unsigned int kMaxCardIndex = MAX_CARD_INDEX;

static constexpr int k100Percent = 100;
static constexpr int k0Percent = 0;

TEST(MixerTest, OpenAndClose) {
    // assume card 0 is always probed.
    mixer *mixer_object = mixer_open(0);
    EXPECT_NE(mixer_object, nullptr);
    mixer_close(mixer_object);
}

TEST(MixerTest, NullParametersCheck) {
    EXPECT_EQ(mixer_open(1000), nullptr);
    mixer_close(nullptr);
    EXPECT_EQ(mixer_add_new_ctls(nullptr), 0);
    EXPECT_EQ(mixer_get_name(nullptr), nullptr);
    EXPECT_EQ(mixer_get_num_ctls(nullptr), 0);
    EXPECT_EQ(mixer_get_num_ctls_by_name(nullptr, ""), 0);
    EXPECT_EQ(mixer_get_num_ctls_by_name(reinterpret_cast<const mixer *>(1), nullptr), 0);
    EXPECT_EQ(mixer_get_ctl_const(nullptr, 0), nullptr);
    EXPECT_EQ(mixer_get_ctl(nullptr, 0), nullptr);
    EXPECT_EQ(mixer_get_ctl_by_name(nullptr, ""), nullptr);
    EXPECT_EQ(mixer_get_ctl_by_name(reinterpret_cast<mixer *>(1), nullptr), nullptr);
    EXPECT_EQ(mixer_get_ctl_by_name_and_index(nullptr, "", 0), nullptr);
    EXPECT_EQ(
            mixer_get_ctl_by_name_and_index(reinterpret_cast<mixer *>(1), nullptr, 0),
            nullptr);
    EXPECT_NE(mixer_subscribe_events(nullptr, 0), 0);
    EXPECT_LT(mixer_wait_event(nullptr, 0), 0);
    EXPECT_EQ(mixer_ctl_get_id(nullptr), std::numeric_limits<unsigned int>::max());
    EXPECT_EQ(mixer_ctl_get_name(nullptr), nullptr);
    EXPECT_EQ(mixer_ctl_get_type(nullptr), MIXER_CTL_TYPE_UNKNOWN);
    EXPECT_STREQ(mixer_ctl_get_type_string(nullptr), "");
    EXPECT_EQ(mixer_ctl_get_num_values(nullptr), 0);
    EXPECT_EQ(mixer_ctl_get_num_enums(nullptr), 0);
    EXPECT_EQ(mixer_ctl_get_enum_string(nullptr, 0), nullptr);
    mixer_ctl_update(nullptr);
    EXPECT_EQ(mixer_ctl_is_access_tlv_rw(nullptr), 0);
    EXPECT_EQ(mixer_ctl_get_percent(nullptr, 0), -EINVAL);
    EXPECT_EQ(mixer_ctl_set_percent(nullptr, 0, 0), -EINVAL);
    EXPECT_EQ(mixer_ctl_get_value(nullptr, 0), -EINVAL);
    EXPECT_EQ(mixer_ctl_get_array(nullptr, reinterpret_cast<void *>(1), 1), -EINVAL);
    EXPECT_EQ(mixer_ctl_get_array(reinterpret_cast<mixer_ctl *>(1), nullptr, 1), -EINVAL);
    EXPECT_EQ(
            mixer_ctl_get_array(
                reinterpret_cast<mixer_ctl *>(1), reinterpret_cast<void *>(1), 0), -EINVAL);
    EXPECT_EQ(mixer_ctl_set_value(nullptr, 0, 0), -EINVAL);
    EXPECT_EQ(mixer_ctl_set_array(nullptr, reinterpret_cast<const void *>(1), 1), -EINVAL);
    EXPECT_EQ(mixer_ctl_set_array(reinterpret_cast<mixer_ctl *>(1), nullptr, 1), -EINVAL);
    EXPECT_EQ(
            mixer_ctl_set_array(
                reinterpret_cast<mixer_ctl *>(1), reinterpret_cast<const void *>(1), 0), -EINVAL);
    EXPECT_EQ(mixer_ctl_set_enum_by_string(nullptr, reinterpret_cast<const char *>(1)), -EINVAL);
    EXPECT_EQ(mixer_ctl_set_enum_by_string(reinterpret_cast<mixer_ctl *>(1), nullptr), -EINVAL);
    EXPECT_EQ(mixer_ctl_get_range_min(nullptr), -EINVAL);
    EXPECT_EQ(mixer_ctl_get_range_max(nullptr), -EINVAL);
    EXPECT_EQ(mixer_read_event(nullptr, reinterpret_cast<mixer_ctl_event *>(1)), -EINVAL);
    EXPECT_EQ(mixer_read_event(reinterpret_cast<mixer *>(1), nullptr), -EINVAL);
    EXPECT_EQ(mixer_consume_event(nullptr), -EINVAL);
}

class MixerTest : public ::testing::TestWithParam<unsigned int> {
  protected:
    MixerTest() : mixer_object(nullptr) {}
    virtual ~MixerTest() = default;

    virtual void SetUp() override {
        unsigned int card = GetParam();
        mixer_object = mixer_open(card);
        ASSERT_NE(mixer_object, nullptr);
    }

    virtual void TearDown() override {
        mixer_close(mixer_object);
    }

    mixer *mixer_object;
};

TEST_P(MixerTest, AddNewControls) {
    ASSERT_EQ(mixer_add_new_ctls(mixer_object), 0);
}

TEST_P(MixerTest, GetName) {
    const char *name = mixer_get_name(mixer_object);
    std::cout << name << std::endl;
    ASSERT_STRNE(name, "");
}

TEST_P(MixerTest, GetNumberOfControls) {
    unsigned int nums = mixer_get_num_ctls(mixer_object);
    std::cout << nums << std::endl;
    ASSERT_GT(nums, 0);
}

class MixerControlsTest : public MixerTest {
  protected:
    MixerControlsTest() : number_of_controls(0), controls(nullptr) {}
    virtual ~MixerControlsTest() = default;

    virtual void SetUp() override {
        MixerTest::SetUp();

        number_of_controls = mixer_get_num_ctls(mixer_object);
        ASSERT_GT(number_of_controls, 0);

        controls = std::make_unique<const mixer_ctl *[]>(number_of_controls);
        ASSERT_NE(controls, nullptr);

        for (unsigned int i = 0; i < number_of_controls; i++) {
            controls[i] = mixer_get_ctl_const(mixer_object, i);
            EXPECT_EQ(mixer_ctl_get_id(controls[i]), i);
            EXPECT_STRNE(mixer_ctl_get_name(controls[i]), "");
            EXPECT_NE(controls[i], nullptr);
        }
    }

    virtual void TearDown() override {
        controls = nullptr;
        MixerTest::TearDown();
    }

    unsigned int number_of_controls;
    std::unique_ptr<const mixer_ctl *[]> controls;
};

TEST_P(MixerControlsTest, GetNumberOfControlsByName) {
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        const char *name = mixer_ctl_get_name(controls[i]);
        ASSERT_GE(mixer_get_num_ctls_by_name(mixer_object, name), 1);
    }

    std::string name{mixer_ctl_get_name(controls[0])};
    name += "1";
    ASSERT_EQ(mixer_get_num_ctls_by_name(mixer_object, name.c_str()), 0);
}

TEST_P(MixerControlsTest, GetControlById) {
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        ASSERT_EQ(mixer_get_ctl(mixer_object, i), controls[i]);
    }

    ASSERT_EQ(mixer_get_ctl(mixer_object, number_of_controls), nullptr);
}

TEST_P(MixerControlsTest, GetControlByName) {
    std::unordered_set<std::string> visited_names_set;
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        std::string name{mixer_ctl_get_name(controls[i])};
        if (visited_names_set.find(name) == visited_names_set.end()) {
            ASSERT_EQ(mixer_get_ctl_by_name(mixer_object, name.c_str()), controls[i]);
            visited_names_set.insert(name);
        }
    }
}

TEST_P(MixerControlsTest, GetControlByNameAndIndex) {
    std::unordered_map<std::string, int32_t> visited_names_and_count_map;
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        std::string name{mixer_ctl_get_name(controls[i])};
        if (visited_names_and_count_map.find(name) == visited_names_and_count_map.end()) {
            visited_names_and_count_map[name] = 0;
        }
        ASSERT_EQ(
                mixer_get_ctl_by_name_and_index(mixer_object,
                                                name.c_str(),
                                                visited_names_and_count_map[name]),
                controls[i]);
        visited_names_and_count_map[name] = visited_names_and_count_map[name] + 1;
    }
}

static inline bool IsValidTypeString(std::string& type) {
    return type == "BOOL" || type == "INT" || type == "ENUM" || type == "BYTE" ||
            type == "IEC958" || type == "INT64";
}

TEST_P(MixerControlsTest, GetControlTypeString) {
    ASSERT_STREQ(mixer_ctl_get_type_string(nullptr), "");

    for (unsigned int i = 0; i < number_of_controls; ++i) {
        std::string type{mixer_ctl_get_type_string(controls[i])};
        ASSERT_TRUE(IsValidTypeString(type));
    }
}

TEST_P(MixerControlsTest, GetNumberOfValues) {
    ASSERT_EQ(mixer_ctl_get_num_values(nullptr), 0);
}

TEST_P(MixerControlsTest, GetNumberOfEnumsAndEnumString) {
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        const mixer_ctl *control = controls[i];
        if (mixer_ctl_get_type(control) == MIXER_CTL_TYPE_ENUM) {
            unsigned int number_of_enums = mixer_ctl_get_num_enums(control);
            ASSERT_GT(number_of_enums, 0);
            for (unsigned int enum_id = 0; enum_id < number_of_enums; ++enum_id) {
                const char *enum_name = mixer_ctl_get_enum_string(
                        const_cast<mixer_ctl *>(control),
                        enum_id);
                ASSERT_STRNE(enum_name, "");
            }
        }
    }
}

TEST_P(MixerControlsTest, UpdateControl) {
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        mixer_ctl_update(const_cast<mixer_ctl *>(controls[i]));
    }
}

TEST_P(MixerControlsTest, GetPercent) {
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        const mixer_ctl *control = controls[i];
        if (mixer_ctl_get_type(control) == MIXER_CTL_TYPE_INT) {
            unsigned int number_of_values = mixer_ctl_get_num_values(controls[i]);
            std::unique_ptr<long []> values = std::make_unique<long []>(number_of_values);
            mixer_ctl_get_array(control, values.get(), number_of_values);
            for (unsigned int value_id = 0; value_id < number_of_values; ++value_id) {
                int max = mixer_ctl_get_range_max(control);
                int min = mixer_ctl_get_range_min(control);
                int percent = mixer_ctl_get_percent(control, value_id);
                ASSERT_GE(percent, k0Percent);
                ASSERT_LE(percent, k100Percent);
                int range = max - min;
                ASSERT_EQ(percent, (values[value_id] - min) * k100Percent / range);
            }
        } else {
            ASSERT_EQ(mixer_ctl_get_percent(control, 0), -EINVAL);
        }
    }
}

TEST_P(MixerControlsTest, SetPercent) {
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        const mixer_ctl *control = controls[i];
        if (mixer_ctl_get_type(control) == MIXER_CTL_TYPE_INT) {
            unsigned int number_of_values = mixer_ctl_get_num_values(controls[i]);
            std::unique_ptr<long []> values = std::make_unique<long []>(number_of_values);
            mixer_ctl_get_array(control, values.get(), number_of_values);
            for (unsigned int value_id = 0; value_id < number_of_values; ++value_id) {
                int max = mixer_ctl_get_range_max(control);
                int min = mixer_ctl_get_range_min(control);
                int value = values[value_id];
                int percent = mixer_ctl_get_percent(control, value_id);
                if (mixer_ctl_set_percent(
                        const_cast<mixer_ctl *>(control), value_id, k100Percent) == 0) {
                    // note: some controls are able to be written, but their values might not be
                    //   changed.
                    mixer_ctl_get_array(control, values.get(), number_of_values);
                    int new_value = values[value_id];
                    ASSERT_TRUE(new_value == value || new_value == max);
                }
                if (mixer_ctl_set_percent(
                        const_cast<mixer_ctl *>(control), value_id, k0Percent) == 0) {
                    mixer_ctl_get_array(control, values.get(), number_of_values);
                    int new_value = values[value_id];
                    ASSERT_TRUE(new_value == value || new_value == min);
                }
                mixer_ctl_set_percent(const_cast<mixer_ctl *>(control), value_id, percent);
            }
        } else {
            ASSERT_EQ(mixer_ctl_get_percent(control, 0), -EINVAL);
        }
    }
}

TEST_P(MixerControlsTest, Event) {
    ASSERT_EQ(mixer_subscribe_events(mixer_object, 1), 0);
    const mixer_ctl *control = nullptr;
    for (unsigned int i = 0; i < number_of_controls; ++i) {
        std::string_view name{mixer_ctl_get_name(controls[i])};

        if (name.find("Volume") != std::string_view::npos) {
            control = controls[i];
        }
    }

    if (control == nullptr) {
        GTEST_SKIP() << "No volume control was found in the controls list.";
    }

    auto *local_mixer_object = mixer_object;
    int percent = mixer_ctl_get_percent(control, 0);
    std::thread thread([local_mixer_object, control, percent] () {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        mixer_ctl_set_percent(
                const_cast<mixer_ctl *>(control), 0,
                percent == k100Percent ? k0Percent : k100Percent);
    });

    EXPECT_EQ(mixer_wait_event(mixer_object, 1000), 1);

    EXPECT_EQ(mixer_consume_event(mixer_object), 1);

    thread.join();
    ASSERT_EQ(mixer_subscribe_events(mixer_object, 0), 0);

    mixer_ctl_set_percent(const_cast<mixer_ctl *>(control), 0, percent);
}

INSTANTIATE_TEST_SUITE_P(
    MixerTest,
    MixerTest,
    ::testing::Range<unsigned int>(
        0,
        kMaxCardIndex + 1
    ));

INSTANTIATE_TEST_SUITE_P(
    MixerControlsTest,
    MixerControlsTest,
    ::testing::Range<unsigned int>(
        0,
        kMaxCardIndex + 1
    ));

} // namespace testing
} // namespace tinyalsa
