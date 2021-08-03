// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdint>
#include <random>
#include <string>
#include <string_view>

#include "AddressRange.h"
#include "FindFunctionAddress.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/TestUtils.h"
#include "TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InstrumentProcess.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::HasError;
using orbit_base::HasNoError;

orbit_grpc_protos::CaptureOptions GetCaptureOptions() {
  orbit_grpc_protos::CaptureOptions capture_options;
  constexpr const char* kFunctionName = "SomethingToInstrument";
  AddressRange range = GetFunctionRelativeAddressRangeOrDie(kFunctionName);
  orbit_grpc_protos::InstrumentedFunction* my_function =
      capture_options.add_instrumented_functions();
  my_function->set_function_id(42);
  my_function->set_file_offset(range.start);
  my_function->set_function_size(range.end - range.start);
  my_function->set_function_name(kFunctionName);
  my_function->set_file_path(orbit_base::GetExecutablePath());
  return capture_options;
}

}  // namespace

extern "C" int SomethingToInstrument() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(1, 6);
  return dis(gen);
}

TEST(InstrumentProcessTest, FailToInstrumentAlreadyAttached) {
  // Skip if not running as root. We need to trace a child process.
  if (geteuid() != 0) {
    GTEST_SKIP();
  }

  const pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    while (true) {
    }
  }

  // We spawn another child and wait for it to trace `pid`. Then we can't attach.
  const pid_t pid_tracer = fork();
  CHECK(pid_tracer != -1);
  if (pid_tracer == 0) {
    ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
    while (true) {
    }
  }
  bool already_tracing = false;
  while (!already_tracing) {
    auto tracer_pid_or_error = orbit_base::GetTracerPidOfProcess(pid);
    CHECK(!tracer_pid_or_error.has_error());
    already_tracing = tracer_pid_or_error.value() != 0;
  }

  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(pid);
  auto function_ids_or_error = InstrumentProcess(capture_options);
  ASSERT_THAT(function_ids_or_error, HasError("is already being traced by"));

  // End tracer process, end child process.
  kill(pid_tracer, SIGKILL);
  waitpid(pid_tracer, NULL, 0);
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

TEST(InstrumentProcessTest, FailToInstrumentInvalidPid) {
  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(-1);
  auto function_ids_or_error = InstrumentProcess(capture_options);
  ASSERT_THAT(function_ids_or_error, HasError("There is no process with pid"));
}

TEST(InstrumentProcessTest, FailToInstrumentThisProcess) {
  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(getpid());
  auto function_ids_or_error = InstrumentProcess(capture_options);
  // We do not fail but just instrument nothing.
  ASSERT_THAT(function_ids_or_error, HasNoError());
  EXPECT_TRUE(function_ids_or_error.value().empty());
}

TEST(InstrumentProcessTest, Instrument) {
  const pid_t pid_process_1 = fork();
  CHECK(pid_process_1 != -1);
  if (pid_process_1 == 0) {
    int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  orbit_grpc_protos::CaptureOptions capture_options = GetCaptureOptions();
  capture_options.set_pid(pid_process_1);
  auto function_ids_or_error = InstrumentProcess(capture_options);
  ASSERT_THAT(function_ids_or_error, HasNoError());
  EXPECT_TRUE(function_ids_or_error.value().contains(42));
  auto result = UninstrumentProcess(capture_options);
  ASSERT_THAT(result, HasNoError());

  // End child pid_process_1.
  kill(pid_process_1, SIGKILL);
  waitpid(pid_process_1, NULL, 0);

  // Just do the same thing with another process to trigger the code path deleting the data for the
  // first. Also Instrument / Uninstrument repeatedly.
  const pid_t pid_process_2 = fork();
  CHECK(pid_process_2 != -1);
  if (pid_process_2 == 0) {
    int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  capture_options.set_pid(pid_process_2);
  for (int i = 0; i < 5; i++) {
    function_ids_or_error = InstrumentProcess(capture_options);
    ASSERT_THAT(function_ids_or_error, HasNoError());
    EXPECT_TRUE(function_ids_or_error.value().contains(42));
    result = UninstrumentProcess(capture_options);
    ASSERT_THAT(result, HasNoError());
  }

  // End child pid_process_2.
  kill(pid_process_2, SIGKILL);
  waitpid(pid_process_2, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation