// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointEventBuffer.h"
#include "gtest/gtest.h"

TEST(TracepointEventBuffer, AddAndGetTracepointEvents) {
  TracepointEventBuffer tracepoint_event_buffer;

  tracepoint_event_buffer.AddTracepointEventAndMapToThreads(1, 0, 0, 1, 0, true);
  tracepoint_event_buffer.AddTracepointEventAndMapToThreads(2, 3, 2, 0, 1, true);
  tracepoint_event_buffer.AddTracepointEventAndMapToThreads(0, 1, 2, 1, 3, true);
  tracepoint_event_buffer.AddTracepointEventAndMapToThreads(7, 1, 2, 1, 3, true);

  tracepoint_event_buffer.AddTracepointEventAndMapToThreads(0, 1, 2, 6, 3, false);
  tracepoint_event_buffer.AddTracepointEventAndMapToThreads(10, 1, 2, 1, 3, false);

  EXPECT_EQ(tracepoint_event_buffer.GetNumTracepointsForThreadId(1), 3);
  EXPECT_EQ(tracepoint_event_buffer.GetNumTracepointsForThreadId(0), 1);

  /*The number of threacepoints for thread id 6 is 0 because this tracepoint does not belong in the
   * target process*/
  EXPECT_EQ(tracepoint_event_buffer.GetNumTracepointsForThreadId(6), 0);

  EXPECT_EQ(
      tracepoint_event_buffer.GetNumTracepointsForThreadId(SamplingProfiler::kAllThreadsFakeTid),
      4);
  EXPECT_EQ(tracepoint_event_buffer.GetNumTracepointsForThreadId(
                SamplingProfiler::kAllTracepointsFakeTid),
            6);

  /*Check the tracepoint events associated to the threads in the target process*/
  const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& tracepoints =
      tracepoint_event_buffer.GetTracepointsOfThread(1);

  ASSERT_TRUE(tracepoints.begin()->first == 0);
  ASSERT_TRUE((++tracepoints.begin())->first == 1);

  auto it = tracepoints.begin();

  ASSERT_TRUE(it->second.time() == 0 && it->second.tracepoint_info_key() == 1 &&
              it->second.pid() == 2 && it->second.cpu() == 3);

  std::vector<orbit_client_protos::TracepointEventInfo> tracepoints_of_thread_1;
  tracepoint_event_buffer.ForEachTracepointEventOfThreadInTimeRange(
      1, 0, 8,
      [&tracepoints_of_thread_1](
          const orbit_client_protos::TracepointEventInfo& tracepoint_event_info) {
        tracepoints_of_thread_1.emplace_back(tracepoint_event_info);
      });

  EXPECT_TRUE(tracepoints_of_thread_1.size() == 3);
  EXPECT_TRUE(tracepoints_of_thread_1[0].tracepoint_info_key() == 1 &&
              tracepoints_of_thread_1[1].tracepoint_info_key() == 0 &&
              tracepoints_of_thread_1[2].tracepoint_info_key() == 1);

  /*Check the retrieval of the tracepoint events from all the threads in the target process
   * in the timestamp between 0 and 3*/
  std::vector<orbit_client_protos::TracepointEventInfo> all_tracepoint_events_target_process;
  tracepoint_event_buffer.ForEachTracepointEventOfThreadInTimeRange(
      SamplingProfiler::kAllThreadsFakeTid, 0, 3,
      [&all_tracepoint_events_target_process](
          const orbit_client_protos::TracepointEventInfo& tracepoint_event_info) {
        all_tracepoint_events_target_process.emplace_back(tracepoint_event_info);
      });

  /*There are 3 events that are part of the target process that have the timestamp between 0 and 3.
   * Since they are ordered by thread id, the first event is the one with the thread id of 0, the
   * second with the tid 1 and the third with the tid 1. We verify the hash keys. For example, the
   * first hash key corresponds to the event of tid 0. therefore the hash key is 3.*/
  EXPECT_TRUE(all_tracepoint_events_target_process.size() == 3);
  EXPECT_TRUE(all_tracepoint_events_target_process[0].tracepoint_info_key() == 3 &&
              all_tracepoint_events_target_process[1].tracepoint_info_key() == 1 &&
              all_tracepoint_events_target_process[2].tracepoint_info_key() == 0);
}
