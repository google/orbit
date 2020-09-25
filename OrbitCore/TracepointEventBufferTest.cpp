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

  const std::map<uint64_t, orbit_client_protos::TracepointEventInfo>& tracepoints =
      tracepoint_event_buffer.GetTracepointsOfThread(1);

  ASSERT_TRUE(tracepoints.begin()->first == 0);
  ASSERT_TRUE((++tracepoints.begin())->first == 1);

  auto it = tracepoints.begin();

  ASSERT_TRUE(it->second.time() == 0 && it->second.tracepoint_info_key() == 1 &&
              it->second.pid() == 2 && it->second.cpu() == 3);

}
