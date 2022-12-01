// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <linux/seccomp.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "TestUtils.h"
#include "TestUtils/TestUtils.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/AddressRange.h"
#include "UserSpaceInstrumentation/InstrumentProcess.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using ::testing::HasSubstr;

constexpr int kFunctionId1 = 42;
constexpr int kFunctionId2 = 43;

void AddFunctionToCaptureOptions(orbit_grpc_protos::CaptureOptions* capture_options,
                                 std::string_view function_name, int function_id) {
  const auto [module_file_path, range] = FindFunctionOrDie(function_name);
  orbit_grpc_protos::InstrumentedFunction* my_function =
      capture_options->add_instrumented_functions();
  my_function->set_function_id(function_id);
  my_function->set_function_virtual_address(range.start);
  my_function->set_function_size(range.end - range.start);
  my_function->set_function_name(std::string{function_name});
  my_function->set_file_path(module_file_path);
}

orbit_grpc_protos::CaptureOptions BuildCaptureOptions() {
  orbit_grpc_protos::CaptureOptions capture_options;

  AddFunctionToCaptureOptions(&capture_options, "SomethingToInstrument", kFunctionId1);
  AddFunctionToCaptureOptions(&capture_options, "ReturnImmediately", kFunctionId2);

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
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    volatile uint64_t counter = 0;
    while (true) {
      // Endless loops without side effects are UB and recent versions of clang optimize it away.
      ++counter;
    }
  }

  // We spawn another child and wait for it to trace `pid`. Then we can't attach.
  const pid_t pid_tracer = fork();
  ORBIT_CHECK(pid_tracer != -1);
  if (pid_tracer == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

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
    ORBIT_CHECK(!tracer_pid_or_error.has_error());
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
  ASSERT_THAT(result_or_error, HasError("The target process is OrbitService itself."));
}

static void VerifyTrampolineAddressRangesAndLibraryPath(
    const InstrumentationManager::InstrumentationResult& instrumentation_result) {
  EXPECT_EQ(instrumentation_result.entry_trampoline_address_ranges.size(), 1);
  if (!instrumentation_result.entry_trampoline_address_ranges.empty()) {
    EXPECT_EQ(instrumentation_result.entry_trampoline_address_ranges.at(0).end -
                  instrumentation_result.entry_trampoline_address_ranges.at(0).start,
              4096 * GetMaxTrampolineSize());
  }

  EXPECT_EQ(instrumentation_result.return_trampoline_address_range.end -
                instrumentation_result.return_trampoline_address_range.start,
            GetReturnTrampolineSize());
  /* copybara:strip_begin(In the internal build the library name can be different.) */

  EXPECT_EQ(instrumentation_result.injected_library_path.filename().string(),
            "liborbituserspaceinstrumentation.so");
  /* copybara:strip_end */
}

