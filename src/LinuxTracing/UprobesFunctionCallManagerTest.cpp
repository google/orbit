// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <memory>
#include <optional>

#include "PerfEventRecords.h"
#include "UprobesFunctionCallManager.h"
#include "capture.pb.h"

using orbit_grpc_protos::FunctionCall;
using ::testing::ElementsAre;

namespace orbit_linux_tracing {

static constexpr perf_event_sample_regs_user_sp_ip_arguments kRegisters{
    .abi = PERF_SAMPLE_REGS_ABI_64,
    .cx = 4,
    .dx = 3,
    .si = 2,
    .di = 1,
    .sp = 0,
    .ip = 0,
    .r8 = 5,
    .r9 = 6,
};

TEST(UprobesFunctionCallManager, OneFunctionCallWithoutArgumentsAndWithoutReturnValue) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(kTid, 100, 1, std::nullopt);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 2, std::nullopt);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 100);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 0);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre());
}

TEST(UprobesFunctionCallManager, OneFunctionCallWithArgumentsAndWithoutReturnValue) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(kTid, 100, 1, kRegisters);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 2, std::nullopt);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 100);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 0);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(UprobesFunctionCallManager, OneFunctionCallWithoutArgumentsAndWithReturnValue) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(kTid, 100, 1, std::nullopt);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 2, 1234);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 100);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 1234);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre());
}

TEST(UprobesFunctionCallManager, OneFunctionCallWithArgumentsAndWithReturnValue) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(kTid, 100, 1, kRegisters);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 2, 1234);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 100);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 2);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 1234);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(UprobesFunctionCallManager, TwoNestedFunctionCallsAndAnotherFunctionCall) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(kTid, 100, 1, kRegisters);

  function_call_manager.ProcessUprobes(kTid, 200, 2, kRegisters);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 3, 1234);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 200);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 3);
  EXPECT_EQ(processed_function_call.value().depth(), 1);
  EXPECT_EQ(processed_function_call.value().return_value(), 1234);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre(1, 2, 3, 4, 5, 6));

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 4, 1235);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 100);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 3);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 4);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 1235);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre(1, 2, 3, 4, 5, 6));

  function_call_manager.ProcessUprobes(kTid, 300, 5, std::nullopt);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 6, std::nullopt);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid);
  EXPECT_EQ(processed_function_call.value().function_id(), 300);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 1);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 6);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 0);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre());
}

TEST(UprobesFunctionCallManager, TwoFunctionCallsOnDifferentThreads) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid1 = 42;
  constexpr pid_t kTid2 = 111;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(kTid1, 100, 1, kRegisters);

  function_call_manager.ProcessUprobes(kTid2, 200, 2, std::nullopt);

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid1, 3, std::nullopt);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid1);
  EXPECT_EQ(processed_function_call.value().function_id(), 100);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 2);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 3);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 0);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre(1, 2, 3, 4, 5, 6));

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid2, 4, 1234);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().pid(), kPid);
  EXPECT_EQ(processed_function_call.value().tid(), kTid2);
  EXPECT_EQ(processed_function_call.value().function_id(), 200);
  EXPECT_EQ(processed_function_call.value().duration_ns(), 2);
  EXPECT_EQ(processed_function_call.value().end_timestamp_ns(), 4);
  EXPECT_EQ(processed_function_call.value().depth(), 0);
  EXPECT_EQ(processed_function_call.value().return_value(), 1234);
  EXPECT_THAT(processed_function_call.value().registers(), ElementsAre());
}

TEST(UprobesFunctionCallManager, OnlyUretprobeNoFunctionCall) {
  constexpr pid_t kPid = 41;
  constexpr pid_t kTid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  processed_function_call = function_call_manager.ProcessUretprobes(kPid, kTid, 2, 1234);
  ASSERT_FALSE(processed_function_call.has_value());
}

}  // namespace orbit_linux_tracing
