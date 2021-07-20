// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/CaptureClient.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <cstdint>
#include <outcome.hpp>
#include <string>
#include <type_traits>
#include <utility>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/FunctionUtils.h"
#include "ClientData/ModuleData.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "capture.pb.h"
#include "tracepoint.pb.h"

namespace orbit_capture_client {

using orbit_client_data::ModuleData;
using orbit_client_data::TracepointInfoSet;

using orbit_client_protos::FunctionInfo;

using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::TracepointInfo;
using orbit_grpc_protos::UnwindingMethod;

using orbit_base::Future;

InstrumentedFunction::FunctionType CaptureClient::InstrumentedFunctionTypeFromOrbitType(
    FunctionInfo::OrbitType orbit_type) {
  switch (orbit_type) {
    case FunctionInfo::kOrbitTimerStart:
      return InstrumentedFunction::kTimerStart;
    case FunctionInfo::kOrbitTimerStop:
      return InstrumentedFunction::kTimerStop;
    case FunctionInfo::kOrbitTimerStartAsync:
    case FunctionInfo::kOrbitTimerStopAsync:
    case FunctionInfo::kOrbitTrackValue:
    case FunctionInfo::kNone:
      return InstrumentedFunction::kRegular;
    case orbit_client_protos::
        FunctionInfo_OrbitType_FunctionInfo_OrbitType_INT_MIN_SENTINEL_DO_NOT_USE_:
    case orbit_client_protos::
        FunctionInfo_OrbitType_FunctionInfo_OrbitType_INT_MAX_SENTINEL_DO_NOT_USE_:
      UNREACHABLE();
  }
  UNREACHABLE();
}

// TODO(b/187170164): This method contains a lot of arguments. Consider making it more structured.
Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> CaptureClient::Capture(
    ThreadPool* thread_pool, int32_t process_id,
    const orbit_client_data::ModuleManager& module_manager,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions, bool always_record_arguments,
    bool record_return_values, TracepointInfoSet selected_tracepoints, double samples_per_second,
    uint16_t stack_dump_size, UnwindingMethod unwinding_method, bool collect_scheduling_info,
    bool collect_thread_state, bool collect_gpu_jobs, bool enable_api, bool enable_introspection,
    bool enable_user_space_instrumentation, uint64_t max_local_marker_depth_per_command_buffer,
    bool collect_memory_info, uint64_t memory_sampling_period_ms,
    std::unique_ptr<CaptureEventProcessor> capture_event_processor) {
  absl::MutexLock lock(&state_mutex_);
  if (state_ != State::kStopped) {
    return {
        ErrorMessage("Capture cannot be started, the previous capture is still "
                     "running/stopping.")};
  }

  state_ = State::kStarting;
  LOG("State is now kStarting");

  auto capture_result = thread_pool->Schedule(
      [this, process_id, &module_manager, selected_functions = std::move(selected_functions),
       always_record_arguments, record_return_values,
       selected_tracepoints = std::move(selected_tracepoints), samples_per_second, stack_dump_size,
       unwinding_method, collect_scheduling_info, collect_thread_state, collect_gpu_jobs,
       enable_api, enable_introspection, enable_user_space_instrumentation,
       max_local_marker_depth_per_command_buffer, collect_memory_info, memory_sampling_period_ms,
       capture_event_processor = std::move(capture_event_processor)]() mutable {
        return CaptureSync(process_id, module_manager, selected_functions, always_record_arguments,
                           record_return_values, selected_tracepoints, samples_per_second,
                           stack_dump_size, unwinding_method, collect_scheduling_info,
                           collect_thread_state, collect_gpu_jobs, enable_api, enable_introspection,
                           enable_user_space_instrumentation,
                           max_local_marker_depth_per_command_buffer, collect_memory_info,
                           memory_sampling_period_ms, capture_event_processor.get());
      });

  return capture_result;
}

// Api functions are declared in Orbit.h. They are implemented in user code through the
// ORBIT_API_INSTANTIATE macro. Those functions are used to query the tracee for Orbit specific
// information, such as the memory location where Orbit should write function pointers to enable
// the Api after having injected liborbit.so.
[[nodiscard]] static std::vector<ApiFunction> FindApiFunctions(
    const orbit_client_data::ModuleManager& module_manager) {
  std::vector<ApiFunction> api_functions;
  for (const ModuleData* module_data : module_manager.GetAllModuleData()) {
    constexpr const char* kOrbitApiGetAddressPrefix = "orbit_api_get_function_table_address_v";
    for (size_t i = 0; i <= kOrbitApiVersion; ++i) {
      std::string function_name = absl::StrFormat("%s%u", kOrbitApiGetAddressPrefix, i);
      const FunctionInfo* function_info = module_data->FindFunctionFromPrettyName(function_name);
      if (function_info == nullptr) continue;
      ApiFunction api_function;
      api_function.set_module_path(function_info->module_path());
      api_function.set_module_build_id(function_info->module_build_id());
      api_function.set_address(function_info->address());
      api_function.set_name(function_name);
      api_function.set_api_version(i);
      api_functions.emplace_back(api_function);
    }
  }
  return api_functions;
}

ErrorMessageOr<CaptureListener::CaptureOutcome> CaptureClient::CaptureSync(
    int32_t process_id, const orbit_client_data::ModuleManager& module_manager,
    const absl::flat_hash_map<uint64_t, FunctionInfo>& selected_functions,
    bool always_record_arguments, bool record_return_values,
    const TracepointInfoSet& selected_tracepoints, double samples_per_second,
    uint16_t stack_dump_size, UnwindingMethod unwinding_method, bool collect_scheduling_info,
    bool collect_thread_state, bool collect_gpu_jobs, bool enable_api, bool enable_introspection,
    bool enable_user_space_instrumentation, uint64_t max_local_marker_depth_per_command_buffer,
    bool collect_memory_info, uint64_t memory_sampling_period_ms,
    CaptureEventProcessor* capture_event_processor) {
  ORBIT_SCOPE_FUNCTION;
  writes_done_failed_ = false;
  try_abort_ = false;
  {
    absl::WriterMutexLock lock{&context_and_stream_mutex_};
    CHECK(client_context_ == nullptr);
    CHECK(reader_writer_ == nullptr);
    client_context_ = std::make_unique<grpc::ClientContext>();
    reader_writer_ = capture_service_->Capture(client_context_.get());
  }

  CaptureRequest request;
  CaptureOptions* capture_options = request.mutable_capture_options();
  capture_options->set_trace_context_switches(collect_scheduling_info);
  capture_options->set_pid(process_id);
  if (samples_per_second == 0) {
    capture_options->set_unwinding_method(CaptureOptions::kUndefined);
  } else {
    capture_options->set_samples_per_second(samples_per_second);
    capture_options->set_stack_dump_size(stack_dump_size);
    if (unwinding_method == UnwindingMethod::kFramePointerUnwinding) {
      capture_options->set_unwinding_method(CaptureOptions::kFramePointers);
    } else {
      capture_options->set_unwinding_method(CaptureOptions::kDwarf);
    }
  }

  capture_options->set_collect_memory_info(collect_memory_info);
  constexpr const uint64_t kMsToNs = 1'000'000;
  capture_options->set_memory_sampling_period_ns(memory_sampling_period_ms * kMsToNs);

  capture_options->set_trace_thread_state(collect_thread_state);
  capture_options->set_trace_gpu_driver(collect_gpu_jobs);
  capture_options->set_max_local_marker_depth_per_command_buffer(
      max_local_marker_depth_per_command_buffer);
  absl::flat_hash_map<uint64_t, InstrumentedFunction> instrumented_functions;
  for (const auto& [function_id, function] : selected_functions) {
    InstrumentedFunction* instrumented_function = capture_options->add_instrumented_functions();
    instrumented_function->set_file_path(function.module_path());
    const ModuleData* module = module_manager.GetModuleByPathAndBuildId(function.module_path(),
                                                                        function.module_build_id());
    CHECK(module != nullptr);
    instrumented_function->set_file_offset(
        orbit_client_data::function_utils::Offset(function, *module));
    instrumented_function->set_file_build_id(function.module_build_id());
    instrumented_function->set_function_id(function_id);
    instrumented_function->set_function_size(function.size());
    instrumented_function->set_function_name(function.pretty_name());
    instrumented_function->set_function_type(
        InstrumentedFunctionTypeFromOrbitType(function.orbit_type()));
    instrumented_function->set_record_arguments(always_record_arguments ||
                                                (function.orbit_type() != FunctionInfo::kNone));
    instrumented_function->set_record_return_value(record_return_values);
    instrumented_functions.insert_or_assign(function_id, *instrumented_function);
  }

  for (const auto& tracepoint : selected_tracepoints) {
    TracepointInfo* instrumented_tracepoint = capture_options->add_instrumented_tracepoint();
    instrumented_tracepoint->set_category(tracepoint.category());
    instrumented_tracepoint->set_name(tracepoint.name());
  }

  capture_options->set_enable_api(enable_api);
  capture_options->set_enable_introspection(enable_introspection);
  capture_options->set_enable_user_space_instrumentation(enable_user_space_instrumentation);

  auto api_functions = FindApiFunctions(module_manager);
  *(capture_options->mutable_api_functions()) = {api_functions.begin(), api_functions.end()};

  bool request_write_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    request_write_succeeded = reader_writer_->Write(request);
    if (!request_write_succeeded) {
      reader_writer_->WritesDone();
    }
  }
  if (!request_write_succeeded) {
    ERROR("Sending CaptureRequest on Capture's gRPC stream");
    ErrorMessageOr<void> finish_result = FinishCapture();
    std::string error_string =
        absl::StrFormat("Error sending capture request.%s",
                        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    return ErrorMessage{error_string};
  }
  LOG("Sent CaptureRequest on Capture's gRPC stream: asking to start capturing");

  while (!writes_done_failed_ && !try_abort_) {
    CaptureResponse response;
    bool read_succeeded;
    {
      absl::ReaderMutexLock lock{&context_and_stream_mutex_};
      read_succeeded = reader_writer_->Read(&response);
    }
    if (read_succeeded) {
      ProcessEvents(capture_event_processor, response.capture_events());
    } else {
      break;
    }
  }

  ErrorMessageOr<void> finish_result = FinishCapture();
  if (try_abort_) {
    LOG("TryCancel on Capture's gRPC context was called: Read on Capture's gRPC stream failed");
    return CaptureListener::CaptureOutcome::kCancelled;
  }

  if (writes_done_failed_) {
    LOG("WritesDone on Capture's gRPC stream failed: stop reading and try to finish the gRPC call");
    std::string error_string = absl::StrFormat(
        "Unable to finish the capture in orderly manner, performing emergency stop.%s",
        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    return ErrorMessage{error_string};
  }

  LOG("Finished reading from Capture's gRPC stream: all capture data has been received");
  if (finish_result.has_error()) {
    return ErrorMessage{absl::StrFormat(
        "Unable to finish the capture in an orderly manner. The following error occurred: %s",
        finish_result.error().message())};
  }
  return CaptureListener::CaptureOutcome::kComplete;
}

bool CaptureClient::StopCapture() {
  {
    absl::MutexLock lock(&state_mutex_);
    if (state_ == State::kStarting) {
      LOG("StopCapture ignored, because it is starting and cannot be stopped at this stage.");
      return false;
    }

    if (state_ != State::kStarted) {
      LOG("StopCapture ignored, because it is already stopping or stopped");
      return false;
    }
    state_ = State::kStopping;
    LOG("State is now kStopping");
  }

  bool writes_done_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    CHECK(reader_writer_ != nullptr);
    writes_done_succeeded = reader_writer_->WritesDone();
  }
  if (!writes_done_succeeded) {
    // Normally the capture thread waits until service stops sending messages,
    // but in this case since we failed to notify the service we pull emergency
    // stop plug. Setting this flag forces capture thread to exit as soon
    // as it notices that it was set.
    ERROR(
        "WritesDone on Capture's gRPC stream failed: unable to finish the "
        "capture in orderly manner, initiating emergency stop");
    writes_done_failed_ = true;
  } else {
    LOG("Finished writing on Capture's gRPC stream: asking to stop capturing");
  }

  return true;
}

bool CaptureClient::AbortCaptureAndWait(int64_t max_wait_ms) {
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    if (client_context_ == nullptr) {
      LOG("AbortCaptureAndWait ignored: no ClientContext to TryCancel");
      return false;
    }
    LOG("Calling TryCancel on Capture's gRPC context: aborting the capture");
    try_abort_ = true;
    client_context_->TryCancel();  // reader_writer_->Read in Capture should then fail
  }

