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
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 100);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 1);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 2);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
  EXPECT_EQ(processed_function_call.value().GetIntegerReturnValue(), 3);
}

TEST(UprobesFunctionCallManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  function_call_manager.ProcessUprobes(tid, 200, 2);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 3, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 200);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 2);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 3);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 1);
  EXPECT_EQ(processed_function_call.value().GetIntegerReturnValue(), 4);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 4, 5);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 100);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 1);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 4);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
  EXPECT_EQ(processed_function_call.value().GetIntegerReturnValue(), 5);

  function_call_manager.ProcessUprobes(tid, 300, 5);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 6, 7);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 300);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 5);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 6);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
  EXPECT_EQ(processed_function_call.value().GetIntegerReturnValue(), 7);
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
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 100);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 1);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 3);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
  EXPECT_EQ(processed_function_call.value().GetIntegerReturnValue(), 4);

  processed_function_call = function_call_manager.ProcessUretprobes(tid2, 4, 5);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid2);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 200);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 2);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 4);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
  EXPECT_EQ(processed_function_call.value().GetIntegerReturnValue(), 5);
}

TEST(UprobesFunctionCallManager, OnlyUretprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 2, 3);
  ASSERT_FALSE(processed_function_call.has_value());
}

}  // namespace LinuxTracing
