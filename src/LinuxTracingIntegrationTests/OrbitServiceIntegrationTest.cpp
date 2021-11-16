// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "IntegrationTestChildProcess.h"
#include "IntegrationTestCommons.h"
#include "IntegrationTestPuppet.h"
#include "IntegrationTestUtils.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitService.h"
#include "OrbitVersion/OrbitVersion.h"
#include "capture.pb.h"
#include "services.grpc.pb.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_linux_tracing_integration_tests {

namespace {

// Note that the tests will behave unexpectedly if another instance of OrbitService is running on
// the system.
constexpr uint16_t kOrbitServicePort = 44765;

int OrbitServiceMain() {
  LOG("OrbitService started");
  std::atomic<bool> exit_requested = false;
  // OrbitService's loop terminates when EOF is received, no need to manipulate exit_requested.
  orbit_service::OrbitService{kOrbitServicePort, /*dev_mode=*/false}.Run(&exit_requested);
  LOG("OrbitService finished");
  return 0;
}

class OrbitServiceIntegrationTestFixture {
 public:
  OrbitServiceIntegrationTestFixture()
      : puppet_{&IntegrationTestPuppetMain}, orbit_service_{&OrbitServiceMain} {
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
  }

  [[nodiscard]] pid_t GetPuppetPidNative() const { return puppet_.GetChildPidNative(); }
  [[nodiscard]] uint32_t GetPuppetPid() const {
    return orbit_base::FromNativeProcessId(GetPuppetPidNative());
  }

  [[nodiscard]] CaptureOptions BuildDefaultCaptureOptions() const {
    CaptureOptions capture_options;
    capture_options.set_trace_context_switches(true);
    capture_options.set_pid(GetPuppetPid());
    capture_options.set_samples_per_second(1000.0);
    capture_options.set_stack_dump_size(65000);
    capture_options.set_unwinding_method(CaptureOptions::kDwarf);
    capture_options.set_dynamic_instrumentation_method(CaptureOptions::kKernelUprobes);
    capture_options.set_trace_thread_state(true);
    capture_options.set_trace_gpu_driver(true);
    return capture_options;
  }

  [[nodiscard]] std::vector<ClientCaptureEvent> CaptureAndGetEvents(
      std::string_view command_for_puppet, CaptureOptions capture_options) {
    StartCapture(std::move(capture_options));
    WaitForFirstEvent();

    // We don't have a signal from OrbitService that all internal producers have started, and we
    // can't have one for external producers, so let's sleep after CaptureStarted has been received.
    constexpr std::chrono::milliseconds kSleepAfterFirstEvent{1000};
    std::this_thread::sleep_for(kSleepAfterFirstEvent);

    puppet_.WriteLine(command_for_puppet);
    while (puppet_.ReadLine() != IntegrationTestPuppetConstants::kDoneResponse) continue;

    // Some producers might miss some of the final data if we stop the capture immediately after the
    // puppet is done.
    constexpr std::chrono::milliseconds kSleepBeforeStopCapture{100};
    std::this_thread::sleep_for(kSleepBeforeStopCapture);
    return StopCaptureAndGetEvents();
  }

 private:
  void StartCapture(CaptureOptions capture_options) {
    CHECK(!capture_thread_.joinable());
    capture_thread_ = std::thread(&OrbitServiceIntegrationTestFixture::CaptureThread, this,
                                  std::move(capture_options));
  }

  [[nodiscard]] std::vector<ClientCaptureEvent> StopCaptureAndGetEvents() {
    CHECK(capture_thread_.joinable());

    {
      absl::ReaderMutexLock lock{&capture_reader_writer_mutex_};
      LOG("Stopping capture");
      bool writes_done_succeeded = capture_reader_writer_->WritesDone();
      CHECK(writes_done_succeeded);
    }

    capture_thread_.join();

    std::vector<ClientCaptureEvent> events;
    {
      absl::MutexLock lock{&capture_events_mutex_};
      events = std::move(capture_events_);
      capture_events_.clear();
    }
    return events;
  }

  void CaptureThread(CaptureOptions capture_options) {
    std::string address = absl::StrFormat("localhost:%u", kOrbitServicePort);
    std::shared_ptr<grpc::Channel> channel =
        grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    grpc::ClientContext context;
    std::unique_ptr<orbit_grpc_protos::CaptureService::Stub> capture_service =
        orbit_grpc_protos::CaptureService::NewStub(channel);

    {
      absl::WriterMutexLock lock{&capture_reader_writer_mutex_};
      CHECK(capture_reader_writer_ == nullptr);
      LOG("Starting capture");
      capture_reader_writer_ = capture_service->Capture(&context);
    }
    {
      absl::ReaderMutexLock lock{&capture_reader_writer_mutex_};
      orbit_grpc_protos::CaptureRequest capture_request;
      *capture_request.mutable_capture_options() = std::move(capture_options);
      bool write_capture_request_result = capture_reader_writer_->Write(capture_request);
      CHECK(write_capture_request_result);
    }

    LOG("Receiving events");
    while (true) {
      orbit_grpc_protos::CaptureResponse capture_response;
      bool read_capture_response_result;
      {
        absl::ReaderMutexLock lock{&capture_reader_writer_mutex_};
        read_capture_response_result = capture_reader_writer_->Read(&capture_response);
      }

      if (!read_capture_response_result) {
        // This signals that CaptureService::Capture has returned.
        break;
      }
      for (ClientCaptureEvent& event : *capture_response.mutable_capture_events()) {
        absl::MutexLock lock{&capture_events_mutex_};
        capture_events_.emplace_back(std::move(event));
      }
    }

    LOG("Capture finished");
    {
      absl::WriterMutexLock lock{&capture_reader_writer_mutex_};
      CHECK(capture_reader_writer_ != nullptr);
      capture_reader_writer_ = nullptr;
    }
  }

  void WaitForFirstEvent() {
    absl::MutexLock lock{&capture_events_mutex_};
    capture_events_mutex_.Await(absl::Condition(
        +[](std::vector<ClientCaptureEvent>* capture_events) { return !capture_events->empty(); },
        &capture_events_));
    LOG("First ClientCaptureEvent received");
  }

  ChildProcess puppet_;
  ChildProcess orbit_service_;

  std::thread capture_thread_;
  absl::Mutex capture_reader_writer_mutex_;
  std::unique_ptr<grpc::ClientReaderWriter<orbit_grpc_protos::CaptureRequest,
                                           orbit_grpc_protos::CaptureResponse>>
      capture_reader_writer_ ABSL_GUARDED_BY(capture_reader_writer_mutex_);
  absl::Mutex capture_events_mutex_;
  std::vector<ClientCaptureEvent> capture_events_ ABSL_GUARDED_BY(capture_events_mutex_);
};

}  // namespace

static void VerifyCaptureStartedEvent(const ClientCaptureEvent& event,
                                      const CaptureOptions& original_capture_options) {
  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kCaptureStarted);
  const orbit_grpc_protos::CaptureStarted& capture_started = event.capture_started();
  EXPECT_EQ(capture_started.process_id(), original_capture_options.pid());
  EXPECT_EQ(capture_started.executable_path(),
            GetExecutableBinaryPath(original_capture_options.pid()));
  EXPECT_NE(capture_started.capture_start_timestamp_ns(), 0);
  EXPECT_NE(capture_started.capture_start_unix_time_ns(), 0);
  EXPECT_EQ(capture_started.orbit_version_major(), orbit_version::GetVersion().major_version);
  EXPECT_EQ(capture_started.orbit_version_minor(), orbit_version::GetVersion().minor_version);
  EXPECT_EQ(capture_started.capture_options().SerializeAsString(),
            original_capture_options.SerializeAsString());
}

static void VerifyClockResolutionEvent(const ClientCaptureEvent& event) {
  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kClockResolutionEvent);
  const orbit_grpc_protos::ClockResolutionEvent& clock_resolution_event =
      event.clock_resolution_event();
  EXPECT_GT(clock_resolution_event.clock_resolution_ns(), 0);
}

