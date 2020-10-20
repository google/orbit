// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "UprobesFunctionCallManager.h"

namespace LinuxTracing {

using orbit_grpc_protos::FunctionCall;

TEST(UprobesFunctionCallManager, OneUprobe) {
  constexpr pid_t pid = 41;
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;
  perf_event_sample_regs_user_sp_ip_arguments registers;

  function_call_manager.ProcessUprobes(tid, 100, 1, registers);

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid, 2, 3);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), pid);
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 100);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 3);
  EXPECT_EQ(processed_function_call.value().registers_size(), 6);
}

TEST(UprobesFunctionCallManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t pid = 41;
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;
  perf_event_sample_regs_user_sp_ip_arguments registers;

  function_call_manager.ProcessUprobes(tid, 100, 1, registers);

  function_call_manager.ProcessUprobes(tid, 200, 2, registers);

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid, 3, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), pid);
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 200);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 3);
  EXPECT_EQ(processed_function_call.value().depth(), 1);
  EXPECT_EQ(processed_function_call.value().return_value(), 4);
  EXPECT_EQ(processed_function_call.value().registers_size(), 6);

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid, 4, 5);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), pid);
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 100);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 4);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 5);
  EXPECT_EQ(processed_function_call.value().registers_size(), 6);

  function_call_manager.ProcessUprobes(tid, 300, 5, registers);

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid, 6, 7);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), pid);
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 300);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 5);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 6);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 7);
  EXPECT_EQ(processed_function_call.value().registers_size(), 6);
}

TEST(UprobesFunctionCallManager, TwoUprobesDifferentThreads) {
  constexpr pid_t pid = 41;
  constexpr pid_t tid = 42;
  constexpr pid_t tid2 = 111;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;
  perf_event_sample_regs_user_sp_ip_arguments registers;

  function_call_manager.ProcessUprobes(tid, 100, 1, registers);

  function_call_manager.ProcessUprobes(tid2, 200, 2, registers);

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid, 3, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), pid);
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 100);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 3);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 4);
  EXPECT_EQ(processed_function_call.value().registers_size(), 6);

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid2, 4, 5);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), pid);
  EXPECT_EQ(processed_function_call.value().tid(), tid2);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 200);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 4);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 5);
  EXPECT_EQ(processed_function_call.value().registers_size(), 6);
}

TEST(UprobesFunctionCallManager, OnlyUretprobe) {
  constexpr pid_t pid = 41;
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  processed_function_call = function_call_manager.ProcessUretprobes(pid, tid, 2, 3);
  ASSERT_FALSE(processed_function_call.has_value());
}

}  // namespace LinuxTracing
