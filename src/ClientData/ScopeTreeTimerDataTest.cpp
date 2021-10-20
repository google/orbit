// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ScopeTreeTimerData.h"

using orbit_client_protos::TimerInfo;

namespace orbit_client_data {

TEST(ScopeTreeTimerData, EmptyWhenCreated) {
  ScopeTreeTimerData scope_tree_timer_data;
  EXPECT_TRUE(scope_tree_timer_data.GetTimerMetadata().is_empty);
  EXPECT_TRUE(scope_tree_timer_data.GetTimers().empty());
  EXPECT_EQ(scope_tree_timer_data.GetChains().size(), 0);
}

TEST(ScopeTreeTimerData, AddTimer) {
  static constexpr uint32_t kThreadId = 2;
  ScopeTreeTimerData scope_tree_timer_data(kThreadId);
  orbit_client_protos::TimerInfo timer_info;

  scope_tree_timer_data.AddTimer(timer_info);
  EXPECT_FALSE(scope_tree_timer_data.GetTimerMetadata().is_empty);
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
  EXPECT_EQ(scope_tree_timer_data.GetTimers().size(), 0);

  scope_tree_timer_data.OnCaptureComplete();
  EXPECT_EQ(scope_tree_timer_data.GetTimers().size(), 1);
}

struct ScopeTreeTimerDataTestTimers {
  static constexpr uint32_t kProcessId = 22;

  const TimerInfo* left;
  static constexpr uint64_t kLeftTimerStart = 2;
  static constexpr uint64_t kLeftTimerEnd = 5;

  const TimerInfo* right;
  static constexpr uint64_t kRightTimerStart = 8;
  static constexpr uint64_t kRightTimerEnd = 11;

  const TimerInfo* down;
  static constexpr uint64_t kDownTimerStart = 10;
  static constexpr uint64_t kDownTimerEnd = 11;

  static constexpr uint64_t kNumTimers = 3;
  static constexpr uint64_t kDepth = 2;
  static constexpr uint64_t kMinTimestamp = 2;
  static constexpr uint64_t kMaxTimestamp = 11;
};

ScopeTreeTimerDataTestTimers AddTimersInScopeTreeTimerDataTest(
    ScopeTreeTimerData& scope_tree_timer_data) {
  ScopeTreeTimerDataTestTimers inserted_timers_ptr;
  TimerInfo timer_info;
  timer_info.set_process_id(ScopeTreeTimerDataTestTimers::kProcessId);

  // left
  timer_info.set_start(ScopeTreeTimerDataTestTimers::kLeftTimerStart);
  timer_info.set_end(ScopeTreeTimerDataTestTimers::kLeftTimerEnd);
  inserted_timers_ptr.left = &scope_tree_timer_data.AddTimer(timer_info);

  // right
  timer_info.set_start(ScopeTreeTimerDataTestTimers::kRightTimerStart);
  timer_info.set_end(ScopeTreeTimerDataTestTimers::kRightTimerEnd);
  inserted_timers_ptr.right = &scope_tree_timer_data.AddTimer(timer_info);

  // down
  timer_info.set_start(ScopeTreeTimerDataTestTimers::kDownTimerStart);
  timer_info.set_end(ScopeTreeTimerDataTestTimers::kDownTimerEnd);
  inserted_timers_ptr.down = &scope_tree_timer_data.AddTimer(timer_info);

  return inserted_timers_ptr;
}

TEST(ScopeTreeTimerData, GetTimerMetadata) {
  ScopeTreeTimerData scope_tree_timer_data;
  AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  const TimerMetadata timer_metadata = scope_tree_timer_data.GetTimerMetadata();

  EXPECT_FALSE(timer_metadata.is_empty);
  EXPECT_EQ(timer_metadata.number_of_timers, ScopeTreeTimerDataTestTimers::kNumTimers);
  EXPECT_EQ(timer_metadata.depth, ScopeTreeTimerDataTestTimers::kDepth);
  EXPECT_EQ(timer_metadata.min_time, ScopeTreeTimerDataTestTimers::kMinTimestamp);
  EXPECT_EQ(timer_metadata.max_time, ScopeTreeTimerDataTestTimers::kMaxTimestamp);
  EXPECT_EQ(timer_metadata.process_id, ScopeTreeTimerDataTestTimers::kProcessId);
}

TEST(ScopeTreeTimerData, GetTimers) {
  ScopeTreeTimerData scope_tree_timer_data;
  AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

  EXPECT_EQ(
      scope_tree_timer_data.GetTimers(0, ScopeTreeTimerDataTestTimers::kLeftTimerStart - 1).size(),
      0);
  EXPECT_EQ(scope_tree_timer_data
                .GetTimers(ScopeTreeTimerDataTestTimers::kRightTimerEnd + 1,
                           ScopeTreeTimerDataTestTimers::kRightTimerEnd + 10)
                .size(),
            0);
  EXPECT_EQ(scope_tree_timer_data
                .GetTimers(ScopeTreeTimerDataTestTimers::kLeftTimerStart - 1,
                           ScopeTreeTimerDataTestTimers::kLeftTimerStart + 1)
                .size(),
            1);  // left
  EXPECT_EQ(scope_tree_timer_data
                .GetTimers(ScopeTreeTimerDataTestTimers::kRightTimerStart,
                           ScopeTreeTimerDataTestTimers::kRightTimerStart + 1)
                .size(),
            1);  // right
  EXPECT_EQ(scope_tree_timer_data
                .GetTimers(ScopeTreeTimerDataTestTimers::kRightTimerStart,
                           ScopeTreeTimerDataTestTimers::kRightTimerEnd)
                .size(),
            2);  // right, down
  EXPECT_EQ(scope_tree_timer_data
                .GetTimers(ScopeTreeTimerDataTestTimers::kLeftTimerEnd - 1,
                           ScopeTreeTimerDataTestTimers::kRightTimerEnd)
                .size(),
            3);
  EXPECT_EQ(scope_tree_timer_data.GetTimers().size(), 3);
}

bool AreSameTimer(const orbit_client_protos::TimerInfo& timer_1,
                  const orbit_client_protos::TimerInfo& timer_2) {
  return timer_1.start() == timer_2.start() && timer_1.end() && timer_2.end();
}

TEST(ScopeTreeTimerData, GetLeftRightUpDown) {
  ScopeTreeTimerData scope_tree_timer_data;
  ScopeTreeTimerDataTestTimers inserted_timers =
      AddTimersInScopeTreeTimerDataTest(scope_tree_timer_data);

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