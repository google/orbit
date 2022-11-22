// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <memory>
#include <vector>

#include "ClientData/TimerChain.h"
#include "ClientData/TimerData.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;

TEST(TimerData, IsEmpty) {
  TimerData timer_data;
  EXPECT_TRUE(timer_data.GetChains().empty());
  EXPECT_EQ(timer_data.GetChain(0), nullptr);
  EXPECT_EQ(timer_data.GetChain(7), nullptr);
  EXPECT_TRUE(timer_data.IsEmpty());
  EXPECT_EQ(timer_data.GetNumberOfTimers(), 0);
  EXPECT_EQ(timer_data.GetMaxTime(), std::numeric_limits<uint64_t>::min());
  EXPECT_EQ(timer_data.GetMinTime(), std::numeric_limits<uint64_t>::max());
}

/*
Timers to test:
_____________________________
|   Left   |     |  Right   |  |
|-----------------------------
     |  Middle / Down   |
     --------------------
 */

constexpr uint64_t kLeftTimerStart = 2;
constexpr uint64_t kLeftTimerEnd = 6;
constexpr uint64_t kMiddleTimerStart = 5;
constexpr uint64_t kMiddleTimerEnd = 9;
constexpr uint64_t kRightTimerStart = 8;
constexpr uint64_t kRightTimerEnd = 12;
constexpr uint64_t kNumTimers = 3;
constexpr uint64_t kDepth = 2;
constexpr uint64_t kMinTimestamp = 2;
constexpr uint64_t kMaxTimestamp = 12;

static TimerInfo GetLeftTimer() {
  TimerInfo timer_info_left;
  timer_info_left.set_start(kLeftTimerStart);
  timer_info_left.set_end(kLeftTimerEnd);
  timer_info_left.set_depth(0);
  return timer_info_left;
}

static TimerInfo GetRightTimer() {
  TimerInfo timer_info_right;
  timer_info_right.set_start(kRightTimerStart);
  timer_info_right.set_end(kRightTimerEnd);
  timer_info_right.set_depth(0);
  return timer_info_right;
}

static TimerInfo GetDownTimer() {
  TimerInfo timer_info_down;
  timer_info_down.set_start(kMiddleTimerStart);
  timer_info_down.set_end(kMiddleTimerEnd);
  timer_info_down.set_depth(1);
  return timer_info_down;
}

static TimerInfo GetMiddleTimer() {
  TimerInfo timer_info_middle;
  timer_info_middle.set_start(kMiddleTimerStart);
  timer_info_middle.set_end(kMiddleTimerEnd);
  timer_info_middle.set_depth(0);
  return timer_info_middle;
}

TEST(TimerData, AddTimers) {
  TimerData timer_data;

  timer_data.AddTimer(GetLeftTimer(), 0);

  EXPECT_FALSE(timer_data.IsEmpty());
  EXPECT_EQ(timer_data.GetNumberOfTimers(), 1);
  ASSERT_NE(timer_data.GetChain(0), nullptr);
  EXPECT_EQ(timer_data.GetChain(1), nullptr);
  EXPECT_EQ(timer_data.GetChain(0)->size(), 1);

  EXPECT_EQ(timer_data.GetMinTime(), kLeftTimerStart);
  EXPECT_EQ(timer_data.GetMaxTime(), kLeftTimerEnd);

  timer_data.AddTimer(GetRightTimer(), 0);

  EXPECT_FALSE(timer_data.IsEmpty());
  EXPECT_EQ(timer_data.GetNumberOfTimers(), 2);
  ASSERT_NE(timer_data.GetChain(0), nullptr);
  EXPECT_EQ(timer_data.GetChain(1), nullptr);
  EXPECT_EQ(timer_data.GetChain(0)->size(), 2);

  EXPECT_EQ(timer_data.GetMinTime(), kLeftTimerStart);
  EXPECT_EQ(timer_data.GetMaxTime(), kRightTimerEnd);

  timer_data.AddTimer(GetDownTimer(), 1);

  EXPECT_EQ(timer_data.GetNumberOfTimers(), kNumTimers);
  EXPECT_FALSE(timer_data.IsEmpty());
  ASSERT_NE(timer_data.GetChain(0), nullptr);
  ASSERT_NE(timer_data.GetChain(1), nullptr);
  EXPECT_EQ(timer_data.GetChain(0)->size(), 2);
  EXPECT_EQ(timer_data.GetChain(1)->size(), 1);
  EXPECT_EQ(timer_data.GetDepth(), kDepth);
  EXPECT_EQ(timer_data.GetMinTime(), kMinTimestamp);
  EXPECT_EQ(timer_data.GetMaxTime(), kMaxTimestamp);
}

