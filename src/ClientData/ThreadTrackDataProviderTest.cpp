// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ClientData/ThreadTrackDataProvider.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;
using ::testing::UnorderedElementsAre;

const uint32_t kThreadId1 = 1;
const uint32_t kThreadId2 = 2;
const uint32_t kProcessId = 42;

TEST(ThreadTrackDataProvider, EmptyWhenCreated) {
  ThreadTrackDataProvider thread_track_data_provider;

  // No ScopeTreeTimerData, no Timers
  EXPECT_TRUE(thread_track_data_provider.GetAllThreadIds().empty());
  EXPECT_TRUE(thread_track_data_provider.GetAllThreadTimerChains().empty());

  thread_track_data_provider.CreateScopeTreeTimerData(kThreadId1);

  // One ScopeTreeTimerData, still no timers
  EXPECT_FALSE(thread_track_data_provider.GetAllThreadIds().empty());
  EXPECT_TRUE(thread_track_data_provider.GetAllThreadTimerChains().empty());

  // Therefore, TimerMetadata of kThreadId1 is still empty.
  EXPECT_TRUE(thread_track_data_provider.GetTimerMetadata(kThreadId1).is_empty);
}

TEST(ThreadTrackDataProvider, InsertAndGetTimer) {
  const uint64_t kTimerStart = 2;
  const uint64_t kTimerEnd = 5;
  ThreadTrackDataProvider thread_track_data_provider;

  TimerInfo timer_info;
  timer_info.set_thread_id(kThreadId1);
  timer_info.set_start(kTimerStart);
  timer_info.set_end(kTimerEnd);
  thread_track_data_provider.AddTimer(timer_info);

  EXPECT_FALSE(thread_track_data_provider.GetTimerMetadata(kThreadId1).is_empty);

  std::vector<const TimerInfo*> all_timers = thread_track_data_provider.GetTimers(kThreadId1);
  EXPECT_EQ(all_timers.size(), 1);

  const orbit_client_protos::TimerInfo* inserted_timer_info = all_timers[0];
  EXPECT_EQ(inserted_timer_info->thread_id(), kThreadId1);
  EXPECT_EQ(inserted_timer_info->start(), kTimerStart);
  EXPECT_EQ(inserted_timer_info->end(), kTimerEnd);
}

TEST(ThreadTrackDataProvider, OnCaptureComplete) {
  const uint64_t kTimerStart = 2;
  const uint64_t kTimerEnd = 5;
  // ScopeTree: Need OnCaptureComplete to process the data when loading a capture.
  ThreadTrackDataProvider thread_track_data_provider(true);

  TimerInfo timer_info;
  timer_info.set_thread_id(kThreadId1);
  timer_info.set_start(kTimerStart);
  timer_info.set_end(kTimerEnd);
  thread_track_data_provider.AddTimer(timer_info);

  EXPECT_EQ(thread_track_data_provider.GetTimers(kThreadId1).size(), 0);

  thread_track_data_provider.OnCaptureComplete();

  std::vector<const TimerInfo*> all_timers = thread_track_data_provider.GetTimers(kThreadId1);
  EXPECT_EQ(all_timers.size(), 1);
  const orbit_client_protos::TimerInfo* inserted_timer_info = all_timers[0];
  EXPECT_EQ(inserted_timer_info->thread_id(), 1);
  EXPECT_EQ(inserted_timer_info->start(), kTimerStart);
  EXPECT_EQ(inserted_timer_info->end(), kTimerEnd);
}

struct TimersInTest {
  // Thread1
  const TimerInfo* left;
  static constexpr uint64_t kLeftTimerStart = 2;
  static constexpr uint64_t kLeftTimerEnd = 5;

  const TimerInfo* center;
  static constexpr uint64_t kCenterTimerStart = 6;
  static constexpr uint64_t kCenterTimerEnd = 9;

  const TimerInfo* right;
  static constexpr uint64_t kRightTimerStart = 9;
  static constexpr uint64_t kRightTimerEnd = 10;

