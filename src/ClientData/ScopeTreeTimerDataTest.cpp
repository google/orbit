// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ScopeTreeTimerData.h"

using orbit_client_protos::TimerInfo;

namespace orbit_client_data {

namespace {

struct TimersInTest {
  const TimerInfo* left;
  const TimerInfo* right;
  const TimerInfo* down;
};

static constexpr uint32_t kProcessId = 22;
static constexpr uint64_t kLeftTimerStart = 2;
static constexpr uint64_t kLeftTimerEnd = 5;
static constexpr uint64_t kRightTimerStart = 8;
static constexpr uint64_t kRightTimerEnd = 11;
static constexpr uint64_t kDownTimerStart = 10;
static constexpr uint64_t kDownTimerEnd = 11;
static constexpr uint64_t kNumTimers = 3;
static constexpr uint64_t kDepth = 2;
static constexpr uint64_t kMinTimestamp = 2;
static constexpr uint64_t kMaxTimestamp = 11;

TimersInTest AddTimersInScopeTreeTimerDataTest(ScopeTreeTimerData& scope_tree_timer_data) {
  TimersInTest inserted_timers;
  TimerInfo timer_info;
  timer_info.set_process_id(kProcessId);

  // left
  timer_info.set_start(kLeftTimerStart);
  timer_info.set_end(kLeftTimerEnd);
  inserted_timers.left = &scope_tree_timer_data.AddTimer(timer_info);

  // right
  timer_info.set_start(kRightTimerStart);
  timer_info.set_end(kRightTimerEnd);
  inserted_timers.right = &scope_tree_timer_data.AddTimer(timer_info);

  // down
  timer_info.set_start(kDownTimerStart);
  timer_info.set_end(kDownTimerEnd);
  inserted_timers.down = &scope_tree_timer_data.AddTimer(timer_info);

  return inserted_timers;
}

}  // namespace

TEST(ScopeTreeTimerData, EmptyWhenCreated) {
  ScopeTreeTimerData scope_tree_timer_data;
  EXPECT_TRUE(scope_tree_timer_data.IsEmpty());
  EXPECT_TRUE(scope_tree_timer_data.GetTimers().empty());
  EXPECT_TRUE(scope_tree_timer_data.GetChains().empty());
}

TEST(ScopeTreeTimerData, AddTimer) {
  static constexpr uint32_t kThreadId = 2;
  ScopeTreeTimerData scope_tree_timer_data(kThreadId);
  orbit_client_protos::TimerInfo timer_info;

  scope_tree_timer_data.AddTimer(timer_info);
  EXPECT_FALSE(scope_tree_timer_data.IsEmpty());
  EXPECT_EQ(scope_tree_timer_data.GetTimers().size(), 1);
  EXPECT_EQ(scope_tree_timer_data.GetThreadId(), kThreadId);
  EXPECT_EQ(scope_tree_timer_data.GetChains().size(), 1);
}

TEST(ScopeTreeTimerData, OnCaptureComplete) {
  ScopeTreeTimerData scope_tree_timer_data(
      -1, ScopeTreeTimerData::ScopeTreeUpdateType::kOnCaptureComplete);
  orbit_client_protos::TimerInfo timer_info;

  scope_tree_timer_data.AddTimer(timer_info);

  // OnCaptureComplete method is needed to build the ScopeTree
  EXPECT_TRUE(scope_tree_timer_data.GetTimers().empty());

  scope_tree_timer_data.OnCaptureComplete();
  EXPECT_FALSE(scope_tree_timer_data.GetTimers().empty());
}

TEST(ScopeTreeTimerData, GetTimerMetadata) {
  ScopeTreeTimerData scope_tree_timer_data;
  AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  EXPECT_FALSE(scope_tree_timer_data.IsEmpty());
  EXPECT_EQ(scope_tree_timer_data.GetNumberOfTimers(), kNumTimers);
  EXPECT_EQ(scope_tree_timer_data.GetDepth(), kDepth);
  EXPECT_EQ(scope_tree_timer_data.GetMinTime(), kMinTimestamp);
  EXPECT_EQ(scope_tree_timer_data.GetMaxTime(), kMaxTimestamp);
  EXPECT_EQ(scope_tree_timer_data.GetProcessId(), kProcessId);
}

TEST(ScopeTreeTimerData, GetTimers) {
  ScopeTreeTimerData scope_tree_timer_data;
  AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  EXPECT_EQ(scope_tree_timer_data.GetTimers(0, kLeftTimerStart - 1).size(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetTimers(kRightTimerEnd + 1, kRightTimerEnd + 10).size(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetTimers(kLeftTimerStart - 1, kLeftTimerStart + 1).size(),
            1);  // left
  EXPECT_EQ(scope_tree_timer_data.GetTimers(kRightTimerStart, kRightTimerStart + 1).size(),
            1);  // right
  EXPECT_EQ(scope_tree_timer_data.GetTimers(kRightTimerStart, kRightTimerEnd).size(),
            2);  // right, down
  EXPECT_EQ(scope_tree_timer_data.GetTimers(kLeftTimerEnd - 1, kRightTimerEnd).size(), 3);
  EXPECT_EQ(scope_tree_timer_data.GetTimers().size(), 3);
}

TEST(ScopeTreeTimerData, GetTimersAtDepth) {
  ScopeTreeTimerData scope_tree_timer_data;
  AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  // Depth 0 -> Left, Right
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(0, 0, kLeftTimerStart - 1).size(), 0);
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepth(0, kLeftTimerStart - 1, kLeftTimerStart + 1).size(),
      1);  // left
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepth(0, kLeftTimerEnd + 1, kRightTimerStart - 1).size(), 0);
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepth(0, kRightTimerStart - 1, kRightTimerStart + 1).size(),
      1);  // right
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepth(0, kLeftTimerEnd - 1, kRightTimerStart + 1).size(), 2);
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepth(0, kRightTimerEnd + 1, kRightTimerEnd + 10).size(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(0).size(), 2);

  // Depth 1 -> Down
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(1).size(), 1);
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(1, 0, kDownTimerStart - 1).size(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(1, 0, kDownTimerStart + 1).size(), 1);
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(1, kDownTimerEnd - 1, kDownTimerEnd).size(), 1);
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepth(1, kDownTimerEnd + 1, kDownTimerEnd + 10).size(),
            0);
}

