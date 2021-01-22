// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/time/clock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <thread>

#include "LinuxTracing/Tracer.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracingIntegrationTestPuppet.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {
namespace {

[[nodiscard]] bool IsRunningAsRoot() { return geteuid() == 0; }

[[nodiscard]] __attribute__((unused)) bool CheckIsRunningAsRoot() {
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

  void StartTracing() {
    CHECK(!tracer_.has_value());
    CHECK(!listener_.has_value());
    orbit_grpc_protos::CaptureOptions capture_options = BuildCaptureOptions();
    tracer_.emplace(capture_options);
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
  [[nodiscard]] orbit_grpc_protos::CaptureOptions BuildCaptureOptions() {
    orbit_grpc_protos::CaptureOptions capture_options;
    capture_options.set_trace_context_switches(true);
    capture_options.set_pid(puppet_.GetChildPid());
    capture_options.set_sampling_rate(1000.0);
    capture_options.set_unwinding_method(orbit_grpc_protos::CaptureOptions::kDwarf);
    capture_options.set_trace_thread_state(true);
    capture_options.set_trace_gpu_driver(true);
    return capture_options;
  }

 private:
  ChildProcess puppet_;
  std::optional<Tracer> tracer_ = std::nullopt;
  std::optional<BufferTracerListener> listener_ = std::nullopt;
};

using PuppetConstants = LinuxTracingIntegrationTestPuppetConstants;

TEST(LinuxTracingIntegrationTest, ModuleUpdateOnDlopen) {
  if (!CheckIsPerfEventParanoidAtMost(0)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  fixture.StartTracing();
  absl::SleepFor(absl::Milliseconds(100));
  fixture.WriteLineToPuppet(PuppetConstants::kDlopenCommand);
  while (fixture.ReadLineFromPuppet() != PuppetConstants::kDoneResponse) continue;
  absl::SleepFor(absl::Milliseconds(100));
  std::vector<orbit_grpc_protos::CaptureEvent> events = fixture.StopTracingAndGetEvents();

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