  const TimerInfo* down;
  static constexpr uint64_t kDownTimerStart = 7;
  static constexpr uint64_t kDownTimerEnd = 9;

  static constexpr uint64_t kNumTimersInThread1 = 4;
  static constexpr uint64_t kDepthThread1 = 2;
  static constexpr uint64_t kMinTimestampinThread1 = 2;
  static constexpr uint64_t kMaxTimestampinThread1 = 10;

  // Thread2
  const TimerInfo* other_thread_id;
  static constexpr uint64_t kOtherThreadIdTimerStart = 5;
  static constexpr uint64_t kOtherThreadIdTimerEnd = 11;

  static constexpr uint64_t kNumTimersInThread2 = 1;
  static constexpr uint64_t kDepthThread2 = 1;
};

// Insert 4 timers with the same thread_id and an extra with a different one.
TimersInTest InsertTimersForTesting(ThreadTrackDataProvider& thread_track_data_provider) {
  TimersInTest inserted_timers_ptr;
  TimerInfo timer_info;

  // left
  timer_info.set_process_id(kProcessId);
  timer_info.set_thread_id(kThreadId1);
  timer_info.set_start(TimersInTest::kLeftTimerStart);
  timer_info.set_end(TimersInTest::kLeftTimerEnd);
  inserted_timers_ptr.left = &thread_track_data_provider.AddTimer(timer_info);

  // center
  timer_info.set_start(TimersInTest::kCenterTimerStart);
  timer_info.set_end(TimersInTest::kCenterTimerEnd);
  inserted_timers_ptr.center = &thread_track_data_provider.AddTimer(timer_info);

  // down
  timer_info.set_start(TimersInTest::kDownTimerStart);
  timer_info.set_end(TimersInTest::kDownTimerEnd);
  inserted_timers_ptr.down = &thread_track_data_provider.AddTimer(timer_info);

  // right
  timer_info.set_start(TimersInTest::kRightTimerStart);
  timer_info.set_end(TimersInTest::kRightTimerEnd);
  inserted_timers_ptr.right = &thread_track_data_provider.AddTimer(timer_info);

  // other thread_id
  timer_info.set_thread_id(kThreadId2);
  timer_info.set_start(TimersInTest::kOtherThreadIdTimerStart);
  timer_info.set_end(TimersInTest::kOtherThreadIdTimerEnd);
  inserted_timers_ptr.other_thread_id = &thread_track_data_provider.AddTimer(timer_info);

  return inserted_timers_ptr;
}

TEST(ThreadTrackDataProvider, GetTimers) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  EXPECT_EQ(thread_track_data_provider.GetTimers(1).size(), TimersInTest::kNumTimersInThread1);
  EXPECT_EQ(thread_track_data_provider.GetTimers(2).size(), TimersInTest::kNumTimersInThread2);
}

TEST(ThreadTrackDataProvider, GetTimersAtDepthDiscretized) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  constexpr uint32_t kNormalResolution = 1000;

  // All timers should be visible in normal conditions
  EXPECT_EQ(thread_track_data_provider
                .GetTimersAtDepthDiscretized(1, 0, kNormalResolution, TimersInTest::kLeftTimerStart,
                                             TimersInTest::kRightTimerEnd)
                .size(),
            3);  // left, center, right
  // Only 1 pixel. There is only 1 visible timer.
  EXPECT_EQ(thread_track_data_provider
                .GetTimersAtDepthDiscretized(1, 0, 1, TimersInTest::kLeftTimerStart,
                                             TimersInTest::kRightTimerEnd)
                .size(),
            1);
  // Zooming-out a lot. Only the first pixel will have a visible timer.
  EXPECT_EQ(thread_track_data_provider
                .GetTimersAtDepthDiscretized(1, 0, kNormalResolution, TimersInTest::kLeftTimerStart,
                                             std::numeric_limits<uint64_t>::max())
                .size(),
            1);
}

TEST(ThreadTrackDataProvider, GetAllThreadIds) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  EXPECT_THAT(thread_track_data_provider.GetAllThreadIds(),
              UnorderedElementsAre(kThreadId1, kThreadId2));
}

