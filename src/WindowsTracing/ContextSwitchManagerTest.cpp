// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ContextSwitchManager.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

using ::testing::SaveArg;

namespace orbit_windows_tracing {

class MockTracerListener : public orbit_windows_tracing::TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
};

TEST(ContextSwitch, ListenerIsCalled) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(1);

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);
}

TEST(ContextSwitch, InvalidPidIsSet) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  ContextSwitchManager manager(&mock_listener);

  orbit_grpc_protos::SchedulingSlice scheduling_slice;
  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(1).WillOnce(SaveArg<0>(&scheduling_slice));

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);

  EXPECT_EQ(scheduling_slice.pid(), orbit_base::kInvalidProcessId);
  EXPECT_EQ(scheduling_slice.tid(), 2);
}

TEST(ContextSwitch, ValidPidIsSet) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  ContextSwitchManager manager(&mock_listener);

  orbit_grpc_protos::SchedulingSlice scheduling_slice;
  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(1).WillOnce(SaveArg<0>(&scheduling_slice));

  manager.ProcessTidToPidMapping(/*tid*/ 2, /*pid=*/3);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);

  EXPECT_EQ(scheduling_slice.pid(), 3);
}

TEST(ContextSwitch, Stats) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(2);

  // TidToPid event not related to context switch below.
  manager.ProcessTidToPidMapping(/*tid*/ 123, /*pid=*/456);

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);

  const ContextSwitchManager::Stats& stats = manager.GetStats();
  EXPECT_EQ(stats.num_processed_thread_events_, 1);
  EXPECT_EQ(stats.num_processed_cpu_events_, 2);
  EXPECT_EQ(stats.num_scheduling_slices, 1);
  EXPECT_EQ(stats.num_tid_mismatches_, 0);
  EXPECT_EQ(stats.num_scheduling_slices_with_invalid_pid, 1);
  EXPECT_EQ(stats.tid_witout_pid_set_.size(), 1);
  EXPECT_TRUE(stats.tid_witout_pid_set_.find(/*tid=*/2) != stats.tid_witout_pid_set_.end());

  manager.ProcessTidToPidMapping(/*tid*/ 2, /*pid=*/3);
  manager.ProcessContextSwitch(/*cpu=*/1, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/1, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);

  EXPECT_EQ(stats.num_processed_thread_events_, 2);
  EXPECT_EQ(stats.num_processed_cpu_events_, 4);
  EXPECT_EQ(stats.num_scheduling_slices, 2);
  EXPECT_EQ(stats.num_tid_mismatches_, 0);
  EXPECT_EQ(stats.num_scheduling_slices_with_invalid_pid, 1);
  EXPECT_EQ(stats.tid_witout_pid_set_.size(), 1);
}

TEST(ContextSwitch, TidMismatch) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(0);

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/0, /*new_tid=*/3, /*timestamp_ns=*/1);

  const ContextSwitchManager::Stats& stats = manager.GetStats();
  EXPECT_EQ(stats.num_tid_mismatches_, 1);
}

}  // namespace orbit_windows_tracing
