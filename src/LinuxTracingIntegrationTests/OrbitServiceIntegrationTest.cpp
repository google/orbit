// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/thread_annotations.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ApiUtils/EncodedString.h"
#include "ApiUtils/GetFunctionTableAddressPrefix.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "IntegrationTestChildProcess.h"
#include "IntegrationTestCommons.h"
#include "IntegrationTestPuppet.h"
#include "IntegrationTestUtils.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitService.h"
#include "OrbitVersion/OrbitVersion.h"

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_linux_tracing_integration_tests {

namespace {

// Note that the tests will behave unexpectedly if another instance of OrbitService is running on
// the system.
constexpr uint16_t kOrbitServicePort = 44765;

int OrbitServiceMain() {
  ORBIT_LOG("OrbitService started");
  std::atomic<bool> exit_requested = false;
  // OrbitService's loop terminates when EOF is received, no need to manipulate exit_requested.
  ErrorMessageOr<void> error_message =
      orbit_service::OrbitService{kOrbitServicePort, /*start_producer_side_server=*/true,
                                  /*dev_mode=*/false}
          .Run(&exit_requested);

  if (error_message.has_error()) {
    ORBIT_LOG("OrbitService finished with an error: %s", error_message.error().message());
    constexpr int kExitCodeIndicatingErrorMessage = 42;
    return kExitCodeIndicatingErrorMessage;
  }

  ORBIT_LOG("OrbitService finished with exit code: 0");
  return 0;
}

using PuppetConstants = IntegrationTestPuppetConstants;

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
    while (puppet_.ReadLine() != PuppetConstants::kDoneResponse) continue;

    // Some producers might miss some of the final data if we stop the capture immediately after the
    // puppet is done.
    constexpr std::chrono::milliseconds kSleepBeforeStopCapture{100};
    std::this_thread::sleep_for(kSleepBeforeStopCapture);
    return StopCaptureAndGetEvents();
  }

 private:
  void StartCapture(CaptureOptions capture_options) {
    ORBIT_CHECK(!capture_thread_.joinable());
    capture_thread_ = std::thread(&OrbitServiceIntegrationTestFixture::CaptureThread, this,
                                  std::move(capture_options));
  }

  [[nodiscard]] std::vector<ClientCaptureEvent> StopCaptureAndGetEvents() {
    ORBIT_CHECK(capture_thread_.joinable());

    {
      absl::ReaderMutexLock lock{&capture_reader_writer_mutex_};
      ORBIT_LOG("Stopping capture");
      bool writes_done_succeeded = capture_reader_writer_->WritesDone();
      ORBIT_CHECK(writes_done_succeeded);
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
      ORBIT_CHECK(capture_reader_writer_ == nullptr);
      ORBIT_LOG("Starting capture");
      capture_reader_writer_ = capture_service->Capture(&context);
    }
    {
      absl::ReaderMutexLock lock{&capture_reader_writer_mutex_};
      orbit_grpc_protos::CaptureRequest capture_request;
      *capture_request.mutable_capture_options() = std::move(capture_options);
      bool write_capture_request_result = capture_reader_writer_->Write(capture_request);
      ORBIT_CHECK(write_capture_request_result);
    }

    ORBIT_LOG("Receiving events");
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

    ORBIT_LOG("Capture finished");
    {
      absl::WriterMutexLock lock{&capture_reader_writer_mutex_};
      ORBIT_CHECK(capture_reader_writer_ != nullptr);
      capture_reader_writer_ = nullptr;
    }
  }

  void WaitForFirstEvent() {
    absl::MutexLock lock{&capture_events_mutex_};
    capture_events_mutex_.Await(absl::Condition(
        +[](std::vector<ClientCaptureEvent>* capture_events) { return !capture_events->empty(); },
        &capture_events_));
    ORBIT_LOG("First ClientCaptureEvent received");
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

static void VerifyInitialAndFinalEvents(absl::Span<const ClientCaptureEvent> events,
                                        const CaptureOptions& original_capture_options) {
  ASSERT_GE(events.size(), 3);
  VerifyCaptureStartedEvent(events.front(), original_capture_options);
  VerifyClockResolutionEvent(events[1]);
  VerifyCaptureFinishedEvent(events.back());
}

static void VerifyErrorEvents(absl::Span<const ClientCaptureEvent> events) {
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
      fixture.CaptureAndGetEvents(PuppetConstants::kSleepCommand, capture_options);

  VerifyInitialAndFinalEvents(events, capture_options);
  VerifyErrorEvents(events);
}

static void VerifyFunctionCallsOfOuterAndInnerFunction(absl::Span<const ClientCaptureEvent> events,
                                                       uint32_t pid, uint64_t outer_function_id,
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
  std::vector<ClientCaptureEvent> events =
      fixture.CaptureAndGetEvents(PuppetConstants::kCallOuterFunctionCommand, capture_options);

  VerifyInitialAndFinalEvents(events, capture_options);
  VerifyErrorEvents(events);
  VerifyFunctionCallsOfOuterAndInnerFunction(events, fixture.GetPuppetPid(), kOuterFunctionId,
                                             kInnerFunctionId);
}

static void AddOrbitApiToCaptureOptions(orbit_grpc_protos::CaptureOptions* capture_options,
                                        pid_t pid) {
  capture_options->set_enable_api(true);

  const orbit_grpc_protos::ModuleInfo& module_info = GetExecutableBinaryModuleInfo(pid);
  const orbit_grpc_protos::ModuleSymbols& module_symbols = GetExecutableBinaryModuleSymbols(pid);

  std::string api_function_name = absl::StrFormat(
      "%s%u", orbit_api_utils::kOrbitApiGetFunctionTableAddressPrefix, kOrbitApiVersion);
  for (const orbit_grpc_protos::SymbolInfo& symbol_info : module_symbols.symbol_infos()) {
    if (symbol_info.demangled_name() == api_function_name) {
      uint64_t absolute_address = orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
          symbol_info.address(), module_info.address_start(), module_info.load_bias(),
          module_info.executable_segment_offset());

      orbit_grpc_protos::ApiFunction* api_function = capture_options->add_api_functions();
      api_function->set_module_path(module_info.file_path());
      api_function->set_module_build_id(module_info.build_id());
      api_function->set_relative_address(symbol_info.address());
      api_function->set_absolute_address(absolute_address);
      api_function->set_name(api_function_name);
      api_function->set_api_version(kOrbitApiVersion);
      break;
    }
  }
  ORBIT_CHECK(capture_options->api_functions_size() == 1);
}

static std::pair<uint64_t, uint64_t> GetUseOrbitApiFunctionVirtualAddressRange(pid_t pid) {
  const orbit_grpc_protos::ModuleInfo& module_info = GetExecutableBinaryModuleInfo(pid);
  const orbit_grpc_protos::ModuleSymbols& module_symbols = GetExecutableBinaryModuleSymbols(pid);
  for (const orbit_grpc_protos::SymbolInfo& symbol : module_symbols.symbol_infos()) {
    if (absl::StrContains(symbol.demangled_name(), PuppetConstants::kUseOrbitApiFunctionName)) {
      const uint64_t virtual_address_start =
          orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
              symbol.address(), module_info.address_start(), module_info.load_bias(),
              module_info.executable_segment_offset());
      const uint64_t virtual_address_end = virtual_address_start + symbol.size() - 1;
      return {virtual_address_start, virtual_address_end};
    }
  }
  ORBIT_UNREACHABLE();
}

