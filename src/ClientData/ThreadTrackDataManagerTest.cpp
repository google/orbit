// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ThreadTrackDataManager.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;

TEST(ThreadTrackDataManager, IsEmpty) {
  ThreadTrackDataManager thread_track_data_manager;
  EXPECT_TRUE(thread_track_data_manager.GetAllScopeTreeTimerData().empty());
}

TEST(ThreadTrackDataManager, InsertOnceEachThreadId) {
  ThreadTrackDataManager thread_track_data_manager;

  uint32_t kThreadId1 = 1;
  uint32_t kThreadId2 = 2;

  thread_track_data_manager.GetOrCreateScopeTreeTimerData(kThreadId1);
  EXPECT_EQ(thread_track_data_manager.GetAllScopeTreeTimerData().size(), 1);

  thread_track_data_manager.GetOrCreateScopeTreeTimerData(kThreadId1);
  EXPECT_EQ(thread_track_data_manager.GetAllScopeTreeTimerData().size(), 1);

  thread_track_data_manager.GetOrCreateScopeTreeTimerData(kThreadId2);
  EXPECT_EQ(thread_track_data_manager.GetAllScopeTreeTimerData().size(), 2);
}

}  // namespace orbit_client_data