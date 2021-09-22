// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ContextSwitchManager.h"
#include "MockTracerListener.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

TEST(ContextSwitch, ListenerIsCalled) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(testing::Exactly(1));

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);
}

TEST(ContextSwitch, InvalidPidIsSet) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  orbit_windows_tracing::FakeTracerListener& fake_listener = mock_listener.fake_;
  mock_listener.DelegateToFake();
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(testing::Exactly(1));

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);

  EXPECT_EQ(fake_listener.scheduling_slices_.size(), 1);
  EXPECT_EQ(fake_listener.scheduling_slices_[0].pid(), orbit_base::kInvalidProcessId);
  EXPECT_EQ(fake_listener.scheduling_slices_[0].tid(), 2);
}

TEST(ContextSwitch, ValidPidIsSet) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  orbit_windows_tracing::FakeTracerListener& fake_listener = mock_listener.fake_;
  mock_listener.DelegateToFake();
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(testing::Exactly(1));

  manager.ProcessTidToPidMapping(/*tid*/ 2, /*pid=*/3);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/2, /*new_tid=*/1, /*timestamp_ns=*/1);

  EXPECT_EQ(fake_listener.scheduling_slices_.size(), 1);
  EXPECT_EQ(fake_listener.scheduling_slices_[0].pid(), 3);
}

TEST(ContextSwitch, Stats) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  orbit_windows_tracing::FakeTracerListener& fake_listener = mock_listener.fake_;
  mock_listener.DelegateToFake();
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(testing::Exactly(2));

  // Thread event that has nothing to do with cpu events below.
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
  EXPECT_EQ(fake_listener.scheduling_slices_[1].pid(), 3);
  EXPECT_EQ(fake_listener.scheduling_slices_[1].tid(), 2);
}

TEST(ContextSwitch, TidMismatch) {
  orbit_windows_tracing::MockTracerListener mock_listener;
  orbit_windows_tracing::FakeTracerListener& fake_listener = mock_listener.fake_;
  mock_listener.DelegateToFake();
  ContextSwitchManager manager(&mock_listener);

  EXPECT_CALL(mock_listener, OnSchedulingSlice).Times(testing::Exactly(0));

  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/1, /*new_tid=*/2, /*timestamp_ns=*/0);
  manager.ProcessContextSwitch(/*cpu=*/0, /*old_tid=*/0, /*new_tid=*/3, /*timestamp_ns=*/1);

  const ContextSwitchManager::Stats& stats = manager.GetStats();
  EXPECT_EQ(stats.num_tid_mismatches_, 1);
}

}  // namespace orbit_windows_tracing