  // With this wait we want to leave at least some time for FinishCapture to be called, so that
  // reader_writer_ and in particular client_context_ are destroyed before returning to the caller.
  {
    absl::MutexLock lock(&state_mutex_);
    state_mutex_.AwaitWithTimeout(
        absl::Condition(
            +[](State* state) { return *state == State::kStopped; }, &state_),
        absl::Milliseconds(max_wait_ms));
  }
  return true;
}

ErrorMessageOr<void> CaptureClient::FinishCapture() {
  ORBIT_SCOPE_FUNCTION;

  grpc::Status status;
  {
    absl::WriterMutexLock lock{&context_and_stream_mutex_};
    CHECK(reader_writer_ != nullptr);
    status = reader_writer_->Finish();
    reader_writer_.reset();
    CHECK(client_context_ != nullptr);
    client_context_.reset();
  }

  {
    absl::MutexLock lock(&state_mutex_);
    state_ = State::kStopped;
    LOG("State is now kStopped");
  }

  if (!status.ok()) {
    ERROR("Finishing gRPC call to Capture: %s", status.error_message());
    return ErrorMessage{status.error_message()};
  }
  return outcome::success();
}

void CaptureClient::ProcessEvents(
    CaptureEventProcessor* capture_event_processor,
    const google::protobuf::RepeatedPtrField<ClientCaptureEvent>& events) {
  for (const auto& event : events) {
    capture_event_processor->ProcessEvent(event);
    if (event.event_case() == ClientCaptureEvent::kCaptureStarted) {
      absl::MutexLock lock{&state_mutex_};
      state_ = State::kStarted;
      LOG("State is now kStarted");
    }
  }
}

}  // namespace orbit_capture_client
