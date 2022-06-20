// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "ThreadStateManager.h"

namespace orbit_linux_tracing {

using orbit_grpc_protos::ThreadStateSlice;

TEST(ThreadStateManager, OneThread) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable, kWasBlockedByTid,
                         kWasBlockedByPid);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);

  slice = manager.OnSchedSwitchOut(300, kTid, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);

  slice = manager.OnSchedWakeup(400, kTid, kWasBlockedByTid, kWasBlockedByPid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);

  slice = manager.OnSchedSwitchIn(500, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 500);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);

  std::vector<ThreadStateSlice> slices = manager.OnCaptureFinished(600);
  ASSERT_TRUE(!slices.empty());
  EXPECT_EQ(slices.size(), 1);
  slice = slices[0];
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 600);
}

TEST(ThreadStateManager, NewTask) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnNewTask(100, kTid, kWasBlockedByTid, kWasBlockedByPid);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);

  slice = manager.OnSchedSwitchOut(300, kTid, ThreadStateSlice::kRunnable);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);

  std::vector<ThreadStateSlice> slices = manager.OnCaptureFinished(400);
  ASSERT_TRUE(!slices.empty());
  EXPECT_EQ(slices.size(), 1);
  slice = slices[0];
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);
}

TEST(ThreadStateManager, TwoThreads) {
  constexpr pid_t kTid1 = 42;
  constexpr pid_t kTid2 = 52;
  constexpr pid_t kWasBlockedByTid1 = 420;
  constexpr pid_t kWasBlockedByPid1 = 4200;
  constexpr pid_t kWasBlockedByTid2 = 520;
  constexpr pid_t kWasBlockedByPid2 = 5200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid1, ThreadStateSlice::kRunnable, kWasBlockedByTid1,
                         kWasBlockedByPid1);

  slice = manager.OnSchedSwitchIn(200, kTid1);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid1);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid1);

  manager.OnNewTask(250, kTid2, kWasBlockedByTid2, kWasBlockedByPid2);

  slice = manager.OnSchedSwitchOut(300, kTid1, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);

  slice = manager.OnSchedSwitchIn(350, kTid2);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid2);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 350);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid2);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid2);

  slice = manager.OnSchedWakeup(400, kTid1, kWasBlockedByTid1, kWasBlockedByPid1);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);

  slice = manager.OnSchedSwitchOut(450, kTid2, ThreadStateSlice::kRunnable);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid2);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 450);

  slice = manager.OnSchedSwitchIn(500, kTid1);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 500);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid1);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid1);

  std::vector<ThreadStateSlice> slices = manager.OnCaptureFinished(600);
  ASSERT_TRUE(slices.size() >= 2);
  EXPECT_EQ(slices.size(), 2);

  if (slices[0].tid() > slices[1].tid()) {
    std::swap(slices[0], slices[1]);
  }

  slice = slices[0];
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 600);

  slice = slices[1];
  EXPECT_EQ(slice->tid(), kTid2);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 150);
  EXPECT_EQ(slice->end_timestamp_ns(), 600);
}

TEST(ThreadStateManager, SwitchOutAfterInitialStateRunnable) {
  constexpr pid_t kTid = 42;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchOut(200, kTid, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
}

TEST(ThreadStateManager, StaleInitialStateWithNewTask) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  manager.OnNewTask(100, kTid, kWasBlockedByTid, kWasBlockedByPid);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);
}

TEST(ThreadStateManager, StaleInitialStateWithSchedWakeup) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedWakeup(100, kTid, kWasBlockedByTid, kWasBlockedByPid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);
}

TEST(ThreadStateManager, StaleInitialStateWithSwitchIn) {
  constexpr pid_t kTid = 42;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchIn(100, kTid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchOut(200, kTid, ThreadStateSlice::kRunnable);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
}

TEST(ThreadStateManager, StaleInitialStateWithSwitchOut) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchOut(100, kTid, ThreadStateSlice::kInterruptibleSleep);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedWakeup(200, kTid, kWasBlockedByTid, kWasBlockedByPid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), 0u);
  EXPECT_EQ(slice->was_blocked_by_thread(), 0u);
}

TEST(ThreadStateManager, UnknownInitialStateWithSchedWakeup) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  slice = manager.OnSchedWakeup(100, kTid, kWasBlockedByTid, kWasBlockedByPid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);
}

TEST(ThreadStateManager, UnknownInitialStateWithSwitchIn) {
  constexpr pid_t kTid = 42;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  slice = manager.OnSchedSwitchIn(100, kTid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchOut(200, kTid, ThreadStateSlice::kRunnable);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
}

TEST(ThreadStateManager, UnknownInitialStateWithSwitchOut) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  slice = manager.OnSchedSwitchOut(100, kTid, ThreadStateSlice::kInterruptibleSleep);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedWakeup(200, kTid, kWasBlockedByTid, kWasBlockedByPid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
}

TEST(ThreadStateManager, NoStateChangeWithSchedWakeup) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedWakeup(150, kTid, kWasBlockedByTid, kWasBlockedByPid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), 0u);
  EXPECT_EQ(slice->was_blocked_by_thread(), 0u);
}

TEST(ThreadStateManager, NoStateChangeWithSwitchIn) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasBlockedByTid = 420;
  constexpr pid_t kWasBlockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable, kWasBlockedByTid,
                         kWasBlockedByPid);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->was_blocked_by_process(), kWasBlockedByPid);
  EXPECT_EQ(slice->was_blocked_by_thread(), kWasBlockedByTid);

  slice = manager.OnSchedSwitchIn(250, kTid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchOut(300, kTid, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);
}

}  // namespace orbit_linux_tracing
