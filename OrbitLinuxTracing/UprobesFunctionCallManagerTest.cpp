// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "UprobesFunctionCallManager.h"

namespace LinuxTracing {

TEST(UprobesFunctionCallManager, OneUprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 2, 3);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 100);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 3);
}

TEST(UprobesFunctionCallManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  function_call_manager.ProcessUprobes(tid, 200, 2);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 3, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 200);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 3);
  EXPECT_EQ(processed_function_call.value().depth(), 1);
  EXPECT_EQ(processed_function_call.value().return_value(), 4);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 4, 5);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 100);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 4);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 5);

  function_call_manager.ProcessUprobes(tid, 300, 5);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 6, 7);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 300);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 5);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 6);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 7);
}

TEST(UprobesFunctionCallManager, TwoUprobesDifferentThreads) {
  constexpr pid_t tid = 42;
  constexpr pid_t tid2 = 111;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  function_call_manager.ProcessUprobes(tid2, 200, 2);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 3, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().tid(), tid);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 100);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 3);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 4);

  processed_function_call = function_call_manager.ProcessUretprobes(tid2, 4, 5);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().tid(), tid2);
  EXPECT_EQ(processed_function_call.value().absolute_address(), 200);
  EXPECT_EQ(processed_function_call.value().begin_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 4);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 5);
}

TEST(UprobesFunctionCallManager, OnlyUretprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 2, 3);
  ASSERT_FALSE(processed_function_call.has_value());
}

}  // namespace LinuxTracing