static void VerifyCaptureFinishedEvent(const ClientCaptureEvent& event) {
  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kCaptureFinished);
  const orbit_grpc_protos::CaptureFinished& capture_finished = event.capture_finished();
  EXPECT_EQ(capture_finished.status(), orbit_grpc_protos::CaptureFinished::kSuccessful);
  EXPECT_EQ(capture_finished.error_message(), "");
}

static void VerifyInitialAndFinalEvents(const std::vector<ClientCaptureEvent>& events,
                                        const CaptureOptions& original_capture_options) {
  ASSERT_GE(events.size(), 3);
  VerifyCaptureStartedEvent(events.front(), original_capture_options);
  VerifyClockResolutionEvent(events[1]);
  VerifyCaptureFinishedEvent(events.back());
}

static void VerifyErrorEvents(const std::vector<ClientCaptureEvent>& events) {
  bool errors_with_perf_event_open_event_found = false;
  for (const ClientCaptureEvent& event : events) {
    EXPECT_NE(event.event_case(), ClientCaptureEvent::kErrorEnablingOrbitApiEvent);
    EXPECT_NE(event.event_case(), ClientCaptureEvent::kErrorEnablingUserSpaceInstrumentationEvent);
    if (event.event_case() == ClientCaptureEvent::kErrorsWithPerfEventOpenEvent) {
      errors_with_perf_event_open_event_found = true;
    }
    EXPECT_NE(event.event_case(), ClientCaptureEvent::kLostPerfRecordsEvent);
    EXPECT_NE(event.event_case(), ClientCaptureEvent::kOutOfOrderEventsDiscardedEvent);
    EXPECT_NE(event.event_case(),
              ClientCaptureEvent::kWarningInstrumentingWithUserSpaceInstrumentationEvent);
  }
  EXPECT_EQ(errors_with_perf_event_open_event_found, !IsRunningAsRoot());
}

TEST(OrbitServiceIntegrationTest, CaptureSmoke) {
  OrbitServiceIntegrationTestFixture fixture;
  CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  std::vector<ClientCaptureEvent> events =
      fixture.CaptureAndGetEvents(IntegrationTestPuppetConstants::kSleepCommand, capture_options);

  VerifyInitialAndFinalEvents(events, capture_options);
  VerifyErrorEvents(events);
}

static void VerifyFunctionCallsOfOuterAndInnerFunction(
    const std::vector<ClientCaptureEvent>& events, uint32_t pid, uint64_t outer_function_id,
    uint64_t inner_function_id) {
  std::vector<orbit_grpc_protos::FunctionCall> function_calls;
  for (const ClientCaptureEvent& event : events) {
    if (!event.has_function_call()) {
      continue;
    }
    function_calls.emplace_back(event.function_call());
  }

  VerifyFunctionCallsOfPuppetOuterAndInnerFunction(function_calls, pid, outer_function_id,
                                                   inner_function_id,
                                                   /*expect_return_value_and_registers=*/false);
}

TEST(OrbitServiceIntegrationTest, FunctionCallsWithUserSpaceInstrumentation) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }

  OrbitServiceIntegrationTestFixture fixture;
  CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  capture_options.set_dynamic_instrumentation_method(CaptureOptions::kUserSpaceInstrumentation);
  constexpr uint64_t kOuterFunctionId = 1;
  constexpr uint64_t kInnerFunctionId = 2;
  AddPuppetOuterAndInnerFunctionToCaptureOptions(&capture_options, fixture.GetPuppetPidNative(),
                                                 kOuterFunctionId, kInnerFunctionId);
  std::vector<ClientCaptureEvent> events = fixture.CaptureAndGetEvents(
      IntegrationTestPuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyInitialAndFinalEvents(events, capture_options);
  VerifyErrorEvents(events);
  VerifyFunctionCallsOfOuterAndInnerFunction(events, fixture.GetPuppetPid(), kOuterFunctionId,
                                             kInnerFunctionId);
}

}  // namespace orbit_linux_tracing_integration_tests
