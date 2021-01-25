// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/time/clock.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <thread>

#include "ElfUtils/ElfFile.h"
#include "LinuxTracing/Tracer.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracingIntegrationTestPuppet.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {
namespace {

[[nodiscard]] bool IsRunningAsRoot() { return geteuid() == 0; }

[[nodiscard]] bool CheckIsRunningAsRoot() {
  if (IsRunningAsRoot()) {
    return true;
  }

  ERROR("Root required for this test");
  return false;
}

[[nodiscard]] int ReadPerfEventParanoid() {
  auto error_or_content = orbit_base::ReadFileToString("/proc/sys/kernel/perf_event_paranoid");
  CHECK(error_or_content.has_value());
  const std::string& content = error_or_content.value();
  int perf_event_paranoid = 2;
  bool atoi_succeeded = absl::SimpleAtoi(content, &perf_event_paranoid);
  CHECK(atoi_succeeded);
  return perf_event_paranoid;
}

[[nodiscard]] bool CheckIsPerfEventParanoidAtMost(int max_perf_event_paranoid) {
  if (IsRunningAsRoot()) {
    return true;
  }

  int perf_event_paranoid = ReadPerfEventParanoid();
  if (perf_event_paranoid <= max_perf_event_paranoid) {
    return true;
  }

  ERROR("Root or max perf_event_paranoid %d (actual is %d) required for this test",
        max_perf_event_paranoid, perf_event_paranoid);
  return false;
}

class ChildProcess {
 public:
  explicit ChildProcess(const std::function<int()>& child_main) {
    std::array<int, 2> parent_to_child_pipe{};
    CHECK(pipe(parent_to_child_pipe.data()) == 0);

    std::array<int, 2> child_to_parent_pipe{};
    CHECK(pipe(child_to_parent_pipe.data()) == 0);

    pid_t child_pid = fork();
    CHECK(child_pid >= 0);
    if (child_pid > 0) {
      // Parent.
      child_pid_ = child_pid;

      // Close unused ends of the pipes.
      CHECK(close(parent_to_child_pipe[0]) == 0);
      CHECK(close(child_to_parent_pipe[1]) == 0);

      reading_fd_ = child_to_parent_pipe[0];
      writing_fd_ = parent_to_child_pipe[1];

    } else {
      // Child.

      // Close unused ends of the pipes.
      CHECK(close(parent_to_child_pipe[1]) == 0);
      CHECK(close(child_to_parent_pipe[0]) == 0);

      // Redirect reading end of parent_to_child_pipe to stdin and close the pipe's original fd.
      CHECK(close(STDIN_FILENO) == 0);
      CHECK(dup2(parent_to_child_pipe[0], STDIN_FILENO) == STDIN_FILENO);
      CHECK(close(parent_to_child_pipe[0]) == 0);

      // Redirect writing end of child_to_parent_pipe to stdout and close the pipe's original fd.
      CHECK(close(STDOUT_FILENO) == 0);
      CHECK(dup2(child_to_parent_pipe[1], STDOUT_FILENO) == STDOUT_FILENO);
      CHECK(close(child_to_parent_pipe[1]) == 0);

      // Run the child and exit.
      exit(child_main());
    }
  }

  ~ChildProcess() {
    CHECK(close(reading_fd_) == 0);
    CHECK(close(writing_fd_) == 0);

    CHECK(waitpid(child_pid_, nullptr, 0) == child_pid_);
  }

  [[nodiscard]] pid_t GetChildPid() const { return child_pid_; }

  void WriteLine(std::string_view str) {
    std::string string_with_newline = std::string{str}.append("\n");
    CHECK(write(writing_fd_, string_with_newline.c_str(), string_with_newline.length()) ==
          static_cast<ssize_t>(string_with_newline.length()));
  }

  [[nodiscard]] std::string ReadLine() {
    std::string str;
    while (true) {
      char c;
      CHECK(read(reading_fd_, &c, sizeof(c)) == sizeof(c));
      if (c == '\n' || c == '\0') break;
      str.push_back(c);
    }
    return str;
  }

