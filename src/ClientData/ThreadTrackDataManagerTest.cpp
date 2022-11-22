// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <vector>

#include "ClientData/ScopeTreeTimerData.h"
#include "ClientData/ThreadTrackDataManager.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;

constexpr uint32_t kThreadId1 = 1;
constexpr uint32_t kThreadId2 = 2;
constexpr uint32_t kNotUsedThreadId = 3;

TEST(ThreadTrackDataManager, IsEmpty) {
  ThreadTrackDataManager thread_track_data_manager;
  EXPECT_TRUE(thread_track_data_manager.GetAllScopeTreeTimerData().empty());
  EXPECT_EQ(thread_track_data_manager.GetScopeTreeTimerData(kNotUsedThreadId), nullptr);
}

TEST(ThreadTrackDataManager, CreateScopeTreeTimerData) {
  ThreadTrackDataManager thread_track_data_manager;

  thread_track_data_manager.CreateScopeTreeTimerData(kThreadId1);
  // One ScopeTreeTimerData, no timers.
  EXPECT_EQ(thread_track_data_manager.GetAllScopeTreeTimerData().size(), 1);
  EXPECT_TRUE(thread_track_data_manager.GetScopeTreeTimerData(kThreadId1)->IsEmpty());
}

TEST(ThreadTrackDataManager, AddTimer) {
  ThreadTrackDataManager thread_track_data_manager;

  // Add 2 timers for kThreadId1 and 1 timer for kThreadId2
  TimerInfo timer_info;
  timer_info.set_thread_id(kThreadId1);
  thread_track_data_manager.CreateScopeTreeTimerData(kThreadId1);
  thread_track_data_manager.AddTimer(timer_info);
  thread_track_data_manager.AddTimer(timer_info);

  EXPECT_EQ(thread_track_data_manager.GetAllScopeTreeTimerData().size(), 1);
  EXPECT_FALSE(thread_track_data_manager.GetScopeTreeTimerData(kThreadId1)->IsEmpty());

  timer_info.set_thread_id(kThreadId2);
  // Adding a timer without creating the data before should also work.
  thread_track_data_manager.AddTimer(timer_info);

  EXPECT_EQ(thread_track_data_manager.GetAllScopeTreeTimerData().size(), 2);
  EXPECT_FALSE(thread_track_data_manager.GetScopeTreeTimerData(kThreadId2)->IsEmpty());
}

}  // namespace orbit_client_data