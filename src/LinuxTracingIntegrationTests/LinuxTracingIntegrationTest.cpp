// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "IntegrationTestChildProcess.h"
#include "IntegrationTestCommons.h"
#include "IntegrationTestPuppet.h"
#include "IntegrationTestUtils.h"
#include "LinuxTracing/Tracer.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_linux_tracing_integration_tests {

namespace {

[[nodiscard]] int ReadPerfEventParanoid() {
  auto error_or_content = orbit_base::ReadFileToString("/proc/sys/kernel/perf_event_paranoid");
  ORBIT_CHECK(error_or_content.has_value());
  const std::string& content = error_or_content.value();
  int perf_event_paranoid = 2;
  bool atoi_succeeded = absl::SimpleAtoi(content, &perf_event_paranoid);
  ORBIT_CHECK(atoi_succeeded);
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

  ORBIT_ERROR("Root or max perf_event_paranoid %d (actual is %d) required for this test",
              max_perf_event_paranoid, perf_event_paranoid);
  return false;
}

class BufferTracerListener : public orbit_linux_tracing::TracerListener {
 public:
  void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_scheduling_slice() = std::move(scheduling_slice);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
    {
      absl::MutexLock lock{&one_scheduling_slice_received_mutex_};
      one_scheduling_slice_received_ = true;
    }
  }