TEST(OrbitServiceIntegrationTest, OrbitApi) {
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }

  OrbitServiceIntegrationTestFixture fixture;
  CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  AddOrbitApiToCaptureOptions(&capture_options, fixture.GetPuppetPidNative());
  const std::pair<uint64_t, uint64_t>& use_orbit_api_virtual_address_range =
      GetUseOrbitApiFunctionVirtualAddressRange(fixture.GetPuppetPidNative());

  // Take an initial capture so that the communication between the target and OrbitService gets
  // initialized and we don't lose any event at the beginning of the next capture.
  // TODO(b/206359125,b/237403760): Remove this extra capture once b/206359125 has been fixed again.
  ORBIT_LOG("Taking an initial capture to initialize CaptureEventProducer in the target");
  std::ignore = fixture.CaptureAndGetEvents(PuppetConstants::kOrbitApiCommand, capture_options);

  ORBIT_LOG("Taking the capture that we are actually going to verify");
  std::vector<ClientCaptureEvent> events =
      fixture.CaptureAndGetEvents(PuppetConstants::kOrbitApiCommand, capture_options);

  VerifyInitialAndFinalEvents(events, capture_options);
  VerifyErrorEvents(events);

  bool expect_next_api_scope_start_coming_from_scope = true;
  uint64_t api_scope_start_count = 0;
  uint64_t api_scope_stop_count = 0;
  uint64_t api_scope_start_async_count = 0;
  uint64_t api_scope_stop_async_count = 0;
  uint64_t api_async_string_count = 0;
  uint64_t api_track_int_count = 0;
  uint64_t api_track_uint_count = 0;
  uint64_t api_track_int64_count = 0;
  uint64_t api_track_uint64_count = 0;
  uint64_t api_track_float_count = 0;
  uint64_t api_track_double_count = 0;
  uint64_t previous_timestamp_ns = 0;
  for (const ClientCaptureEvent& event : events) {
    switch (event.event_case()) {
      case ClientCaptureEvent::kApiScopeStart: {
        const orbit_grpc_protos::ApiScopeStart& api_scope_start = event.api_scope_start();
        EXPECT_EQ(api_scope_start.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_scope_start.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_scope_start.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_scope_start.timestamp_ns();
        std::string decoded_name = orbit_api::DecodeString(
            api_scope_start.encoded_name_1(), api_scope_start.encoded_name_2(),
            api_scope_start.encoded_name_3(), api_scope_start.encoded_name_4(),
            api_scope_start.encoded_name_5(), api_scope_start.encoded_name_6(),
            api_scope_start.encoded_name_7(), api_scope_start.encoded_name_8(),
            api_scope_start.encoded_name_additional().data(),
            api_scope_start.encoded_name_additional_size());
        if (expect_next_api_scope_start_coming_from_scope) {
          EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiScopeName);
          EXPECT_EQ(api_scope_start.color_rgba(), PuppetConstants::kOrbitApiScopeColor);
          EXPECT_EQ(api_scope_start.group_id(), PuppetConstants::kOrbitApiScopeGroupId);
        } else {
          EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiStartName);
          EXPECT_EQ(api_scope_start.color_rgba(), PuppetConstants::kOrbitApiStartColor);
          EXPECT_EQ(api_scope_start.group_id(), PuppetConstants::kOrbitApiStartGroupId);
        }
        expect_next_api_scope_start_coming_from_scope =
            !expect_next_api_scope_start_coming_from_scope;
        EXPECT_THAT(api_scope_start.address_in_function(),
                    testing::AllOf(testing::Ge(use_orbit_api_virtual_address_range.first),
                                   testing::Le(use_orbit_api_virtual_address_range.second)));
        ++api_scope_start_count;
      } break;

      case ClientCaptureEvent::kApiScopeStop: {
        const orbit_grpc_protos::ApiScopeStop& api_scope_stop = event.api_scope_stop();
        EXPECT_EQ(api_scope_stop.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_scope_stop.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_scope_stop.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_scope_stop.timestamp_ns();
        ++api_scope_stop_count;
      } break;

      case ClientCaptureEvent::kApiScopeStartAsync: {
        const orbit_grpc_protos::ApiScopeStartAsync& api_scope_start_async =
            event.api_scope_start_async();
        EXPECT_EQ(api_scope_start_async.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_scope_start_async.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_scope_start_async.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_scope_start_async.timestamp_ns();
        std::string decoded_name = orbit_api::DecodeString(
            api_scope_start_async.encoded_name_1(), api_scope_start_async.encoded_name_2(),
            api_scope_start_async.encoded_name_3(), api_scope_start_async.encoded_name_4(),
            api_scope_start_async.encoded_name_5(), api_scope_start_async.encoded_name_6(),
            api_scope_start_async.encoded_name_7(), api_scope_start_async.encoded_name_8(),
            api_scope_start_async.encoded_name_additional().data(),
            api_scope_start_async.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiStartAsyncName);
        EXPECT_EQ(api_scope_start_async.id(), PuppetConstants::kOrbitApiStartAsyncId);
        EXPECT_EQ(api_scope_start_async.color_rgba(), PuppetConstants::kOrbitApiStartAsyncColor);
        ++api_scope_start_async_count;
      } break;

      case ClientCaptureEvent::kApiScopeStopAsync: {
        const orbit_grpc_protos::ApiScopeStopAsync& api_scope_stop_async =
            event.api_scope_stop_async();
        EXPECT_EQ(api_scope_stop_async.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_scope_stop_async.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_scope_stop_async.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_scope_stop_async.timestamp_ns();
        EXPECT_EQ(api_scope_stop_async.id(), PuppetConstants::kOrbitApiStartAsyncId);
        ++api_scope_stop_async_count;
      } break;

      case ClientCaptureEvent::kApiStringEvent: {
        const orbit_grpc_protos::ApiStringEvent& api_async_string = event.api_string_event();
        EXPECT_EQ(api_async_string.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_async_string.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_async_string.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_async_string.timestamp_ns();
        std::string decoded_name = orbit_api::DecodeString(
            api_async_string.encoded_name_1(), api_async_string.encoded_name_2(),
            api_async_string.encoded_name_3(), api_async_string.encoded_name_4(),
            api_async_string.encoded_name_5(), api_async_string.encoded_name_6(),
            api_async_string.encoded_name_7(), api_async_string.encoded_name_8(),
            api_async_string.encoded_name_additional().data(),
            api_async_string.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiAsyncStringName);
        EXPECT_EQ(api_async_string.id(), PuppetConstants::kOrbitApiStartAsyncId);
        EXPECT_EQ(api_async_string.color_rgba(), PuppetConstants::kOrbitApiAsyncStringColor);
        ++api_async_string_count;
      } break;

      case ClientCaptureEvent::kApiTrackDouble: {
        const orbit_grpc_protos::ApiTrackDouble& api_track_double = event.api_track_double();
        EXPECT_EQ(api_track_double.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_track_double.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_track_double.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_track_double.timestamp_ns();
        EXPECT_EQ(api_track_double.data(), PuppetConstants::kOrbitApiDoubleValue);
        std::string decoded_name = orbit_api::DecodeString(
            api_track_double.encoded_name_1(), api_track_double.encoded_name_2(),
            api_track_double.encoded_name_3(), api_track_double.encoded_name_4(),
            api_track_double.encoded_name_5(), api_track_double.encoded_name_6(),
            api_track_double.encoded_name_7(), api_track_double.encoded_name_8(),
            api_track_double.encoded_name_additional().data(),
            api_track_double.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiDoubleName);
        EXPECT_EQ(api_track_double.color_rgba(), PuppetConstants::kOrbitApiDoubleColor);
        ++api_track_double_count;
      } break;

      case ClientCaptureEvent::kApiTrackFloat: {
        const orbit_grpc_protos::ApiTrackFloat& api_track_float = event.api_track_float();
        EXPECT_EQ(api_track_float.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_track_float.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_track_float.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_track_float.timestamp_ns();
        EXPECT_EQ(api_track_float.data(), PuppetConstants::kOrbitApiFloatValue);
        std::string decoded_name = orbit_api::DecodeString(
            api_track_float.encoded_name_1(), api_track_float.encoded_name_2(),
            api_track_float.encoded_name_3(), api_track_float.encoded_name_4(),
            api_track_float.encoded_name_5(), api_track_float.encoded_name_6(),
            api_track_float.encoded_name_7(), api_track_float.encoded_name_8(),
            api_track_float.encoded_name_additional().data(),
            api_track_float.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiFloatName);
        EXPECT_EQ(api_track_float.color_rgba(), PuppetConstants::kOrbitApiFloatColor);
        ++api_track_float_count;
      } break;

      case ClientCaptureEvent::kApiTrackInt: {
        const orbit_grpc_protos::ApiTrackInt& api_track_int = event.api_track_int();
        EXPECT_EQ(api_track_int.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_track_int.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_track_int.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_track_int.timestamp_ns();
        EXPECT_EQ(api_track_int.data(), PuppetConstants::kOrbitApiIntValue);
        std::string decoded_name =
            orbit_api::DecodeString(api_track_int.encoded_name_1(), api_track_int.encoded_name_2(),
                                    api_track_int.encoded_name_3(), api_track_int.encoded_name_4(),
                                    api_track_int.encoded_name_5(), api_track_int.encoded_name_6(),
                                    api_track_int.encoded_name_7(), api_track_int.encoded_name_8(),
                                    api_track_int.encoded_name_additional().data(),
                                    api_track_int.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiIntName);
        EXPECT_EQ(api_track_int.color_rgba(), PuppetConstants::kOrbitApiIntColor);
        ++api_track_int_count;
      } break;

      case ClientCaptureEvent::kApiTrackInt64: {
        const orbit_grpc_protos::ApiTrackInt64& api_track_int64 = event.api_track_int64();
        EXPECT_EQ(api_track_int64.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_track_int64.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_track_int64.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_track_int64.timestamp_ns();
        EXPECT_EQ(api_track_int64.data(), PuppetConstants::kOrbitApiInt64Value);
        std::string decoded_name = orbit_api::DecodeString(
            api_track_int64.encoded_name_1(), api_track_int64.encoded_name_2(),
            api_track_int64.encoded_name_3(), api_track_int64.encoded_name_4(),
            api_track_int64.encoded_name_5(), api_track_int64.encoded_name_6(),
            api_track_int64.encoded_name_7(), api_track_int64.encoded_name_8(),
            api_track_int64.encoded_name_additional().data(),
            api_track_int64.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiInt64Name);
        EXPECT_EQ(api_track_int64.color_rgba(), PuppetConstants::kOrbitApiInt64Color);
        ++api_track_int64_count;
      } break;

      case ClientCaptureEvent::kApiTrackUint: {
        const orbit_grpc_protos::ApiTrackUint& api_track_uint = event.api_track_uint();
        EXPECT_EQ(api_track_uint.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_track_uint.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_track_uint.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_track_uint.timestamp_ns();
        EXPECT_EQ(api_track_uint.data(), PuppetConstants::kOrbitApiUintValue);
        std::string decoded_name = orbit_api::DecodeString(
            api_track_uint.encoded_name_1(), api_track_uint.encoded_name_2(),
            api_track_uint.encoded_name_3(), api_track_uint.encoded_name_4(),
            api_track_uint.encoded_name_5(), api_track_uint.encoded_name_6(),
            api_track_uint.encoded_name_7(), api_track_uint.encoded_name_8(),
            api_track_uint.encoded_name_additional().data(),
            api_track_uint.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiUintName);
        EXPECT_EQ(api_track_uint.color_rgba(), PuppetConstants::kOrbitApiUintColor);
        ++api_track_uint_count;
      } break;

      case ClientCaptureEvent::kApiTrackUint64: {
        const orbit_grpc_protos::ApiTrackUint64& api_track_uint64 = event.api_track_uint64();
        EXPECT_EQ(api_track_uint64.pid(), fixture.GetPuppetPid());
        EXPECT_EQ(api_track_uint64.tid(), fixture.GetPuppetPid());
        EXPECT_GT(api_track_uint64.timestamp_ns(), previous_timestamp_ns);
        previous_timestamp_ns = api_track_uint64.timestamp_ns();
        EXPECT_EQ(api_track_uint64.data(), PuppetConstants::kOrbitApiUint64Value);
        std::string decoded_name = orbit_api::DecodeString(
            api_track_uint64.encoded_name_1(), api_track_uint64.encoded_name_2(),
            api_track_uint64.encoded_name_3(), api_track_uint64.encoded_name_4(),
            api_track_uint64.encoded_name_5(), api_track_uint64.encoded_name_6(),
            api_track_uint64.encoded_name_7(), api_track_uint64.encoded_name_8(),
            api_track_uint64.encoded_name_additional().data(),
            api_track_uint64.encoded_name_additional_size());
        EXPECT_EQ(decoded_name, PuppetConstants::kOrbitApiUint64Name);
        EXPECT_EQ(api_track_uint64.color_rgba(), PuppetConstants::kOrbitApiUint64Color);
        ++api_track_uint64_count;
      } break;

      case ClientCaptureEvent::EVENT_NOT_SET:
        ORBIT_UNREACHABLE();
      default:
        break;
    }
  }

  EXPECT_EQ(api_scope_start_count, 2 * PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_scope_stop_count, 2 * PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_scope_start_async_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_scope_stop_async_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_async_string_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_track_int_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_track_uint_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_track_int64_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_track_uint64_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_track_float_count, PuppetConstants::kOrbitApiUsageCount);
  EXPECT_EQ(api_track_double_count, PuppetConstants::kOrbitApiUsageCount);
}

TEST(OrbitServiceIntegrationTest, MemoryTracing) {
  // Memory tracing won't work if the target doesn't have a memory cgroup. See http://b/208998708.
  if (!CheckIsStadiaInstance()) {
    GTEST_SKIP();
  }
  if (!CheckIsRunningAsRoot()) {
    GTEST_SKIP();
  }

  OrbitServiceIntegrationTestFixture fixture;
  CaptureOptions capture_options = fixture.BuildDefaultCaptureOptions();
  capture_options.set_collect_memory_info(true);
  constexpr uint64_t kMemorySamplingPeriodNs = 10'000'000;
  capture_options.set_memory_sampling_period_ns(kMemorySamplingPeriodNs);
  std::vector<ClientCaptureEvent> events =
      fixture.CaptureAndGetEvents(PuppetConstants::kIncreaseRssCommand, capture_options);
  VerifyInitialAndFinalEvents(events, capture_options);
  VerifyErrorEvents(events);

  uint64_t memory_usage_event_count = 0;
  uint64_t initial_memory_usage_timestamp_ns = 0;
  uint64_t previous_memory_usage_timestamp_ns = 0;
  int64_t previous_system_total_kb = 0;
  std::string previous_cgroup_name;
  int64_t previous_cgroup_limit = 0;

  int64_t initial_process_rss_anon_kb = 0;
  int64_t initial_cgroup_rss = 0;
  int64_t previous_process_rss_anon_kb = 0;
  int64_t previous_cgroup_rss = 0;
  for (const ClientCaptureEvent& event : events) {
    if (!event.has_memory_usage_event()) {
      continue;
    }

    ++memory_usage_event_count;
    const orbit_grpc_protos::MemoryUsageEvent& memory_usage_event = event.memory_usage_event();

    ASSERT_TRUE(memory_usage_event.has_system_memory_usage());
    const orbit_grpc_protos::SystemMemoryUsage& system_memory_usage_event =
        memory_usage_event.system_memory_usage();
    ASSERT_TRUE(memory_usage_event.has_process_memory_usage());
    const orbit_grpc_protos::ProcessMemoryUsage& process_memory_usage_event =
        memory_usage_event.process_memory_usage();
    ASSERT_TRUE(memory_usage_event.has_cgroup_memory_usage());
    const orbit_grpc_protos::CGroupMemoryUsage& cgroup_memory_usage_event =
        memory_usage_event.cgroup_memory_usage();

    // Basic expectations.
    EXPECT_GT(memory_usage_event.timestamp_ns(), 0);
    EXPECT_GT(system_memory_usage_event.timestamp_ns(), 0);
    EXPECT_GT(system_memory_usage_event.total_kb(), 0);
    EXPECT_GT(system_memory_usage_event.free_kb(), 0);
    EXPECT_GT(system_memory_usage_event.available_kb(), 0);
    EXPECT_GT(system_memory_usage_event.buffers_kb(), 0);
    EXPECT_GT(system_memory_usage_event.cached_kb(), 0);
    EXPECT_GT(system_memory_usage_event.pgfault(), 0);
    EXPECT_GT(system_memory_usage_event.pgmajfault(), 0);
    EXPECT_GT(process_memory_usage_event.pid(), 0);
    EXPECT_GT(process_memory_usage_event.timestamp_ns(), 0);
    EXPECT_GT(process_memory_usage_event.minflt(), 0);
    EXPECT_GE(process_memory_usage_event.majflt(), 0);
    EXPECT_GT(process_memory_usage_event.rss_anon_kb(), 0);
    EXPECT_GT(cgroup_memory_usage_event.timestamp_ns(), 0);
    EXPECT_GT(cgroup_memory_usage_event.limit_bytes(), 0);
    EXPECT_GT(cgroup_memory_usage_event.rss_bytes(), 0);
    EXPECT_GT(cgroup_memory_usage_event.mapped_file_bytes(), 0);
    EXPECT_GT(cgroup_memory_usage_event.pgfault(), 0);
    EXPECT_GE(cgroup_memory_usage_event.pgmajfault(), 0);
    EXPECT_GE(cgroup_memory_usage_event.unevictable_bytes(), 0);
    EXPECT_GT(cgroup_memory_usage_event.inactive_anon_bytes(), 0);
    EXPECT_GE(cgroup_memory_usage_event.active_anon_bytes(), 0);
    EXPECT_GE(cgroup_memory_usage_event.inactive_file_bytes(), 0);
    EXPECT_GT(cgroup_memory_usage_event.active_file_bytes(), 0);

    // Expect MemoryUsageEvents to be in order.
    EXPECT_GT(memory_usage_event.timestamp_ns(), previous_memory_usage_timestamp_ns);

    // The events should be synchronized correctly.
    EXPECT_GE(memory_usage_event.timestamp_ns(),
              std::min({system_memory_usage_event.timestamp_ns(),
                        process_memory_usage_event.timestamp_ns(),
                        cgroup_memory_usage_event.timestamp_ns()}));
    EXPECT_LE(memory_usage_event.timestamp_ns(),
              std::max({system_memory_usage_event.timestamp_ns(),
                        process_memory_usage_event.timestamp_ns(),
                        cgroup_memory_usage_event.timestamp_ns()}));
    EXPECT_LE(std::max({system_memory_usage_event.timestamp_ns(),
                        process_memory_usage_event.timestamp_ns(),
                        cgroup_memory_usage_event.timestamp_ns()}) -
                  std::min({system_memory_usage_event.timestamp_ns(),
                            process_memory_usage_event.timestamp_ns(),
                            cgroup_memory_usage_event.timestamp_ns()}),
              kMemorySamplingPeriodNs);

    if (initial_memory_usage_timestamp_ns == 0) {
      initial_memory_usage_timestamp_ns = memory_usage_event.timestamp_ns();
    }
    previous_memory_usage_timestamp_ns = memory_usage_event.timestamp_ns();

    // The total memory (MemTotal) should be constant and reasonably large.
    constexpr int64_t kMinExpectedSystemTotal = 1024LL * 1024 * 1024;
    EXPECT_GE(system_memory_usage_event.total_kb() * 1024, kMinExpectedSystemTotal);
    if (previous_system_total_kb != 0) {
      EXPECT_EQ(system_memory_usage_event.total_kb(), previous_system_total_kb);
    }
    previous_system_total_kb = system_memory_usage_event.total_kb();

    // MemFree and MemAvailable should be reasonably large.
    constexpr int64_t kMinExpectedSystemFree = 1024LL * 1024 * 1024;
    EXPECT_GE(system_memory_usage_event.free_kb() * 1024, kMinExpectedSystemFree);
    EXPECT_LT(system_memory_usage_event.free_kb(), system_memory_usage_event.total_kb());
    constexpr int64_t kMinExpectedSystemAvailable = 1024LL * 1024 * 1024;
    EXPECT_GE(system_memory_usage_event.available_kb() * 1024, kMinExpectedSystemAvailable);
    EXPECT_LT(system_memory_usage_event.available_kb(), system_memory_usage_event.total_kb());

    EXPECT_EQ(process_memory_usage_event.pid(), fixture.GetPuppetPid());

    // The name of the memory cgroup should be constant and not empty.
    EXPECT_NE(cgroup_memory_usage_event.cgroup_name(), "");
    if (!previous_cgroup_name.empty()) {
      EXPECT_EQ(previous_cgroup_name, cgroup_memory_usage_event.cgroup_name());
    }
    previous_cgroup_name = cgroup_memory_usage_event.cgroup_name();

    // The memory limit of the cgroup should be constant and reasonably large.
    constexpr int64_t kMinExpectedCgroupMemoryLimit = 1024LL * 1024 * 1024;
    EXPECT_GE(cgroup_memory_usage_event.limit_bytes(), kMinExpectedCgroupMemoryLimit);
    if (previous_cgroup_limit != 0) {
      EXPECT_EQ(cgroup_memory_usage_event.limit_bytes(), previous_cgroup_limit);
    }
    previous_cgroup_limit = cgroup_memory_usage_event.limit_bytes();

    // Expect an increase in resident set size of the process and the cgroup as the puppet executes
    // the command.
    if (initial_process_rss_anon_kb == 0) {
      EXPECT_EQ(initial_cgroup_rss, 0);
      initial_process_rss_anon_kb = process_memory_usage_event.rss_anon_kb();
      initial_cgroup_rss = cgroup_memory_usage_event.rss_bytes();
    }
    if (previous_process_rss_anon_kb != 0) {
      EXPECT_NE(previous_cgroup_rss, 0);
      EXPECT_GE(process_memory_usage_event.rss_anon_kb(), previous_process_rss_anon_kb);
      EXPECT_GE(cgroup_memory_usage_event.rss_bytes(), previous_cgroup_rss);
    }
    previous_process_rss_anon_kb = process_memory_usage_event.rss_anon_kb();
    previous_cgroup_rss = cgroup_memory_usage_event.rss_bytes();
  }

  // Verify the memory sampling period.
  ASSERT_GT(memory_usage_event_count, 1);
  uint64_t avg_memory_event_period_ns =
      (previous_memory_usage_timestamp_ns - initial_memory_usage_timestamp_ns) /
      memory_usage_event_count;
  constexpr uint64_t kMinExpectedMemoryEventPeriodNs = kMemorySamplingPeriodNs / 10 * 9;
  constexpr uint64_t kMaxExpectedMemoryEventPeriodNs = kMemorySamplingPeriodNs / 10 * 11;
  EXPECT_GE(avg_memory_event_period_ns, kMinExpectedMemoryEventPeriodNs);
  EXPECT_LE(avg_memory_event_period_ns, kMaxExpectedMemoryEventPeriodNs);

  // Verify that the increase in resident set size was recorded.
  constexpr uint64_t kRssIncreaseTolerance = PuppetConstants::kRssIncreaseB / 10;
  EXPECT_GE(
      previous_process_rss_anon_kb * 1024,
      initial_process_rss_anon_kb * 1024 + PuppetConstants::kRssIncreaseB - kRssIncreaseTolerance);
  EXPECT_GE(previous_cgroup_rss,
            initial_cgroup_rss + PuppetConstants::kRssIncreaseB - kRssIncreaseTolerance);
}

}  // namespace orbit_linux_tracing_integration_tests