TEST(ThreadTrackDataProvider, GetChains) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  std::vector<const TimerChain*> chains_thread_1 = thread_track_data_provider.GetChains(kThreadId1);
  EXPECT_EQ(chains_thread_1.size(), 1);
  EXPECT_EQ(chains_thread_1[0]->size(), TimersInTest::kNumTimersInThread1);

  std::vector<const TimerChain*> chains_thread_2 = thread_track_data_provider.GetChains(kThreadId2);
  EXPECT_EQ(chains_thread_2.size(), 1);
  EXPECT_EQ(chains_thread_2[0]->size(), TimersInTest::kNumTimersInThread2);

  // 2 Chains, 5 Timers in Total.
  std::vector<const TimerChain*> all_chains = thread_track_data_provider.GetAllThreadTimerChains();
  EXPECT_EQ(all_chains.size(), 2);
  EXPECT_EQ(all_chains[0]->size() + all_chains[1]->size(),
            TimersInTest::kNumTimersInThread1 + TimersInTest::kNumTimersInThread2);
}

TEST(ThreadTrackDataProvider, GetStatsFromThreadId) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  const TimerMetadata timer_metadata_thread_1 =
      thread_track_data_provider.GetTimerMetadata(kThreadId1);
  const TimerMetadata timer_metadata_thread_2 =
      thread_track_data_provider.GetTimerMetadata(kThreadId2);

  EXPECT_EQ(timer_metadata_thread_1.number_of_timers, TimersInTest::kNumTimersInThread1);
  EXPECT_EQ(timer_metadata_thread_1.min_time, TimersInTest::kMinTimestampinThread1);
  EXPECT_EQ(timer_metadata_thread_1.max_time, TimersInTest::kMaxTimestampinThread1);
  EXPECT_EQ(timer_metadata_thread_1.depth, TimersInTest::kDepthThread1);
  EXPECT_EQ(timer_metadata_thread_1.process_id, kProcessId);

  EXPECT_EQ(timer_metadata_thread_2.number_of_timers, TimersInTest::kNumTimersInThread2);
  EXPECT_EQ(timer_metadata_thread_2.min_time, TimersInTest::kOtherThreadIdTimerStart);
  EXPECT_EQ(timer_metadata_thread_2.max_time, TimersInTest::kOtherThreadIdTimerEnd);
  EXPECT_EQ(timer_metadata_thread_2.depth, TimersInTest::kDepthThread2);
  EXPECT_EQ(timer_metadata_thread_2.process_id, kProcessId);
}

TEST(ThreadTrackDataProvider, GetLeftRightUpDown) {
  ThreadTrackDataProvider thread_track_data_provider;
  TimersInTest inserted_timers = InsertTimersForTesting(thread_track_data_provider);

  const TimerInfo* left = inserted_timers.left;
  const TimerInfo* right = inserted_timers.right;
  const TimerInfo* center = inserted_timers.center;
  const TimerInfo* down = inserted_timers.down;
  const TimerInfo* other_thread_id = inserted_timers.other_thread_id;

  auto check_neighbors = [&](const TimerInfo* current, const TimerInfo* expected_left,
                             const TimerInfo* expected_right, const TimerInfo* expected_down,
                             const TimerInfo* expected_up) {
    EXPECT_EQ(thread_track_data_provider.GetLeft(*current), expected_left);
    EXPECT_EQ(thread_track_data_provider.GetRight(*current), expected_right);
    EXPECT_EQ(thread_track_data_provider.GetDown(*current), expected_down);
    EXPECT_EQ(thread_track_data_provider.GetUp(*current), expected_up);
  };

  check_neighbors(left, nullptr, center, nullptr, nullptr);
  check_neighbors(center, left, right, down, nullptr);
  check_neighbors(right, center, nullptr, nullptr, nullptr);
  check_neighbors(down, nullptr, nullptr, nullptr, center);
  check_neighbors(other_thread_id, nullptr, nullptr, nullptr, nullptr);
}

}  // namespace orbit_client_data