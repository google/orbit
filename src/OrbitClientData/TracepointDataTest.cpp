// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "OrbitBase/ThreadConstants.h"
#include "OrbitClientData/TracepointData.h"
#include "capture_data.pb.h"
#include "gtest/gtest.h"
#include "tracepoint.pb.h"

using ::testing::UnorderedElementsAre;

TEST(TracepointData, AddAndGetTracepointEvents) {
  TracepointData tracepoint_data;

  tracepoint_data.AddUniqueTracepointInfo(0, {});
  tracepoint_data.AddUniqueTracepointInfo(1, {});
  tracepoint_data.AddUniqueTracepointInfo(3, {});

  tracepoint_data.EmplaceTracepointEvent(1, 0, 0, 1, 0, true);
  tracepoint_data.EmplaceTracepointEvent(2, 3, 2, 0, 1, true);
  tracepoint_data.EmplaceTracepointEvent(0, 1, 2, 1, 3, true);
  tracepoint_data.EmplaceTracepointEvent(7, 1, 2, 1, 3, true);

  tracepoint_data.EmplaceTracepointEvent(0, 1, 2, 6, 3, false);
  tracepoint_data.EmplaceTracepointEvent(10, 1, 2, 1, 3, false);

  EXPECT_EQ(tracepoint_data.GetNumTracepointEventsForThreadId(1), 3);
  EXPECT_EQ(tracepoint_data.GetNumTracepointEventsForThreadId(0), 1);

  /*The number of tracepoints for thread id 6 is 0 because this tracepoint does not belong in the
   * target process*/
  EXPECT_EQ(tracepoint_data.GetNumTracepointEventsForThreadId(6), 0);
  EXPECT_EQ(tracepoint_data.GetNumTracepointEventsForThreadId(orbit_base::kAllProcessThreadsTid),
            4);
  EXPECT_EQ(
      tracepoint_data.GetNumTracepointEventsForThreadId(orbit_base::kAllThreadsOfAllProcessesTid),
      6);

  std::vector<uint64_t> tracepoints_of_thread_1;
  tracepoint_data.ForEachTracepointEventOfThreadInTimeRange(
      1, 0, 8,
      [&tracepoints_of_thread_1](
          const orbit_client_protos::TracepointEventInfo& tracepoint_event_info) {
        tracepoints_of_thread_1.emplace_back(tracepoint_event_info.tracepoint_info_key());
      });

  EXPECT_THAT(tracepoints_of_thread_1, UnorderedElementsAre(0, 1, 1));

  /*Check the retrieval of the tracepoint events from all the threads in the target process
   * in the timestamp between 0 and 3*/
  std::vector<uint64_t> all_tracepoint_events_target_process;
  tracepoint_data.ForEachTracepointEventOfThreadInTimeRange(
      orbit_base::kAllProcessThreadsTid, 0, 3,
      [&all_tracepoint_events_target_process](
          const orbit_client_protos::TracepointEventInfo& tracepoint_event_info) {
        all_tracepoint_events_target_process.emplace_back(
            tracepoint_event_info.tracepoint_info_key());
      });

  EXPECT_THAT(all_tracepoint_events_target_process, UnorderedElementsAre(0, 1, 3));
}

TEST(TracepointData, Contains) {
  TracepointData tracepoint_data;

  tracepoint_data.AddUniqueTracepointInfo(1, {});
  EXPECT_TRUE(tracepoint_data.HasTracepointKey(1));
  EXPECT_FALSE(tracepoint_data.HasTracepointKey(0));
}

TEST(TracepointData, AddUniqueTracepointEventInfo) {
  TracepointData tracepoint_info_manager;

  EXPECT_TRUE(tracepoint_info_manager.AddUniqueTracepointInfo(1, {}));
  EXPECT_TRUE(tracepoint_info_manager.HasTracepointKey(1));

  EXPECT_FALSE(tracepoint_info_manager.AddUniqueTracepointInfo(1, {}));
  EXPECT_TRUE(tracepoint_info_manager.AddUniqueTracepointInfo(2, {}));
  EXPECT_TRUE(tracepoint_info_manager.HasTracepointKey(2));
}

TEST(TracepointData, Get) {
  TracepointData tracepoint_data;

  orbit_grpc_protos::TracepointInfo tracepoint_info;
  tracepoint_info.set_category("sched");
  tracepoint_info.set_name("sched_switch");

  EXPECT_TRUE(tracepoint_data.AddUniqueTracepointInfo(1, {}));
  EXPECT_TRUE(tracepoint_data.AddUniqueTracepointInfo(2, {}));
  EXPECT_TRUE(tracepoint_data.AddUniqueTracepointInfo(3, tracepoint_info));
  EXPECT_FALSE(tracepoint_data.AddUniqueTracepointInfo(1, {}));

  EXPECT_TRUE(tracepoint_data.GetTracepointInfo(3).category() == "sched" &&
              tracepoint_data.GetTracepointInfo(3).name() == "sched_switch");
  EXPECT_FALSE(tracepoint_data.GetTracepointInfo(2).category() == "sched" &&
               tracepoint_data.GetTracepointInfo(2).name() == "sched_switch");
}