TEST(ScopeTreeTimerData, GetTimersAtDepthOptimized) {
  ScopeTreeTimerData scope_tree_timer_data;
  AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  // We should see both timers with a normal resolution.
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepthDiscretized(0, 1000, kLeftTimerStart, kRightTimerEnd)
          .size(),
      2);

  // We should see only one timer if we have 1 pixel resolution.
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepthDiscretized(0, 1, kLeftTimerStart, kRightTimerEnd)
                .size(),
            1);

  // We should only see 1 if we zoom-out a lot even with a normal resolution.
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepthDiscretized(0, 1000, 0, 10000000).size(), 1);

  // If there is a timer in the range, we should see it in any resolution.
  EXPECT_EQ(
      scope_tree_timer_data.GetTimersAtDepthDiscretized(0, 1, kLeftTimerStart, kLeftTimerStart + 1)
          .size(),
      1);  // Left
  EXPECT_EQ(scope_tree_timer_data
                .GetTimersAtDepthDiscretized(0, 1000, kLeftTimerStart, kLeftTimerStart + 1)
                .size(),
            1);  // Left
  EXPECT_EQ(scope_tree_timer_data.GetTimersAtDepthDiscretized(1, 1000, 0, 10000000).size(),
            1);  // Down
}

TEST(ScopeTreeTimerData, GetLeftRightUpDown) {
  ScopeTreeTimerData scope_tree_timer_data;
  TimersInTest inserted_timers = AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  const TimerInfo* left = inserted_timers.left;
  const TimerInfo* right = inserted_timers.right;
  const TimerInfo* down = inserted_timers.down;

  auto check_neighbors = [&](const TimerInfo* current, const TimerInfo* expected_left,
                             const TimerInfo* expected_right, const TimerInfo* expected_down,
                             const TimerInfo* expected_up) {
    EXPECT_EQ(scope_tree_timer_data.GetLeft(*current), expected_left);
    EXPECT_EQ(scope_tree_timer_data.GetRight(*current), expected_right);
    EXPECT_EQ(scope_tree_timer_data.GetDown(*current), expected_down);
    EXPECT_EQ(scope_tree_timer_data.GetUp(*current), expected_up);
  };

  check_neighbors(left, nullptr, right, nullptr, nullptr);
  check_neighbors(right, left, nullptr, down, nullptr);
  check_neighbors(down, nullptr, nullptr, nullptr, right);
}

}  // namespace orbit_client_data