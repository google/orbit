// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointInfoManager.h"
#include "gtest/gtest.h"

TEST(TracepointInfoManager, Contains) {
  TracepointInfoManager tracepoint_info_manager;

  tracepoint_info_manager.AddUniqueTracepointEventInfo(1, orbit_grpc_protos::TracepointInfo{});
  EXPECT_TRUE(tracepoint_info_manager.Contains(1));
  EXPECT_FALSE(tracepoint_info_manager.Contains(0));
}

TEST(TracepointInfoManager, AddUniqueTracepointEventInfo) {
  TracepointInfoManager tracepoint_info_manager;

  EXPECT_TRUE(
      tracepoint_info_manager.AddUniqueTracepointEventInfo(1, orbit_grpc_protos::TracepointInfo{}));
  EXPECT_TRUE(tracepoint_info_manager.Contains(1));

  EXPECT_FALSE(
      tracepoint_info_manager.AddUniqueTracepointEventInfo(1, orbit_grpc_protos::TracepointInfo{}));
  EXPECT_TRUE(
      tracepoint_info_manager.AddUniqueTracepointEventInfo(2, orbit_grpc_protos::TracepointInfo{}));
  EXPECT_TRUE(tracepoint_info_manager.Contains(2));
}

TEST(TracepointInfoManager, Get) {
  TracepointInfoManager tracepoint_info_manager;

  orbit_grpc_protos::TracepointInfo tracepoint_info;
  tracepoint_info.set_category("sched");
  tracepoint_info.set_name("sched_switch");

  EXPECT_TRUE(
      tracepoint_info_manager.AddUniqueTracepointEventInfo(1, orbit_grpc_protos::TracepointInfo{}));
  EXPECT_TRUE(
      tracepoint_info_manager.AddUniqueTracepointEventInfo(2, orbit_grpc_protos::TracepointInfo{}));
  EXPECT_TRUE(tracepoint_info_manager.AddUniqueTracepointEventInfo(3, tracepoint_info));
  EXPECT_FALSE(
      tracepoint_info_manager.AddUniqueTracepointEventInfo(1, orbit_grpc_protos::TracepointInfo{}));

  EXPECT_TRUE(tracepoint_info_manager.Get(3).category().compare("sched") == 0 &&
              tracepoint_info_manager.Get(3).name().compare("sched_switch") == 0);
  EXPECT_FALSE(tracepoint_info_manager.Get(2).category().compare("sched") == 0 &&
               tracepoint_info_manager.Get(2).name().compare("sched_switch") == 0);
}