std::unique_ptr<TimerData> GetOrderedTimersSameDepth() {
  auto timer_data = std::make_unique<TimerData>();
  timer_data->AddTimer(GetLeftTimer());
  timer_data->AddTimer(GetMiddleTimer());
  timer_data->AddTimer(GetRightTimer());
  return timer_data;
}

std::unique_ptr<TimerData> GetUnorderedTimersSameDepth() {
  auto timer_data = std::make_unique<TimerData>();
  timer_data->AddTimer(GetRightTimer());
  timer_data->AddTimer(GetLeftTimer());
  timer_data->AddTimer(GetMiddleTimer());
  return timer_data;
}

std::unique_ptr<TimerData> GetTimersDifferentDepths() {
  auto timer_data = std::make_unique<TimerData>();
  timer_data->AddTimer(GetLeftTimer());
  timer_data->AddTimer(GetRightTimer());
  timer_data->AddTimer(GetDownTimer(), /*depth=*/1);
  return timer_data;
}

// TODO(b/204173036): Make GetFirstAfterStartTime private and test GetLeft/Right/Top/Down instead.
TEST(TimerData, FindTimers) {
  std::unique_ptr<TimerData> timer_data = GetOrderedTimersSameDepth();

  {
    const TimerInfo* timer_info = timer_data->GetFirstAfterStartTime(kMiddleTimerStart - 1, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), kMiddleTimerStart);
    EXPECT_EQ(timer_info->end(), kMiddleTimerEnd);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstAfterStartTime(kLeftTimerStart, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), kMiddleTimerStart);
    EXPECT_EQ(timer_info->end(), kMiddleTimerEnd);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstAfterStartTime(kLeftTimerStart - 1, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), kLeftTimerStart);
    EXPECT_EQ(timer_info->end(), kLeftTimerEnd);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstAfterStartTime(kRightTimerStart - 1, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), kRightTimerStart);
    EXPECT_EQ(timer_info->end(), kRightTimerEnd);
  }

  {
    const TimerInfo* timer_info =
        timer_data->GetFirstAfterStartTime(std::numeric_limits<uint64_t>::max(), 0);
    EXPECT_EQ(timer_info, nullptr);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstAfterStartTime(0, 1);
    EXPECT_EQ(timer_info, nullptr);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstBeforeStartTime(kMiddleTimerStart, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), kLeftTimerStart);
    EXPECT_EQ(timer_info->end(), kLeftTimerEnd);
  }

  {
    const TimerInfo* timer_info =
        timer_data->GetFirstBeforeStartTime(std::numeric_limits<uint64_t>::max(), 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), kRightTimerStart);
    EXPECT_EQ(timer_info->end(), kRightTimerEnd);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstBeforeStartTime(kLeftTimerStart, 0);
    EXPECT_EQ(timer_info, nullptr);
  }

  {
    const TimerInfo* timer_info = timer_data->GetFirstBeforeStartTime(0, 0);
    EXPECT_EQ(timer_info, nullptr);
  }
  {
    const TimerInfo* timer_info =
        timer_data->GetFirstBeforeStartTime(std::numeric_limits<uint64_t>::max(), 1);
    EXPECT_EQ(timer_info, nullptr);
  }
}