TEST(InstrumentProcessTest, Instrument) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  const pid_t pid_process_1 = fork();
  ORBIT_CHECK(pid_process_1 != -1);
  if (pid_process_1 == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    [[maybe_unused]] volatile int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  orbit_grpc_protos::CaptureOptions capture_options = BuildCaptureOptions();
  capture_options.set_pid(pid_process_1);
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_TRUE(result_or_error.value().instrumented_function_ids.contains(kFunctionId1));
  VerifyTrampolineAddressRangesAndLibraryPath(result_or_error.value());
  auto result = instrumentation_manager->UninstrumentProcess(pid_process_1);
  ASSERT_THAT(result, HasNoError());

  // End child pid_process_1.
  kill(pid_process_1, SIGKILL);
  waitpid(pid_process_1, nullptr, 0);

  // Just do the same thing with another process to trigger the code path deleting the data for the
  // first. Also Instrument / Uninstrument repeatedly.
  const pid_t pid_process_2 = fork();
  ORBIT_CHECK(pid_process_2 != -1);
  if (pid_process_2 == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    [[maybe_unused]] volatile int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  capture_options.set_pid(pid_process_2);
  for (int i = 0; i < 5; i++) {
    result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
    ASSERT_THAT(result_or_error, HasNoError());
    EXPECT_TRUE(result_or_error.value().instrumented_function_ids.contains(kFunctionId1));
    VerifyTrampolineAddressRangesAndLibraryPath(result_or_error.value());
    result = instrumentation_manager->UninstrumentProcess(pid_process_2);
    ASSERT_THAT(result, HasNoError());
  }

  // End child pid_process_2.
  kill(pid_process_2, SIGKILL);
  waitpid(pid_process_2, nullptr, 0);
}

TEST(InstrumentProcessTest, GetErrorMessage) {
  // The function "ReturnImmediately" compiles to something unexpected in gcc. So we only run this
  // test with the release build of clang.
#if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__) || !defined(NDEBUG)
  GTEST_SKIP();
#endif
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */

  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  const pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    [[maybe_unused]] volatile int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  orbit_grpc_protos::CaptureOptions capture_options = BuildCaptureOptions();
  capture_options.set_pid(pid);
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_FALSE(result_or_error.value().instrumented_function_ids.contains(kFunctionId2));
  ASSERT_EQ(result_or_error.value().function_ids_to_error_messages.size(), 1);
  EXPECT_THAT(result_or_error.value().function_ids_to_error_messages[kFunctionId2],
              HasSubstr("Failed to create trampoline: Unable to disassemble enough of the function "
                        "to instrument it. Code: c3"));
  VerifyTrampolineAddressRangesAndLibraryPath(result_or_error.value());
  auto result = instrumentation_manager->UninstrumentProcess(pid);
  ASSERT_THAT(result, HasNoError());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

// Don't #include <complex.h>, it defines the macro I which can break compilation of other headers.
extern "C" long double creall(_Complex long double z);
extern "C" long double cimagl(_Complex long double z);

// Sets st(0) and st(1). Use optnone instead of noinline to prevent constant folding.
extern "C" __attribute__((optnone)) _Complex long double ReturnComplexLongDouble() {
  return {42.0L, 43.0L};
}

// The top two elements of the x87 FPU register stack are used in the System V calling convention to
// return (complex) long double values. We do not back them up in the return trampoline, because we
// can't do it in a way that is correct and also has minimal overhead. But we assume that the
// ExitPayload doesn't change the content. This test verifies it.
TEST(InstrumentProcessTest, ExitPayloadDoesNotUseX87Fpu) {
  /* copybara:insert(b/237251106 injecting the library into the target process triggers some
                     initilization code that check fails.)
  GTEST_SKIP();
  */
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  const pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    while (true) {
      volatile _Complex long double value = ReturnComplexLongDouble();
      ORBIT_CHECK(creall(value) == 42.0L && cimagl(value) == 43.0L);
    }
  }

  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_pid(pid);
  AddFunctionToCaptureOptions(&capture_options, "ReturnComplexLongDouble", kFunctionId1);
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error, HasNoError());
  EXPECT_TRUE(result_or_error.value().instrumented_function_ids.contains(kFunctionId1));
  VerifyTrampolineAddressRangesAndLibraryPath(result_or_error.value());

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // This will fail or hang if the child crashed.
  auto result = instrumentation_manager->UninstrumentProcess(pid);
  ASSERT_THAT(result, HasNoError());

  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

TEST(InstrumentProcessTest, AnyTargetThreadInStrictSeccompMode) {
  InstrumentationManager* instrumentation_manager = GetInstrumentationManager();

  std::array<int, 2> child_to_parent_pipe{};
  ORBIT_CHECK(pipe(child_to_parent_pipe.data()) == 0);

  const pid_t pid = fork();
  ORBIT_CHECK(pid != -1);
  if (pid == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Close the read end of the pipe.
    ORBIT_CHECK(close(child_to_parent_pipe[0]) == 0);

    std::thread t{[&child_to_parent_pipe] {
      // Transition to strict seccomp mode.
      ORBIT_CHECK(syscall(SYS_seccomp, SECCOMP_SET_MODE_STRICT, 0, nullptr) == 0);

      // Send one byte to the parent to notify that the child has called seccomp. Note that the
      // strict seccomp mode still allows write.
      ORBIT_CHECK(write(child_to_parent_pipe[1], "a", 1) == 1);

      [[maybe_unused]] volatile uint64_t counter = 0;
      while (true) {
        ++counter;
      }
    }};

    // Endless loops without side effects are UB and recent versions of clang optimize
    // it away. Making `sum` volatile avoids that problem.
    [[maybe_unused]] volatile int sum = 0;
    while (true) {
      sum += SomethingToInstrument();
    }
  }

  // Close the write end of the pipe.
  ORBIT_CHECK(close(child_to_parent_pipe[1]) == 0);

  // Wait for the child to execute the seccomp syscall.
  char buf[1];
  ORBIT_CHECK(read(child_to_parent_pipe[0], buf, 1) == 1);

  orbit_grpc_protos::CaptureOptions capture_options = BuildCaptureOptions();
  capture_options.set_pid(pid);
  auto result_or_error = instrumentation_manager->InstrumentProcess(capture_options);
  ASSERT_THAT(result_or_error,
              HasError("At least one thread of the target process is in strict seccomp mode."));

  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation