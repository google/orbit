// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <sys/types.h>

#include <optional>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "ThreadStateManager.h"

using orbit_grpc_protos::ThreadStateSlice;

namespace orbit_linux_tracing {

constexpr pid_t kWakeupPidTidWhenWakeupReasonNotApplicable = 0;
constexpr pid_t kSwitchOutOrWakeupCallstackIdNotApplicable = 0;

TEST(ThreadStateManager, OneThread) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchOut(300, kTid, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedWakeup(400, kTid, kWasUnblockedByTid, kWasUnblockedByPid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchIn(500, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 500);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kUnblocked);
  EXPECT_EQ(slice->wakeup_pid(), kWasUnblockedByPid);
  EXPECT_EQ(slice->wakeup_tid(), kWasUnblockedByTid);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  std::vector<ThreadStateSlice> slices = manager.OnCaptureFinished(600);
  ASSERT_TRUE(!slices.empty());
  EXPECT_EQ(slices.size(), 1);
  slice = slices[0];
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 600);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, NewTask) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasCreatedByTid = 420;
  constexpr pid_t kWasCreatedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnNewTask(100, kTid, kWasCreatedByTid, kWasCreatedByPid);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kCreated);
  EXPECT_EQ(slice->wakeup_pid(), kWasCreatedByPid);
  EXPECT_EQ(slice->wakeup_tid(), kWasCreatedByTid);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchOut(300, kTid, ThreadStateSlice::kRunnable);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  std::vector<ThreadStateSlice> slices = manager.OnCaptureFinished(400);
  ASSERT_TRUE(!slices.empty());
  EXPECT_EQ(slices.size(), 1);
  slice = slices[0];
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, TwoThreads) {
  constexpr pid_t kTid1 = 42;
  constexpr pid_t kTid2 = 52;
  constexpr pid_t kWasUnblockedByTid1 = 420;
  constexpr pid_t kWasUnblockedByPid1 = 4200;
  constexpr pid_t kWasCreatedByTid2 = 520;
  constexpr pid_t kWasCreatedByPid2 = 5200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid1, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchIn(200, kTid1);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  manager.OnNewTask(250, kTid2, kWasCreatedByTid2, kWasCreatedByPid2);

  slice = manager.OnSchedSwitchOut(300, kTid1, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchIn(350, kTid2);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid2);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 350);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kCreated);
  EXPECT_EQ(slice->wakeup_pid(), kWasCreatedByPid2);
  EXPECT_EQ(slice->wakeup_tid(), kWasCreatedByTid2);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedWakeup(400, kTid1, kWasUnblockedByTid1, kWasUnblockedByPid1);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchOut(450, kTid2, ThreadStateSlice::kRunnable);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid2);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 450);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchIn(500, kTid1);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid1);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 500);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kUnblocked);
  EXPECT_EQ(slice->wakeup_pid(), kWasUnblockedByPid1);
  EXPECT_EQ(slice->wakeup_tid(), kWasUnblockedByTid1);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

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
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = slices[1];
  EXPECT_EQ(slice->tid(), kTid2);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 150);
  EXPECT_EQ(slice->end_timestamp_ns(), 600);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
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
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, StaleInitialStateWithNewTask) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasCreatedByTid = 420;
  constexpr pid_t kWasCreatedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  manager.OnNewTask(100, kTid, kWasCreatedByTid, kWasCreatedByPid);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kCreated);
  EXPECT_EQ(slice->wakeup_pid(), kWasCreatedByPid);
  EXPECT_EQ(slice->wakeup_tid(), kWasCreatedByTid);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, StaleInitialStateWithSchedWakeup) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedWakeup(100, kTid, kWasUnblockedByTid, kWasUnblockedByPid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kUnblocked);
  EXPECT_EQ(slice->wakeup_pid(), kWasUnblockedByPid);
  EXPECT_EQ(slice->wakeup_tid(), kWasUnblockedByTid);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
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
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, StaleInitialStateWithSwitchOut) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(150, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchOut(100, kTid, ThreadStateSlice::kInterruptibleSleep);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedWakeup(200, kTid, kWasUnblockedByTid, kWasUnblockedByPid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, UnknownInitialStateWithSchedWakeup) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  slice = manager.OnSchedWakeup(100, kTid, kWasUnblockedByTid, kWasUnblockedByPid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kUnblocked);
  EXPECT_EQ(slice->wakeup_pid(), kWasUnblockedByPid);
  EXPECT_EQ(slice->wakeup_tid(), kWasUnblockedByTid);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
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
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, UnknownInitialStateWithSwitchOut) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  slice = manager.OnSchedSwitchOut(100, kTid, ThreadStateSlice::kInterruptibleSleep);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedWakeup(200, kTid, kWasUnblockedByTid, kWasUnblockedByPid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, NoStateChangeWithSchedWakeup) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedWakeup(150, kTid, kWasUnblockedByTid, kWasUnblockedByPid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, NoStateChangeWithSwitchIn) {
  constexpr pid_t kTid = 42;
  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunnable);

  slice = manager.OnSchedSwitchIn(200, kTid);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchIn(250, kTid);
  EXPECT_FALSE(slice.has_value());

  slice = manager.OnSchedSwitchOut(300, kTid, ThreadStateSlice::kInterruptibleSleep);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

TEST(ThreadStateManager, SwitchOutAndWakeupWaitForCallstacks) {
  constexpr pid_t kTid = 42;
  constexpr pid_t kWasUnblockedByTid = 420;
  constexpr pid_t kWasUnblockedByPid = 4200;

  ThreadStateManager manager;
  std::optional<ThreadStateSlice> slice;

  manager.OnInitialState(100, kTid, ThreadStateSlice::kRunning);

  slice = manager.OnSchedSwitchOut(200, kTid, ThreadStateSlice::kInterruptibleSleep,
                                   /*has_wakeup_or_switch_out_callstack*/ true);
  ASSERT_TRUE(slice.has_value());
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 200);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedWakeup(300, kTid, kWasUnblockedByTid, kWasUnblockedByPid,
                                /*has_wakeup_or_switch_out_callstack*/ true);
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kInterruptibleSleep);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 300);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kWaitingForCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchIn(400, kTid);
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(slice->duration_ns(), 100);
  EXPECT_EQ(slice->end_timestamp_ns(), 400);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kUnblocked);
  EXPECT_EQ(slice->wakeup_pid(), kWasUnblockedByPid);
  EXPECT_EQ(slice->wakeup_tid(), kWasUnblockedByTid);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kWaitingForCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);

  slice = manager.OnSchedSwitchOut(600, kTid, ThreadStateSlice::kDead,
                                   /*has_wakeup_or_switch_out_callstack*/ false);
  EXPECT_EQ(slice->tid(), kTid);
  EXPECT_EQ(slice->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(slice->duration_ns(), 200);
  EXPECT_EQ(slice->end_timestamp_ns(), 600);
  EXPECT_EQ(slice->wakeup_reason(), orbit_grpc_protos::ThreadStateSlice::kNotApplicable);
  EXPECT_EQ(slice->wakeup_pid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->wakeup_tid(), kWakeupPidTidWhenWakeupReasonNotApplicable);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_status(),
            orbit_grpc_protos::ThreadStateSlice::kNoCallstack);
  EXPECT_EQ(slice->switch_out_or_wakeup_callstack_id(), kSwitchOutOrWakeupCallstackIdNotApplicable);
}

}  // namespace orbit_linux_tracing