 private:
  pid_t child_pid_ = -1;
  int reading_fd_ = -1;
  int writing_fd_ = -1;
};

class BufferTracerListener : public TracerListener {
 public:
  void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_scheduling_slice() = std::move(scheduling_slice);
    events_.emplace_back(std::move(event));
  }

  void OnCallstackSample(orbit_grpc_protos::CallstackSample callstack_sample) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_callstack_sample() = std::move(callstack_sample);
    events_.emplace_back(std::move(event));
  }

  void OnFunctionCall(orbit_grpc_protos::FunctionCall function_call) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_function_call() = std::move(function_call);
    events_.emplace_back(std::move(event));
  }

  void OnIntrospectionScope(orbit_grpc_protos::IntrospectionScope introspection_scope) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_introspection_scope() = std::move(introspection_scope);
    events_.emplace_back(std::move(event));
  }

  void OnGpuJob(orbit_grpc_protos::GpuJob gpu_job) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_gpu_job() = std::move(gpu_job);
    events_.emplace_back(std::move(event));
  }

  void OnThreadName(orbit_grpc_protos::ThreadName thread_name) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_thread_name() = std::move(thread_name);
    events_.emplace_back(std::move(event));
  }

  void OnThreadStateSlice(orbit_grpc_protos::ThreadStateSlice thread_state_slice) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_thread_state_slice() = std::move(thread_state_slice);
    events_.emplace_back(std::move(event));
  }

  void OnAddressInfo(orbit_grpc_protos::AddressInfo address_info) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_address_info() = std::move(address_info);
    events_.emplace_back(std::move(event));
  }

  void OnTracepointEvent(orbit_grpc_protos::TracepointEvent tracepoint_event) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_tracepoint_event() = std::move(tracepoint_event);
    events_.emplace_back(std::move(event));
  }

  void OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) override {
    orbit_grpc_protos::CaptureEvent event;
    *event.mutable_module_update_event() = std::move(module_update_event);
    events_.emplace_back(std::move(event));
  }

  [[nodiscard]] const std::vector<orbit_grpc_protos::CaptureEvent>& GetEvents() const {
    return events_;
  }

  [[nodiscard]] std::vector<orbit_grpc_protos::CaptureEvent>&& GetEvents() {
    return std::move(events_);
  }

 private:
  std::vector<orbit_grpc_protos::CaptureEvent> events_;
};

// GTest's test fixtures seem to mess with the pipe logic in ChildProcess.
// Let's make the fixture ourselves, which is also advised in http://go/totw/122.
class LinuxTracingIntegrationTestFixture {
 public:
  LinuxTracingIntegrationTestFixture() : puppet_{&LinuxTracingIntegrationTestPuppetMain} {}

  [[nodiscard]] pid_t GetPuppetPid() const { return puppet_.GetChildPid(); }

  void WriteLineToPuppet(std::string_view str) { puppet_.WriteLine(str); }

  [[nodiscard]] std::string ReadLineFromPuppet() { return puppet_.ReadLine(); }

  [[nodiscard]] orbit_grpc_protos::CaptureOptions BuildDefaultCaptureOptions() {
    orbit_grpc_protos::CaptureOptions capture_options;
    capture_options.set_trace_context_switches(true);
    capture_options.set_pid(puppet_.GetChildPid());
    capture_options.set_sampling_rate(1000.0);
    capture_options.set_unwinding_method(orbit_grpc_protos::CaptureOptions::kDwarf);
    capture_options.set_trace_thread_state(true);
    capture_options.set_trace_gpu_driver(true);
    return capture_options;
  }

  void StartTracing(orbit_grpc_protos::CaptureOptions capture_options) {
    CHECK(!tracer_.has_value());
    CHECK(!listener_.has_value());
    tracer_.emplace(std::move(capture_options));
    listener_.emplace();
    tracer_->SetListener(&*listener_);
    tracer_->Start();
  }

  [[nodiscard]] std::vector<orbit_grpc_protos::CaptureEvent> StopTracingAndGetEvents() {
    CHECK(tracer_.has_value());
    CHECK(listener_.has_value());
    tracer_->Stop();
    tracer_.reset();
    std::vector<orbit_grpc_protos::CaptureEvent> events = std::move(listener_->GetEvents());
    listener_.reset();
    return events;
  }

 private:
  ChildProcess puppet_;
  std::optional<Tracer> tracer_ = std::nullopt;
  std::optional<BufferTracerListener> listener_ = std::nullopt;
};

using PuppetConstants = LinuxTracingIntegrationTestPuppetConstants;

[[nodiscard]] std::vector<orbit_grpc_protos::CaptureEvent> TraceAndGetEvents(
    LinuxTracingIntegrationTestFixture* fixture, std::string_view command,
    std::optional<orbit_grpc_protos::CaptureOptions> capture_options = std::nullopt) {
  CHECK(fixture != nullptr);
  if (!capture_options.has_value()) {
    capture_options = fixture->BuildDefaultCaptureOptions();
  }

  fixture->StartTracing(capture_options.value());
  constexpr absl::Duration kSleepAfterStartTracing = absl::Milliseconds(100);
  absl::SleepFor(kSleepAfterStartTracing);

  fixture->WriteLineToPuppet(command);
  while (fixture->ReadLineFromPuppet() != PuppetConstants::kDoneResponse) continue;

  constexpr absl::Duration kSleepBeforeStopTracing = absl::Milliseconds(100);
  absl::SleepFor(kSleepBeforeStopTracing);
  return fixture->StopTracingAndGetEvents();
}

TEST(LinuxTracingIntegrationTest, SchedulingSlices) {
  if (!CheckIsPerfEventParanoidAtMost(-1)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::CaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kSleepCommand);

  uint64_t scheduling_slice_count = 0;
  uint64_t last_out_timestamp_ns = 0;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::CaptureEvent::kSchedulingSlice) {
      continue;
    }

    const orbit_grpc_protos::SchedulingSlice& scheduling_slice = event.scheduling_slice();
    if (scheduling_slice.pid() != fixture.GetPuppetPid()) {
      continue;
    }

    ++scheduling_slice_count;

    // The puppet is not expected to spawn new threads.
    EXPECT_EQ(scheduling_slice.tid(), scheduling_slice.pid());

    EXPECT_GT(scheduling_slice.duration_ns(), 0);
    // SchedulingSlices are expected to be in order of out_timestamp_ns across all CPUs.
    EXPECT_GT(scheduling_slice.out_timestamp_ns(), last_out_timestamp_ns);
    last_out_timestamp_ns = scheduling_slice.out_timestamp_ns();
  }

  LOG("scheduling_slice_count=%lu", scheduling_slice_count);
  EXPECT_GE(scheduling_slice_count, PuppetConstants::kSleepCount);
}

TEST(LinuxTracingIntegrationTest, FunctionCalls) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  orbit_grpc_protos::CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();

  // Find the offset in the ELF file of the functions to instrument and add those functions to the
  // CaptureOptions.
  auto error_or_executable_path = orbit_base::GetExecutablePath(fixture.GetPuppetPid());
  CHECK(error_or_executable_path.has_value());
  const std::filesystem::path& executable_path = error_or_executable_path.value();

  auto error_or_elf_file = orbit_elf_utils::ElfFile::Create(executable_path.string());
  CHECK(error_or_elf_file.has_value());
  const std::unique_ptr<orbit_elf_utils::ElfFile>& elf_file = error_or_elf_file.value();

  auto error_or_module = elf_file->LoadSymbols();
  CHECK(error_or_module.has_value());
  orbit_grpc_protos::ModuleSymbols module = error_or_module.value();

  bool outer_function_symbol_found = false;
  bool inner_function_symbol_found = false;
  constexpr uint64_t kOuterFunctionId = 1;
  constexpr uint64_t kInnerFunctionId = 2;
  for (const orbit_grpc_protos::SymbolInfo& symbol : module.symbol_infos()) {
    if (symbol.name() == PuppetConstants::kOuterFunctionName) {
      CHECK(!outer_function_symbol_found);
      outer_function_symbol_found = true;
      orbit_grpc_protos::CaptureOptions::InstrumentedFunction instrumented_function;
      instrumented_function.set_file_path(executable_path);
      instrumented_function.set_file_offset(symbol.address() - module.load_bias());
      instrumented_function.set_function_id(kOuterFunctionId);
      capture_options.mutable_instrumented_functions()->Add(std::move(instrumented_function));
    }

    if (symbol.name() == PuppetConstants::kInnerFunctionName) {
      CHECK(!inner_function_symbol_found);
      inner_function_symbol_found = true;
      orbit_grpc_protos::CaptureOptions::InstrumentedFunction instrumented_function;
      instrumented_function.set_file_path(executable_path);
      instrumented_function.set_file_offset(symbol.address() - module.load_bias());
      instrumented_function.set_function_id(kInnerFunctionId);
      capture_options.mutable_instrumented_functions()->Add(std::move(instrumented_function));
    }
  }
  CHECK(outer_function_symbol_found);
  CHECK(inner_function_symbol_found);

  std::vector<orbit_grpc_protos::CaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kCallOuterFunctionCommand, capture_options);

  std::vector<orbit_grpc_protos::FunctionCall> function_calls;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::CaptureEvent::kFunctionCall) {
      continue;
    }

    const orbit_grpc_protos::FunctionCall& function_call = event.function_call();
    ASSERT_EQ(function_call.pid(), fixture.GetPuppetPid());
    ASSERT_EQ(function_call.tid(), fixture.GetPuppetPid());
    function_calls.emplace_back(function_call);
  }

  // We expect an ordered sequence of kInnerFunctionCallCount calls to the "inner" function followed
  // by one call to the "outer" function, repeated kOuterFunctionCallCount times.
  ASSERT_EQ(function_calls.size(), PuppetConstants::kOuterFunctionCallCount +
                                       PuppetConstants::kOuterFunctionCallCount *
                                           PuppetConstants::kInnerFunctionCallCount);
  size_t function_call_index = 0;
  for (size_t outer_index = 0; outer_index < PuppetConstants::kOuterFunctionCallCount;
       ++outer_index) {
    uint64_t inner_calls_duration_ns_sum = 0;
    for (size_t inner_index = 0; inner_index < PuppetConstants::kInnerFunctionCallCount;
         ++inner_index) {
      const orbit_grpc_protos::FunctionCall& function_call = function_calls[function_call_index];
      EXPECT_EQ(function_call.function_id(), kInnerFunctionId);
      EXPECT_GT(function_call.duration_ns(), 0);
      inner_calls_duration_ns_sum += function_call.duration_ns();
      if (function_call_index > 0) {
        EXPECT_GT(function_call.end_timestamp_ns(),
                  function_calls[function_call_index - 1].end_timestamp_ns());
      }
      EXPECT_EQ(function_call.depth(), 1);
      ++function_call_index;
    }

    {
      const orbit_grpc_protos::FunctionCall& function_call = function_calls[function_call_index];
      EXPECT_EQ(function_call.function_id(), kOuterFunctionId);
      EXPECT_GT(function_call.duration_ns(), inner_calls_duration_ns_sum);
      if (function_call_index > 0) {
        EXPECT_GT(function_call.end_timestamp_ns(),
                  function_calls[function_call_index - 1].end_timestamp_ns());
      }
      EXPECT_EQ(function_call.depth(), 0);
      ++function_call_index;
    }
  }
}