  void OnCallstackSample(orbit_grpc_protos::FullCallstackSample callstack_sample) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_full_callstack_sample() = std::move(callstack_sample);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnFunctionCall(orbit_grpc_protos::FunctionCall function_call) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_function_call() = std::move(function_call);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnGpuJob(orbit_grpc_protos::FullGpuJob full_gpu_job_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_full_gpu_job() = std::move(full_gpu_job_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnThreadName(orbit_grpc_protos::ThreadName thread_name) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_thread_name() = std::move(thread_name);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnThreadNamesSnapshot(
      orbit_grpc_protos::ThreadNamesSnapshot thread_names_snapshot) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_thread_names_snapshot() = std::move(thread_names_snapshot);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnThreadStateSlice(orbit_grpc_protos::ThreadStateSlice thread_state_slice) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_thread_state_slice() = std::move(thread_state_slice);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnThreadStateSliceCallstack(
      orbit_grpc_protos::ThreadStateSliceCallstack /*thread_state_slice_callstack*/) override {
    // TODO(b/243515756): Add test for OnThreadStateSliceCallstack
  }

  void OnAddressInfo(orbit_grpc_protos::FullAddressInfo address_info) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_full_address_info() = std::move(address_info);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnTracepointEvent(orbit_grpc_protos::FullTracepointEvent tracepoint_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_full_tracepoint_event() = std::move(tracepoint_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_module_update_event() = std::move(module_update_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnModulesSnapshot(orbit_grpc_protos::ModulesSnapshot modules_snapshot) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_modules_snapshot() = std::move(modules_snapshot);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnErrorsWithPerfEventOpenEvent(
      orbit_grpc_protos::ErrorsWithPerfEventOpenEvent errors_with_perf_event_open_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_errors_with_perf_event_open_event() =
        std::move(errors_with_perf_event_open_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnLostPerfRecordsEvent(
      orbit_grpc_protos::LostPerfRecordsEvent lost_perf_records_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_lost_perf_records_event() = std::move(lost_perf_records_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnOutOfOrderEventsDiscardedEvent(orbit_grpc_protos::OutOfOrderEventsDiscardedEvent
                                            out_of_order_events_discarded_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_out_of_order_events_discarded_event() =
        std::move(out_of_order_events_discarded_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnWarningInstrumentingWithUprobesEvent(
      orbit_grpc_protos::WarningInstrumentingWithUprobesEvent
          warning_instrumenting_with_uprobes_event) override {
    orbit_grpc_protos::ProducerCaptureEvent event;
    *event.mutable_warning_instrumenting_with_uprobes_event() =
        std::move(warning_instrumenting_with_uprobes_event);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  [[nodiscard]] std::vector<orbit_grpc_protos::ProducerCaptureEvent> GetAndClearEvents() {
    absl::MutexLock lock{&events_mutex_};
    std::vector<orbit_grpc_protos::ProducerCaptureEvent> events = std::move(events_);
    events_.clear();
    return events;
  }

  void WaitForAtLeastOneSchedulingSlice() {
    one_scheduling_slice_received_mutex_.LockWhen(absl::Condition(
        +[](bool* one_scheduling_slice_received) { return *one_scheduling_slice_received; },
        &one_scheduling_slice_received_));
    one_scheduling_slice_received_mutex_.Unlock();
  }

 private:
  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events_;
  absl::Mutex events_mutex_;

  bool one_scheduling_slice_received_ = false;
  absl::Mutex one_scheduling_slice_received_mutex_;
};

// GTest's test fixtures seem to mess with the pipe logic in ChildProcess.
// Let's make the fixture ourselves, which is also advised in http://go/totw/122.
class LinuxTracingIntegrationTestFixture {
 public:
  LinuxTracingIntegrationTestFixture() : puppet_{&IntegrationTestPuppetMain} {}

  [[nodiscard]] pid_t GetPuppetPidNative() const { return puppet_.GetChildPidNative(); }
  [[nodiscard]] uint32_t GetPuppetPid() const {
    return orbit_base::FromNativeProcessId(GetPuppetPidNative());
  }

  void WriteLineToPuppet(std::string_view str) { puppet_.WriteLine(str); }

  [[nodiscard]] std::string ReadLineFromPuppet() { return puppet_.ReadLine(); }

  [[nodiscard]] orbit_grpc_protos::CaptureOptions BuildDefaultCaptureOptions() const {
    orbit_grpc_protos::CaptureOptions capture_options;
    capture_options.set_trace_context_switches(true);
    capture_options.set_pid(GetPuppetPid());
    capture_options.set_samples_per_second(1000.0);
    capture_options.set_stack_dump_size(65000);
    capture_options.set_unwinding_method(orbit_grpc_protos::CaptureOptions::kDwarf);
    capture_options.set_dynamic_instrumentation_method(
        orbit_grpc_protos::CaptureOptions::kKernelUprobes);
    capture_options.set_trace_thread_state(true);
    capture_options.set_trace_gpu_driver(true);
    return capture_options;
  }

  void StartTracingAndWaitForTracingLoopStarted(
      const orbit_grpc_protos::CaptureOptions& capture_options) {
    ORBIT_CHECK(tracer_ == nullptr);
    ORBIT_CHECK(!listener_.has_value());

    if (IsRunningAsRoot()) {
      // Needed for BufferTracerListener::WaitForAtLeastOneSchedulingSlice().
      ORBIT_CHECK(capture_options.trace_context_switches());
    }

    listener_.emplace();
    tracer_ = orbit_linux_tracing::Tracer::Create(
        capture_options, /*user_space_instrumentation_addresses=*/nullptr, &listener_.value());
    tracer_->Start();

    if (IsRunningAsRoot()) {
      // Waiting for the first SchedulingSlice (at least one of which is always expected as long as
      // capture_options.trace_context_switches() is true) guarantees that the main loop in
      // TracerThread has started, and hence that the capture has been fully set up.
      listener_->WaitForAtLeastOneSchedulingSlice();
    } else {
      // Some tests verify events that don't require root, but SchedulingSlices do need root.
      // So when running those tests without being root, sleep for a long time instead of waiting
      // for the first SchedulingSlice.
      absl::SleepFor(absl::Milliseconds(2500));
    }
  }

  [[nodiscard]] std::vector<orbit_grpc_protos::ProducerCaptureEvent> StopTracingAndGetEvents() {
    ORBIT_CHECK(tracer_ != nullptr);
    ORBIT_CHECK(listener_.has_value());
    tracer_->Stop();
    tracer_.reset();
    std::vector<orbit_grpc_protos::ProducerCaptureEvent> events = listener_->GetAndClearEvents();
    listener_.reset();
    return events;
  }

 private:
  ChildProcess puppet_;
  std::unique_ptr<orbit_linux_tracing::Tracer> tracer_ = nullptr;
  std::optional<BufferTracerListener> listener_ = std::nullopt;
};

using PuppetConstants = IntegrationTestPuppetConstants;

[[nodiscard]] std::vector<orbit_grpc_protos::ProducerCaptureEvent> TraceAndGetEvents(
    LinuxTracingIntegrationTestFixture* fixture, std::string_view command,
    std::optional<orbit_grpc_protos::CaptureOptions> capture_options = std::nullopt) {
  ORBIT_CHECK(fixture != nullptr);
  if (!capture_options.has_value()) {
    capture_options = fixture->BuildDefaultCaptureOptions();
  }

  fixture->StartTracingAndWaitForTracingLoopStarted(capture_options.value());

  fixture->WriteLineToPuppet(command);
  while (fixture->ReadLineFromPuppet() != PuppetConstants::kDoneResponse) continue;

  constexpr absl::Duration kSleepBeforeStopTracing = absl::Milliseconds(100);
  absl::SleepFor(kSleepBeforeStopTracing);
  return fixture->StopTracingAndGetEvents();
}

void VerifyOrderOfAllEvents(absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
  uint64_t previous_event_timestamp_ns = 0;
  for (const auto& event : events) {
    // Please keep the cases alphabetically ordered, as in the definition of the
    // ProducerCaptureEvent message.
    switch (event.event_case()) {
      case orbit_grpc_protos::ProducerCaptureEvent::kApiScopeStart:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiScopeStartAsync:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiScopeStop:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiScopeStopAsync:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiStringEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiTrackDouble:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiTrackFloat:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiTrackInt:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiTrackInt64:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiTrackUint:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kApiTrackUint64:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kCallstackSample:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kCaptureFinished:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kCaptureStarted:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kClockResolutionEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kErrorEnablingOrbitApiEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kErrorEnablingUserSpaceInstrumentationEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kErrorsWithPerfEventOpenEvent:
        EXPECT_GE(event.errors_with_perf_event_open_event().timestamp_ns(),
                  previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.errors_with_perf_event_open_event().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kFullCallstackSample:
        EXPECT_GE(event.full_callstack_sample().timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.full_callstack_sample().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kFullAddressInfo:
        // AddressInfos have no timestamp.
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kFullGpuJob:
        EXPECT_GE(event.full_gpu_job().dma_fence_signaled_time_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.full_gpu_job().dma_fence_signaled_time_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kFullTracepointEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kFunctionCall:
        EXPECT_GE(event.function_call().end_timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.function_call().end_timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kFunctionEntry:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kFunctionExit:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kGpuQueueSubmission:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kInternedCallstack:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kInternedString:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kLostPerfRecordsEvent:
        EXPECT_GE(event.lost_perf_records_event().end_timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.lost_perf_records_event().end_timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kMemoryUsageEvent:
        // Cases of memory events are tested in MemoryTracingIntegrationTest.
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kModulesSnapshot:
        EXPECT_GE(event.modules_snapshot().timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.modules_snapshot().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kModuleUpdateEvent:
        EXPECT_GE(event.module_update_event().timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.module_update_event().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kPresentEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kOutOfOrderEventsDiscardedEvent:
        EXPECT_GE(event.out_of_order_events_discarded_event().end_timestamp_ns(),
                  previous_event_timestamp_ns);
        previous_event_timestamp_ns =
            event.out_of_order_events_discarded_event().end_timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kSchedulingSlice:
        EXPECT_GE(event.scheduling_slice().out_timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.scheduling_slice().out_timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kThreadName:
        EXPECT_GE(event.thread_name().timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.thread_name().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kThreadNamesSnapshot:
        EXPECT_GE(event.thread_names_snapshot().timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.thread_names_snapshot().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kThreadStateSlice:
        EXPECT_GE(event.thread_state_slice().end_timestamp_ns(), previous_event_timestamp_ns);
        previous_event_timestamp_ns = event.thread_state_slice().end_timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::kThreadStateSliceCallstack:
        // TODO(b/243515756): Add test for scheduling tracepoints with callstacks
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kWarningEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::kWarningInstrumentingWithUprobesEvent:
        EXPECT_GE(event.warning_instrumenting_with_uprobes_event().timestamp_ns(),
                  previous_event_timestamp_ns);
        previous_event_timestamp_ns =
            event.warning_instrumenting_with_uprobes_event().timestamp_ns();
        break;
      case orbit_grpc_protos::ProducerCaptureEvent::
          kWarningInstrumentingWithUserSpaceInstrumentationEvent:
        ORBIT_UNREACHABLE();
      case orbit_grpc_protos::ProducerCaptureEvent::EVENT_NOT_SET:
        ORBIT_UNREACHABLE();
    }
  }
}

void VerifyNoLostOrDiscardedEvents(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
  for (const orbit_grpc_protos::ProducerCaptureEvent& event : events) {
    EXPECT_FALSE(event.has_lost_perf_records_event());
    EXPECT_FALSE(event.has_out_of_order_events_discarded_event());
  }
}

void VerifyErrorsWithPerfEventOpenEvent(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
  bool errors_with_perf_event_open_event_found = false;
  for (const auto& event : events) {
    if (event.has_errors_with_perf_event_open_event()) {
      EXPECT_FALSE(errors_with_perf_event_open_event_found);
      errors_with_perf_event_open_event_found = true;
    }
  }
  EXPECT_EQ(errors_with_perf_event_open_event_found, !IsRunningAsRoot());
}

void VerifyNoWarningInstrumentingWithUprobesEvents(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
  for (const orbit_grpc_protos::ProducerCaptureEvent& event : events) {
    EXPECT_FALSE(event.has_warning_instrumenting_with_uprobes_event());
  }
}

TEST(LinuxTracingIntegrationTest, SchedulingSlices) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kSleepCommand);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  uint64_t scheduling_slice_count = 0;
  uint64_t last_out_timestamp_ns = 0;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kSchedulingSlice) {
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

  ORBIT_LOG("scheduling_slice_count=%lu", scheduling_slice_count);
  // "- 1" as it is the expected number of SchedulingSlices *only between* the first and last sleep.
  EXPECT_GE(scheduling_slice_count, PuppetConstants::kSleepCount - 1);
}

void VerifyFunctionCallsOfOuterAndInnerFunction(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events, uint32_t pid,
    uint64_t outer_function_id, uint64_t inner_function_id) {
  std::vector<orbit_grpc_protos::FunctionCall> function_calls;
  for (const orbit_grpc_protos::ProducerCaptureEvent& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kFunctionCall) {
      continue;
    }
    function_calls.emplace_back(event.function_call());
  }

  VerifyFunctionCallsOfPuppetOuterAndInnerFunction(function_calls, pid, outer_function_id,
                                                   inner_function_id,
                                                   /*expect_return_value_and_registers=*/true);
}

TEST(LinuxTracingIntegrationTest, FunctionCalls) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  orbit_grpc_protos::CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  constexpr uint64_t kOuterFunctionId = 1;
  constexpr uint64_t kInnerFunctionId = 2;
  AddPuppetOuterAndInnerFunctionToCaptureOptions(&capture_options, fixture.GetPuppetPidNative(),
                                                 kOuterFunctionId, kInnerFunctionId);

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  VerifyFunctionCallsOfOuterAndInnerFunction(events, fixture.GetPuppetPid(), kOuterFunctionId,
                                             kInnerFunctionId);
}

std::pair<std::pair<uint64_t, uint64_t>, std::pair<uint64_t, uint64_t>>
GetOuterAndInnerFunctionVirtualAddressRanges(pid_t pid) {
  const orbit_grpc_protos::ModuleInfo& module_info = GetExecutableBinaryModuleInfo(pid);
  const orbit_grpc_protos::ModuleSymbols& module_symbols = GetExecutableBinaryModuleSymbols(pid);

  uint64_t outer_function_virtual_address_start = 0;
  uint64_t outer_function_virtual_address_end = 0;
  uint64_t inner_function_virtual_address_start = 0;
  uint64_t inner_function_virtual_address_end = 0;
  for (const orbit_grpc_protos::SymbolInfo& symbol : module_symbols.symbol_infos()) {
    if (absl::StrContains(symbol.demangled_name(), PuppetConstants::kOuterFunctionName)) {
      ORBIT_CHECK(outer_function_virtual_address_start == 0 &&
                  outer_function_virtual_address_end == 0);
      outer_function_virtual_address_start =
          orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
              symbol.address(), module_info.address_start(), module_info.load_bias(),
              module_info.executable_segment_offset());
      outer_function_virtual_address_end = outer_function_virtual_address_start + symbol.size() - 1;
    }

    if (absl::StrContains(symbol.demangled_name(), PuppetConstants::kInnerFunctionName)) {
      ORBIT_CHECK(inner_function_virtual_address_start == 0 &&
                  inner_function_virtual_address_end == 0);
      inner_function_virtual_address_start =
          orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
              symbol.address(), module_info.address_start(), module_info.load_bias(),
              module_info.executable_segment_offset());
      inner_function_virtual_address_end = inner_function_virtual_address_start + symbol.size() - 1;
    }
  }
  ORBIT_CHECK(outer_function_virtual_address_start != 0);
  ORBIT_CHECK(outer_function_virtual_address_end != 0);
  ORBIT_CHECK(inner_function_virtual_address_start != 0);
  ORBIT_CHECK(inner_function_virtual_address_end != 0);
  return std::make_pair(
      std::make_pair(outer_function_virtual_address_start, outer_function_virtual_address_end),
      std::make_pair(inner_function_virtual_address_start, inner_function_virtual_address_end));
}

absl::flat_hash_set<uint64_t> VerifyAndGetAddressInfosWithOuterAndInnerFunction(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events,
    const std::filesystem::path& executable_path,
    std::pair<uint64_t, uint64_t> outer_function_virtual_address_range,
    std::pair<uint64_t, uint64_t> inner_function_virtual_address_range) {
  absl::flat_hash_set<uint64_t> address_infos_received;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kFullAddressInfo) {
      continue;
    }

    const orbit_grpc_protos::FullAddressInfo& address_info = event.full_address_info();
    if (!(address_info.absolute_address() >= outer_function_virtual_address_range.first &&
          address_info.absolute_address() <= outer_function_virtual_address_range.second) &&
        !(address_info.absolute_address() >= inner_function_virtual_address_range.first &&
          address_info.absolute_address() <= inner_function_virtual_address_range.second)) {
      continue;
    }
    address_infos_received.emplace(address_info.absolute_address());

    if (address_info.absolute_address() >= outer_function_virtual_address_range.first &&
        address_info.absolute_address() <= outer_function_virtual_address_range.second) {
      EXPECT_EQ(address_info.function_name(), PuppetConstants::kOuterFunctionName);
      EXPECT_EQ(address_info.offset_in_function(),
                address_info.absolute_address() - outer_function_virtual_address_range.first);
    } else if (address_info.absolute_address() >= inner_function_virtual_address_range.first &&
               address_info.absolute_address() <= inner_function_virtual_address_range.second) {
      EXPECT_EQ(address_info.function_name(), PuppetConstants::kInnerFunctionName);
      EXPECT_EQ(address_info.offset_in_function(),
                address_info.absolute_address() - inner_function_virtual_address_range.first);
    }

    EXPECT_EQ(address_info.module_name(), executable_path.string());
  }
  return address_infos_received;
}

void VerifyCallstackSamplesWithOuterAndInnerFunction(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events, uint32_t pid,
    std::pair<uint64_t, uint64_t> outer_function_virtual_address_range,
    std::pair<uint64_t, uint64_t> inner_function_virtual_address_range, double sampling_rate,
    const absl::flat_hash_set<uint64_t>* address_infos_received, bool unwound_with_frame_pointers) {
  uint64_t previous_callstack_timestamp_ns = 0;
  size_t matching_callstack_count = 0;
  uint64_t first_matching_callstack_timestamp_ns = std::numeric_limits<uint64_t>::max();
  uint64_t last_matching_callstack_timestamp_ns = 0;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kFullCallstackSample) {
      continue;
    }

    const orbit_grpc_protos::FullCallstackSample& callstack_sample = event.full_callstack_sample();

    // All CallstackSamples should be ordered by timestamp.
    EXPECT_GT(callstack_sample.timestamp_ns(), previous_callstack_timestamp_ns);
    previous_callstack_timestamp_ns = callstack_sample.timestamp_ns();

    // We don't expect other reasons for broken callstacks other than these.
    std::vector<orbit_grpc_protos::Callstack::CallstackType> expected_callstack_types;
    if (unwound_with_frame_pointers) {
      expected_callstack_types = {orbit_grpc_protos::Callstack::kComplete,
                                  orbit_grpc_protos::Callstack::kFramePointerUnwindingError,
                                  orbit_grpc_protos::Callstack::kStackTopForDwarfUnwindingTooSmall,
                                  orbit_grpc_protos::Callstack::kStackTopDwarfUnwindingError,
                                  orbit_grpc_protos::Callstack::kInUprobes};
    } else {
      expected_callstack_types = {orbit_grpc_protos::Callstack::kComplete,
                                  orbit_grpc_protos::Callstack::kDwarfUnwindingError,
                                  orbit_grpc_protos::Callstack::kInUprobes};
    }
    EXPECT_THAT(expected_callstack_types, ::testing::Contains(callstack_sample.callstack().type()));

    // We are only sampling the puppet.
    EXPECT_EQ(callstack_sample.pid(), pid);
    // The puppet is expected single-threaded.
    ASSERT_EQ(callstack_sample.tid(), pid);

    if (callstack_sample.callstack().type() != orbit_grpc_protos::Callstack::kComplete) {
      ORBIT_LOG(
          "callstack_sample.callstack().type() == %s",
          orbit_grpc_protos::Callstack::CallstackType_Name(callstack_sample.callstack().type()));
      continue;
    }

    const orbit_grpc_protos::Callstack& callstack = callstack_sample.callstack();
    for (int32_t pc_index = 0; pc_index < callstack.pcs_size(); ++pc_index) {
      // We found one of the callstacks we are looking for: it contains the "inner" function's
      // address and the caller address should match the "outer" function's address.
      if (callstack.pcs(pc_index) >= inner_function_virtual_address_range.first &&
          callstack.pcs(pc_index) <= inner_function_virtual_address_range.second) {
        if (address_infos_received != nullptr) {
          // Verify that we got the AddressInfo for this virtual address of the "inner" function.
          EXPECT_TRUE(address_infos_received->contains(callstack.pcs(pc_index)));
        }

        // Verify that the caller of the "inner" function is the "outer" function.
        ASSERT_LT(pc_index + 1, callstack.pcs_size());
        ASSERT_TRUE(callstack.pcs(pc_index + 1) >= outer_function_virtual_address_range.first &&
                    callstack.pcs(pc_index + 1) <= outer_function_virtual_address_range.second);

        if (address_infos_received != nullptr) {
          // Verify that we got the AddressInfo for this virtual address of the "outer" function.
          EXPECT_TRUE(address_infos_received->contains(callstack.pcs(pc_index + 1)));
        }

        ++matching_callstack_count;
        first_matching_callstack_timestamp_ns =
            std::min(first_matching_callstack_timestamp_ns, callstack_sample.timestamp_ns());
        last_matching_callstack_timestamp_ns =
            std::max(last_matching_callstack_timestamp_ns, callstack_sample.timestamp_ns());
        break;
      }
    }
  }

  ASSERT_GT(matching_callstack_count, 0);
  ORBIT_CHECK(first_matching_callstack_timestamp_ns <= last_matching_callstack_timestamp_ns);
  ORBIT_LOG("Found %lu of the expected callstacks over %.0f ms", matching_callstack_count,
            (last_matching_callstack_timestamp_ns - first_matching_callstack_timestamp_ns) / 1e6);
  constexpr double kMinExpectedScheduledRelativeTime = 0.67;
  const auto min_expected_matching_callstack_count = static_cast<uint64_t>(
      std::floor((last_matching_callstack_timestamp_ns - first_matching_callstack_timestamp_ns) /
                 1e9 * sampling_rate * kMinExpectedScheduledRelativeTime));
  EXPECT_GE(matching_callstack_count, min_expected_matching_callstack_count);
}

void VerifyCallstackSamplesWithOuterAndInnerFunctionForDwarfUnwinding(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events, uint32_t pid,
    std::pair<uint64_t, uint64_t> outer_function_virtual_address_range,
    std::pair<uint64_t, uint64_t> inner_function_virtual_address_range, double sampling_rate,
    const absl::flat_hash_set<uint64_t>* address_infos_received) {
  VerifyCallstackSamplesWithOuterAndInnerFunction(
      events, pid, outer_function_virtual_address_range, inner_function_virtual_address_range,
      sampling_rate, address_infos_received, /*unwound_with_frame_pointers=*/false);
}

void VerifyCallstackSamplesWithOuterAndInnerFunctionForFramePointerUnwinding(
    absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events, uint32_t pid,
    std::pair<uint64_t, uint64_t> outer_function_virtual_address_range,
    std::pair<uint64_t, uint64_t> inner_function_virtual_address_range, double sampling_rate,
    const absl::flat_hash_set<uint64_t>* address_infos_received) {
  VerifyCallstackSamplesWithOuterAndInnerFunction(
      events, pid, outer_function_virtual_address_range, inner_function_virtual_address_range,
      sampling_rate, address_infos_received, /*unwound_with_frame_pointers=*/true);
}

TEST(LinuxTracingIntegrationTest, CallstackSamplesAndAddressInfos) {
  if (!CheckIsPerfEventParanoidAtMost(0)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  const auto& [outer_function_virtual_address_range, inner_function_virtual_address_range] =
      GetOuterAndInnerFunctionVirtualAddressRanges(fixture.GetPuppetPidNative());
  const std::filesystem::path& executable_path =
      GetExecutableBinaryPath(fixture.GetPuppetPidNative());

  orbit_grpc_protos::CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  const double samples_per_second = capture_options.samples_per_second();

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyErrorsWithPerfEventOpenEvent(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  absl::flat_hash_set<uint64_t> address_infos_received =
      VerifyAndGetAddressInfosWithOuterAndInnerFunction(events, executable_path,
                                                        outer_function_virtual_address_range,
                                                        inner_function_virtual_address_range);

  VerifyCallstackSamplesWithOuterAndInnerFunctionForDwarfUnwinding(
      events, fixture.GetPuppetPid(), outer_function_virtual_address_range,
      inner_function_virtual_address_range, samples_per_second, &address_infos_received);
}

TEST(LinuxTracingIntegrationTest, CallstackSamplesTogetherWithFunctionCalls) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  const auto& [outer_function_virtual_address_range, inner_function_virtual_address_range] =
      GetOuterAndInnerFunctionVirtualAddressRanges(fixture.GetPuppetPidNative());
  const std::filesystem::path& executable_path =
      GetExecutableBinaryPath(fixture.GetPuppetPidNative());

  orbit_grpc_protos::CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  constexpr uint64_t kOuterFunctionId = 1;
  constexpr uint64_t kInnerFunctionId = 2;
  AddPuppetOuterAndInnerFunctionToCaptureOptions(&capture_options, fixture.GetPuppetPidNative(),
                                                 kOuterFunctionId, kInnerFunctionId);
  const double sampling_rate = capture_options.samples_per_second();

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  VerifyFunctionCallsOfOuterAndInnerFunction(events, fixture.GetPuppetPid(), kOuterFunctionId,
                                             kInnerFunctionId);

  absl::flat_hash_set<uint64_t> address_infos_received =
      VerifyAndGetAddressInfosWithOuterAndInnerFunction(events, executable_path,
                                                        outer_function_virtual_address_range,
                                                        inner_function_virtual_address_range);

  VerifyCallstackSamplesWithOuterAndInnerFunctionForDwarfUnwinding(
      events, fixture.GetPuppetPid(), outer_function_virtual_address_range,
      inner_function_virtual_address_range, sampling_rate, &address_infos_received);
}

void VerifyNoAddressInfos(absl::Span<const orbit_grpc_protos::ProducerCaptureEvent> events) {
  for (const auto& event : events) {
    EXPECT_NE(event.event_case(), orbit_grpc_protos::ProducerCaptureEvent::kFullAddressInfo);
  }
}

TEST(LinuxTracingIntegrationTest, CallstackSamplesWithFramePointers) {
  if (!CheckIsPerfEventParanoidAtMost(0)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  const auto& [outer_function_virtual_address_range, inner_function_virtual_address_range] =
      GetOuterAndInnerFunctionVirtualAddressRanges(fixture.GetPuppetPidNative());

  orbit_grpc_protos::CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  capture_options.set_unwinding_method(orbit_grpc_protos::CaptureOptions::kFramePointers);
  capture_options.set_stack_dump_size(512);
  const double sampling_rate = capture_options.samples_per_second();

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyErrorsWithPerfEventOpenEvent(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  // AddressInfos are not sent when unwinding with frame pointers as they are produced by
  // libunwindstack.
  VerifyNoAddressInfos(events);

  // Note that this test requires that the "inner" function of the puppet use frame pointers.
  VerifyCallstackSamplesWithOuterAndInnerFunctionForFramePointerUnwinding(
      events, fixture.GetPuppetPid(), outer_function_virtual_address_range,
      inner_function_virtual_address_range, sampling_rate, /*address_infos_received=*/nullptr);
}

TEST(LinuxTracingIntegrationTest, CallstackSamplesWithFramePointersTogetherWithFunctionCalls) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  const auto& [outer_function_virtual_address_range, inner_function_virtual_address_range] =
      GetOuterAndInnerFunctionVirtualAddressRanges(fixture.GetPuppetPidNative());

  orbit_grpc_protos::CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  capture_options.set_unwinding_method(orbit_grpc_protos::CaptureOptions::kFramePointers);
  constexpr uint64_t kOuterFunctionId = 1;
  constexpr uint64_t kInnerFunctionId = 2;
  AddPuppetOuterAndInnerFunctionToCaptureOptions(&capture_options, fixture.GetPuppetPidNative(),
                                                 kOuterFunctionId, kInnerFunctionId);
  const double sampling_rate = capture_options.samples_per_second();

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  VerifyFunctionCallsOfOuterAndInnerFunction(events, fixture.GetPuppetPid(), kOuterFunctionId,
                                             kInnerFunctionId);

  VerifyNoAddressInfos(events);

  VerifyCallstackSamplesWithOuterAndInnerFunctionForFramePointerUnwinding(
      events, fixture.GetPuppetPid(), outer_function_virtual_address_range,
      inner_function_virtual_address_range, sampling_rate, /*address_infos_received=*/nullptr);
}

TEST(LinuxTracingIntegrationTest, ThreadStateSlices) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kSleepCommand);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  uint64_t running_slice_count = 0;
  uint64_t runnable_slice_count = 0;
  uint64_t interruptible_sleep_slice_count = 0;
  uint64_t last_end_timestamp_ns = 0;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kThreadStateSlice) {
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

  ORBIT_LOG("running_slice_count=%lu", running_slice_count);
  ORBIT_LOG("runnable_slice_count=%lu", runnable_slice_count);
  ORBIT_LOG("interruptible_sleep_slice_count=%lu", interruptible_sleep_slice_count);
  // "- 1" as these are the expected numbers of kRunning and kRunnable ThreadStateSlices *only
  // between* the first and last sleep.
  EXPECT_GE(running_slice_count, PuppetConstants::kSleepCount - 1);
  EXPECT_GE(runnable_slice_count, PuppetConstants::kSleepCount - 1);
  EXPECT_GE(interruptible_sleep_slice_count, PuppetConstants::kSleepCount);
}

TEST(LinuxTracingIntegrationTest, ThreadNames) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  // We also collect the initial name of each thread of the target at the start of the capture:
  // save the actual initial name so that we can later verify that it was received.
  std::string initial_puppet_name = orbit_base::GetThreadNameNative(fixture.GetPuppetPidNative());

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kPthreadSetnameNpCommand);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  std::vector<std::string> changed_thread_names;
  std::vector<std::string> initial_thread_names;
  for (const auto& event : events) {
    if (event.event_case() == orbit_grpc_protos::ProducerCaptureEvent::kThreadNamesSnapshot) {
      const orbit_grpc_protos::ThreadNamesSnapshot& thread_names_snapshot =
          event.thread_names_snapshot();
      for (const auto& thread_name : thread_names_snapshot.thread_names()) {
        if (thread_name.pid() != fixture.GetPuppetPid()) {
          continue;
        }

        // There is only one thread and it is the main thread.
        EXPECT_EQ(thread_name.tid(), fixture.GetPuppetPid());

        initial_thread_names.emplace_back(thread_name.name());
      }
    }
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kThreadName) {
      continue;
    }

    const orbit_grpc_protos::ThreadName& thread_name = event.thread_name();
    if (thread_name.pid() != fixture.GetPuppetPid()) {
      continue;
    }

    // There is only one thread and it is the main thread.
    EXPECT_EQ(thread_name.tid(), fixture.GetPuppetPid());

    changed_thread_names.emplace_back(thread_name.name());
  }

  EXPECT_THAT(initial_thread_names, ::testing::ElementsAre(initial_puppet_name));

  EXPECT_THAT(changed_thread_names, ::testing::ElementsAre(PuppetConstants::kNewThreadName));
}

TEST(LinuxTracingIntegrationTest, ModuleUpdateOnDlopen) {
  if (!CheckIsPerfEventParanoidAtMost(0)) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kDlopenCommand);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyErrorsWithPerfEventOpenEvent(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  bool module_update_found = false;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kModuleUpdateEvent) {
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

TEST(LinuxTracingIntegrationTest, GpuJobs) {
  if (!CheckIsStadiaInstance()) {
    GTEST_SKIP();
  }
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }
  LinuxTracingIntegrationTestFixture fixture;

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> events =
      TraceAndGetEvents(&fixture, PuppetConstants::kVulkanTutorialCommand);

  VerifyOrderOfAllEvents(events);

  VerifyNoLostOrDiscardedEvents(events);

  VerifyNoWarningInstrumentingWithUprobesEvents(events);

  bool another_process_used_gpu = false;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kFullGpuJob) {
      continue;
    }

    const orbit_grpc_protos::FullGpuJob& gpu_job = event.full_gpu_job();
    if (gpu_job.pid() != fixture.GetPuppetPid()) {
      another_process_used_gpu = true;
      break;
    }
  }
  ORBIT_LOG("another_process_used_gpu=%d", another_process_used_gpu);

  uint64_t gpu_job_count = 0;
  for (const auto& event : events) {
    if (event.event_case() != orbit_grpc_protos::ProducerCaptureEvent::kFullGpuJob) {
      continue;
    }

    const orbit_grpc_protos::FullGpuJob& gpu_job = event.full_gpu_job();
    if (gpu_job.pid() != fixture.GetPuppetPid()) {
      continue;
    }

    // The puppet is single-threaded.
    EXPECT_EQ(gpu_job.tid(), fixture.GetPuppetPid());

    if (!another_process_used_gpu) {
      EXPECT_EQ(gpu_job.depth(), 0);
    }

    EXPECT_LT(gpu_job.amdgpu_cs_ioctl_time_ns(), gpu_job.amdgpu_sched_run_job_time_ns());
    // If no other job is running on the GPU (which is the case if the puppet is the only process
    // using the GPU), then we assume (it's the best we can do) that the job starts running on the
    // hardware at the same time as it is scheduled by the driver, hence the EXPECT_EQ.
    // Otherwise, use EXPECT_LE.
    if (another_process_used_gpu) {
      EXPECT_LE(gpu_job.amdgpu_sched_run_job_time_ns(), gpu_job.gpu_hardware_start_time_ns());
    } else {
      EXPECT_EQ(gpu_job.amdgpu_sched_run_job_time_ns(), gpu_job.gpu_hardware_start_time_ns());
    }
    EXPECT_LT(gpu_job.gpu_hardware_start_time_ns(), gpu_job.dma_fence_signaled_time_ns());

    EXPECT_EQ(gpu_job.timeline(), "gfx");

    ++gpu_job_count;
  }

  ORBIT_LOG("gpu_job_count=%lu", gpu_job_count);
  EXPECT_GE(gpu_job_count, PuppetConstants::kFrameCount);
}

}  // namespace
}  // namespace orbit_linux_tracing_integration_tests