void CheckGetTimers(std::unique_ptr<TimerData> timer_data) {
  EXPECT_EQ(timer_data->GetTimers(0, kLeftTimerStart - 1).size(), 0);
  EXPECT_EQ(timer_data->GetTimers(kRightTimerEnd + 1, kRightTimerEnd + 10).size(), 0);
  EXPECT_EQ(timer_data->GetTimers(kLeftTimerStart - 1, kLeftTimerStart + 1).size(), 1);  // left
  EXPECT_EQ(timer_data->GetTimers(kLeftTimerStart + 1, kLeftTimerEnd).size(), 2);  // left, middle
  EXPECT_EQ(timer_data->GetTimers(kMiddleTimerStart, kMiddleTimerEnd).size(), 3);
  EXPECT_EQ(timer_data->GetTimers(kRightTimerStart, kRightTimerEnd).size(), 2);     // middle, right
  EXPECT_EQ(timer_data->GetTimers(kMiddleTimerEnd + 1, kRightTimerEnd).size(), 1);  // right
  EXPECT_EQ(timer_data->GetTimers().size(), 3);
}

TEST(TimerData, GetTimers) {
  CheckGetTimers(GetOrderedTimersSameDepth());
  CheckGetTimers(GetUnorderedTimersSameDepth());
  CheckGetTimers(GetTimersDifferentDepths());
}

TEST(TimerData, GetTimersAtDepthDiscretized) {
  // Left, right and down timers
  std::unique_ptr<TimerData> timer_data = GetTimersDifferentDepths();

  uint32_t kOnePixel = 1;
  uint32_t kNormalResolution = 1000;

  auto verify_size = [&timer_data](uint32_t depth, uint32_t resolution, uint64_t start_ns,
                                   uint64_t end_ns, size_t expected_size) {
    EXPECT_EQ(timer_data->GetTimersAtDepthDiscretized(depth, resolution, start_ns, end_ns).size(),
              expected_size);
  };

  // Normal case. Left and right timer are visible.
  verify_size(0, kNormalResolution, kMinTimestamp, kMaxTimestamp, 2);

  // Range tests.
  {
    // No visible timers at the left and right of the visible range.
    verify_size(0, kNormalResolution, 0, kMinTimestamp - 1, 0);
    verify_size(0, kNormalResolution, kMaxTimestamp + 1, kMaxTimestamp + 10, 0);

    // Only left timer will be visible if the right timer is out of range.
    verify_size(0, kNormalResolution, kMinTimestamp, kRightTimerStart - 1, 1);

    // Only right timer will be visible if the left timer is out of range.
    verify_size(0, kNormalResolution, kLeftTimerEnd + 1, kMaxTimestamp, 1);

    // Both timers will be visible even if we include them partially.
    verify_size(0, kNormalResolution, kLeftTimerEnd, kRightTimerStart, 2);
  }

  // Resolution tests.
  {
    // Only one timer will be visible if we have 1 pixel resolution.
    verify_size(0, kOnePixel, kMinTimestamp, kMaxTimestamp, 1);

    // Only one timer will be visible if we zoom-out a lot even with a normal resolution.
    verify_size(0, kNormalResolution, 0, 10000000, 1);

    // If there is a timer in the range, we should see it in any resolution.
    verify_size(0, kOnePixel, kMinTimestamp, kMinTimestamp + 1, 1);
    verify_size(0, kNormalResolution, kMinTimestamp, kMinTimestamp + 1, 1);
  }

  // Queries with `depth = 1` should just return the down timer (if it is in the range).
  verify_size(1, kNormalResolution, kMinTimestamp, kMaxTimestamp, 1);

  // No timers with `depth = 2` in TimerData.
  verify_size(2, kNormalResolution, kMinTimestamp, kMaxTimestamp, 0);
}

}  // namespace orbit_client_data