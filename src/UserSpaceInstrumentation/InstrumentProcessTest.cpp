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
#include "TestUtils.h"
#include "TestUtils/TestUtils.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InstrumentProcess.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using ::testing::HasSubstr;

constexpr int kFunctionId1 = 42;
constexpr int kFunctionId2 = 43;

orbit_grpc_protos::CaptureOptions BuildCaptureOptions() {
  orbit_grpc_protos::CaptureOptions capture_options;

  constexpr const char* kFunctionName1 = "SomethingToInstrument";
  AddressRange range = GetFunctionRelativeAddressRangeOrDie(kFunctionName1);
  orbit_grpc_protos::InstrumentedFunction* my_function =
      capture_options.add_instrumented_functions();
  my_function->set_function_id(kFunctionId1);
  my_function->set_file_offset(range.start);
  my_function->set_function_size(range.end - range.start);
  my_function->set_function_name(kFunctionName1);
  my_function->set_file_path(orbit_base::GetExecutablePath());

  constexpr const char* kFunctionName2 = "ReturnImmediately";
  range = GetFunctionRelativeAddressRangeOrDie(kFunctionName2);
  my_function = capture_options.add_instrumented_functions();
  my_function->set_function_id(kFunctionId2);
  my_function->set_file_offset(range.start);
  my_function->set_function_size(range.end - range.start);
  my_function->set_function_name(kFunctionName2);
  my_function->set_file_path(orbit_base::GetExecutablePath());

  return capture_options;
}

[[nodiscard]] InstrumentationManager* GetInstrumentationManager() {
  static std::unique_ptr<InstrumentationManager> m = InstrumentationManager::Create();
  return m.get();
}

}  // namespace

extern "C" int SomethingToInstrument() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(1, 6);
  return dis(gen);
}

// We will not be able to instrument this - the function is just one byte long and we need five
// bytes to write a jump.
extern "C" __attribute__((naked)) int ReturnImmediately() {
  __asm__ __volatile__("ret \n\t" : : :);
}

TEST(InstrumentProcessTest, FailToInstrumentAlreadyAttached) {
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  // Skip if not running as root. We need to trace a child process.
  if (geteuid() != 0) {
    GTEST_SKIP();
  }

  const pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // We spawn another child and wait for it to trace `pid`. Then we can't attach.
  const pid_t pid_tracer = fork();
  CHECK(pid_tracer != -1);
  if (pid_tracer == 0) {
    ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
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
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error, HasError("is already being traced by"));

  // End tracer process, end child process.
  kill(pid_tracer, SIGKILL);
  waitpid(pid_tracer, nullptr, 0);
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(InstrumentProcessTest, FailToInstrumentInvalidPid) {
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(-1);
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error, HasError("There is no process with pid"));
}

TEST(InstrumentProcessTest, FailToInstrumentThisProcess) {
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(getpid());
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  // We do not fail but just instrument nothing.
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_TRUE(result_or_error.value().instrumented_function_ids.empty());
}

TEST(InstrumentProcessTest, Instrument) {
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  const pid_t pid_process_1 = fork();
  CHECK(pid_process_1 != -1);
  if (pid_process_1 == 0) {
    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    volatile int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  orbit_grpc_protos::CaptureOptions capture_options = BuildCaptureOptions();
  capture_options.set_pid(pid_process_1);
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_TRUE(result_or_error.value().instrumented_function_ids.contains(kFunctionId1));
  EXPECT_FALSE(result_or_error.value().instrumented_function_ids.contains(kFunctionId2));
  ASSERT_EQ(result_or_error.value().error_messages.size(), 1);
  EXPECT_THAT(result_or_error.value().error_messages[0],
              HasSubstr("Failed to create trampoline: Unable to disassemble enough of the function "
                        "to instrument it. Code: c3"));
  auto result = instrumentation_manager->UninstrumentProcess(pid_process_1);
  ASSERT_THAT(result, HasNoError());

  // End child pid_process_1.
  kill(pid_process_1, SIGKILL);
  waitpid(pid_process_1, nullptr, 0);

  // Just do the same thing with another process to trigger the code path deleting the data for the
  // first. Also Instrument / Uninstrument repeatedly.
  const pid_t pid_process_2 = fork();
  CHECK(pid_process_2 != -1);
  if (pid_process_2 == 0) {
    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    volatile int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  capture_options.set_pid(pid_process_2);
  for (int i = 0; i < 5; i++) {
    result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
    ASSERT_THAT(result_or_error, HasNoError());
    EXPECT_TRUE(result_or_error.value().instrumented_function_ids.contains(kFunctionId1));
    result = instrumentation_manager->UninstrumentProcess(pid_process_2);
    ASSERT_THAT(result, HasNoError());
  }

  // End child pid_process_2.
  kill(pid_process_2, SIGKILL);
  waitpid(pid_process_2, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation