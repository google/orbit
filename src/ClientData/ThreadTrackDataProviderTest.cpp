// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ThreadTrackDataProvider.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;

const uint32_t kThreadId1 = 1;
const uint32_t kThreadId2 = 2;
const uint32_t kProcessId = 42;

TEST(ThreadTrackDataProvider, Empty) {
  ThreadTrackDataProvider thread_track_data_provider;
  EXPECT_TRUE(thread_track_data_provider.GetAllThreadTimerChains().empty());
  EXPECT_TRUE(thread_track_data_provider.GetAllThreadId().empty());

  thread_track_data_provider.CreateScopeTreeTimerData(kThreadId1);
  EXPECT_TRUE(thread_track_data_provider.IsEmpty(kThreadId1));
}

TEST(ThreadTrackDataProvider, InsertAndGetTimer) {
  ThreadTrackDataProvider thread_track_data_provider;

  TimerInfo timer_info;
  timer_info.set_thread_id(kThreadId1);
  timer_info.set_start(2);
  timer_info.set_end(5);
  thread_track_data_provider.AddTimer(timer_info);

  EXPECT_FALSE(thread_track_data_provider.IsEmpty(kThreadId1));

  auto all_timers = thread_track_data_provider.GetTimersAtDepth(kThreadId1, 0);
  EXPECT_EQ(all_timers.size(), 1);

  const orbit_client_protos::TimerInfo* inserted_timer_info = all_timers[0];
  EXPECT_EQ(inserted_timer_info->thread_id(), kThreadId1);
  EXPECT_EQ(inserted_timer_info->start(), 2);
  EXPECT_EQ(inserted_timer_info->end(), 5);
}

TEST(ThreadTrackDataProvider, OnCaptureComplete) {
  // ScopeTree: Need OnCaptureComplete to process the data when loading a capture.
  ThreadTrackDataProvider thread_track_data_provider(true);

  TimerInfo timer_info;
  timer_info.set_thread_id(kThreadId1);
  timer_info.set_start(2);
  timer_info.set_end(5);
  thread_track_data_provider.AddTimer(timer_info);

  EXPECT_TRUE(thread_track_data_provider.IsEmpty(kThreadId1));
  EXPECT_EQ(thread_track_data_provider.GetTimersAtDepth(kThreadId1, 0).size(), 0);

  thread_track_data_provider.OnCaptureComplete();

  EXPECT_FALSE(thread_track_data_provider.IsEmpty(kThreadId1));
  auto all_timers = thread_track_data_provider.GetTimersAtDepth(kThreadId1, 0);
  EXPECT_EQ(all_timers.size(), 1);
  const orbit_client_protos::TimerInfo* inserted_timer_info = all_timers[0];
  EXPECT_EQ(inserted_timer_info->thread_id(), 1);
  EXPECT_EQ(inserted_timer_info->start(), 2);
  EXPECT_EQ(inserted_timer_info->end(), 5);
}

struct TimersInTest {
  const TimerInfo* left;
  const TimerInfo* center;
  const TimerInfo* right;
  const TimerInfo* down;
  const TimerInfo* other_thread_id;
};

// Insert 4 timers with the same thread_id and an extra with a different one.
TimersInTest InsertTimersForTesting(ThreadTrackDataProvider& thread_track_data_provider) {
  TimersInTest inserted_timers_ptr;
  TimerInfo timer_info;

  // left
  timer_info.set_process_id(kProcessId);
  timer_info.set_thread_id(kThreadId1);
  timer_info.set_start(2);
  timer_info.set_end(5);
  inserted_timers_ptr.left = &thread_track_data_provider.AddTimer(timer_info);

  // center
  timer_info.set_start(6);
  timer_info.set_end(9);
  inserted_timers_ptr.center = &thread_track_data_provider.AddTimer(timer_info);

  // down
  timer_info.set_start(7);
  timer_info.set_end(9);
  inserted_timers_ptr.down = &thread_track_data_provider.AddTimer(timer_info);

  // right
  timer_info.set_start(9);
  timer_info.set_end(10);
  inserted_timers_ptr.right = &thread_track_data_provider.AddTimer(timer_info);

  // other thread_id
  timer_info.set_thread_id(kThreadId2);
  timer_info.set_start(5);
  timer_info.set_end(11);
  inserted_timers_ptr.other_thread_id = &thread_track_data_provider.AddTimer(timer_info);

  return inserted_timers_ptr;
}