TEST(LinuxTracingIntegrationTest, ThreadStateSlices) {
  if (!CheckIsPerfEventParanoidAtMost(-1)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::CaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kSleepCommand);

  uint64_t running_slice_count = 0;
  uint64_t runnable_slice_count = 0;
  uint64_t interruptible_sleep_slice_count = 0;
  uint64_t last_end_timestamp_ns = 0;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::CaptureEvent::kThreadStateSlice) {
      continue;
    }

    const orbit_grpc_protos::ThreadStateSlice& thread_state_slice = event.thread_state_slice();
    if (thread_state_slice.tid() != fixture.GetPuppetPid()) {
      continue;
    }

    // We currently don't set the pid.
    EXPECT_EQ(thread_state_slice.pid(), 0);

    EXPECT_TRUE(
        thread_state_slice.thread_state() == orbit_grpc_protos::ThreadStateSlice::kRunning ||
        thread_state_slice.thread_state() == orbit_grpc_protos::ThreadStateSlice::kRunnable ||
        thread_state_slice.thread_state() ==
            orbit_grpc_protos::ThreadStateSlice::kInterruptibleSleep);
    switch (thread_state_slice.thread_state()) {
      case orbit_grpc_protos::ThreadStateSlice_ThreadState_kRunning:
        ++running_slice_count;
        break;
      case orbit_grpc_protos::ThreadStateSlice_ThreadState_kRunnable:
        ++runnable_slice_count;
        break;
      case orbit_grpc_protos::ThreadStateSlice_ThreadState_kInterruptibleSleep:
        ++interruptible_sleep_slice_count;
        break;
      default:
        break;
    }

    EXPECT_GT(thread_state_slice.duration_ns(), 0);
    EXPECT_GT(thread_state_slice.end_timestamp_ns(), last_end_timestamp_ns);
    last_end_timestamp_ns = thread_state_slice.end_timestamp_ns();
  }

  LOG("running_slice_count=%lu", running_slice_count);
  LOG("runnable_slice_count=%lu", runnable_slice_count);
  LOG("interruptible_sleep_slice_count=%lu", interruptible_sleep_slice_count);
  EXPECT_GE(running_slice_count, PuppetConstants::kSleepCount);
  EXPECT_GE(runnable_slice_count, PuppetConstants::kSleepCount);
  EXPECT_GE(interruptible_sleep_slice_count, PuppetConstants::kSleepCount);
}

TEST(LinuxTracingIntegrationTest, ThreadNames) {
  if (!CheckIsPerfEventParanoidAtMost(-1)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  // We also collect the initial name of each thread of the target at the start of the capture:
  // save the actual initial name so that we can later verify that it was received.
  std::string initial_puppet_name = orbit_base::GetThreadName(fixture.GetPuppetPid());

  std::vector<orbit_grpc_protos::CaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kPthreadSetnameNpCommand);

  std::vector<std::string> collected_event_names;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::CaptureEvent::kThreadName) {
      continue;
    }

    const orbit_grpc_protos::ThreadName& thread_name = event.thread_name();
    if (thread_name.tid() != fixture.GetPuppetPid()) {
      continue;
    }

    // We currently don't set the pid.
    EXPECT_EQ(thread_name.pid(), 0);

    collected_event_names.emplace_back(thread_name.name());
  }

  EXPECT_THAT(collected_event_names,
              ::testing::ElementsAre(initial_puppet_name, PuppetConstants::kNewThreadName));
}

TEST(LinuxTracingIntegrationTest, ModuleUpdateOnDlopen) {
  if (!CheckIsPerfEventParanoidAtMost(0)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::CaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kDlopenCommand);

  bool module_update_found = false;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::CaptureEvent::kModuleUpdateEvent) {
      continue;
    }

    const orbit_grpc_protos::ModuleUpdateEvent& module_update = event.module_update_event();
    if (module_update.pid() != fixture.GetPuppetPid()) {
      continue;
    }

    const orbit_grpc_protos::ModuleInfo& module_info = module_update.module();
    if (module_info.name() != PuppetConstants::kSharedObjectFileName) {
      continue;
    }

    EXPECT_FALSE(module_update_found);
    module_update_found = true;
  }

  EXPECT_TRUE(module_update_found);
}

}  // namespace
}  // namespace orbit_linux_tracing