TEST(ThreadTrackDataProvider, GetTimers) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  EXPECT_EQ(thread_track_data_provider.GetTimersAtDepth(1, 0).size(), 3);  // left, center, right
  EXPECT_EQ(thread_track_data_provider.GetTimersAtDepth(1, 1).size(), 1);  // down
  EXPECT_EQ(thread_track_data_provider.GetTimersAtDepth(2, 0).size(), 1);  // other_thread_id
}

TEST(ThreadTrackDataProvider, GetAllThreadId) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  auto all_thread_id = thread_track_data_provider.GetAllThreadId();
  EXPECT_EQ(all_thread_id.size(), 2);
  EXPECT_EQ(std::min(all_thread_id[0], all_thread_id[1]), kThreadId1);
  EXPECT_EQ(std::max(all_thread_id[0], all_thread_id[1]), kThreadId2);
}

TEST(ThreadTrackDataProvider, GetChains) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  auto chains_thread_1 = thread_track_data_provider.GetChains(kThreadId1);
  EXPECT_EQ(chains_thread_1.size(), 1);
  EXPECT_EQ(chains_thread_1[0]->size(), 4);

  auto chains_thread_2 = thread_track_data_provider.GetChains(kThreadId2);
  EXPECT_EQ(chains_thread_2.size(), 1);
  EXPECT_EQ(chains_thread_2[0]->size(), 1);

  // 2 Chains, 5 Timers in Total.
  auto all_chains = thread_track_data_provider.GetAllThreadTimerChains();
  EXPECT_EQ(all_chains.size(), 2);
  EXPECT_EQ(all_chains[0]->size() + all_chains[1]->size(), 5);
}

TEST(ThreadTrackDataProvider, GetStatsFromThreadId) {
  ThreadTrackDataProvider thread_track_data_provider;
  InsertTimersForTesting(thread_track_data_provider);

  EXPECT_EQ(thread_track_data_provider.GetMinTime(kThreadId1), 2);
  EXPECT_EQ(thread_track_data_provider.GetMaxTime(kThreadId1), 10);
  EXPECT_EQ(thread_track_data_provider.GetNumberOfTimers(kThreadId1), 4);
  EXPECT_EQ(thread_track_data_provider.GetDepth(kThreadId1), 2);
  EXPECT_EQ(thread_track_data_provider.GetProcessId(kThreadId1), kProcessId);

  EXPECT_EQ(thread_track_data_provider.GetMinTime(kThreadId2), 5);
  EXPECT_EQ(thread_track_data_provider.GetMaxTime(kThreadId2), 11);
  EXPECT_EQ(thread_track_data_provider.GetNumberOfTimers(kThreadId2), 1);
  EXPECT_EQ(thread_track_data_provider.GetDepth(kThreadId2), 1);
  EXPECT_EQ(thread_track_data_provider.GetProcessId(kThreadId2), kProcessId);
}

TEST(ThreadTrackDataProvider, GetLeftRightUpDown) {
  ThreadTrackDataProvider thread_track_data_provider;
  auto TimersInTest = InsertTimersForTesting(thread_track_data_provider);

  const TimerInfo* left = TimersInTest.left;
  const TimerInfo* right = TimersInTest.right;
  const TimerInfo* center = TimersInTest.center;
  const TimerInfo* down = TimersInTest.down;
  const TimerInfo* other_thread_id = TimersInTest.other_thread_id;

  EXPECT_EQ(thread_track_data_provider.GetLeft(*left), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetRight(*left), center);
  EXPECT_EQ(thread_track_data_provider.GetDown(*left), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetUp(*left), nullptr);

  EXPECT_EQ(thread_track_data_provider.GetLeft(*center), left);
  EXPECT_EQ(thread_track_data_provider.GetRight(*center), right);
  EXPECT_EQ(thread_track_data_provider.GetDown(*center), down);
  EXPECT_EQ(thread_track_data_provider.GetUp(*center), nullptr);

  EXPECT_EQ(thread_track_data_provider.GetLeft(*right), center);
  EXPECT_EQ(thread_track_data_provider.GetRight(*right), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetDown(*right), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetUp(*right), nullptr);

  EXPECT_EQ(thread_track_data_provider.GetLeft(*down), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetRight(*down), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetDown(*down), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetUp(*down), center);

  EXPECT_EQ(thread_track_data_provider.GetLeft(*other_thread_id), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetRight(*other_thread_id), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetDown(*other_thread_id), nullptr);
  EXPECT_EQ(thread_track_data_provider.GetUp(*other_thread_id), nullptr);
}

}  // namespace orbit_client_data