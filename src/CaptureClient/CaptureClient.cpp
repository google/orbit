// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/CaptureClient.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <stddef.h>

#include <algorithm>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ApiUtils/GetFunctionTableAddressPrefix.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/TracepointCustom.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "Introspection/Introspection.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_client {

using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::TracepointInfoSet;

using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::TracepointInfo;
using DynamicInstrumentationMethod =
    orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod;
using UnwindingMethod = orbit_grpc_protos::CaptureOptions::UnwindingMethod;

using orbit_base::Future;

namespace {

// Api functions are declared in Orbit.h. They are implemented in user code through the
// ORBIT_API_INSTANTIATE macro. Those functions are used to query the tracee for Orbit specific
// information, such as the memory location where Orbit should write function pointers to enable
// the Api after having injected liborbit.so.
std::vector<ApiFunction> FindApiFunctions(const orbit_client_data::ModuleManager& module_manager,
                                          const orbit_client_data::ProcessData& process_data) {
  // We have a different function name for each supported platform.
  static const std::vector<std::string> orbit_api_get_function_table_address_prefixes{
      orbit_api_utils::kOrbitApiGetFunctionTableAddressPrefix,
      orbit_api_utils::kOrbitApiGetFunctionTableAddressWinPrefix};
  std::vector<ApiFunction> api_functions;
  std::map<uint64_t, orbit_client_data::ModuleInMemory> modules_in_memory_map =
      process_data.GetMemoryMapCopy();
  for (const auto& [unused_start_address, module_in_memory] : modules_in_memory_map) {
    const ModuleData* module_data =
        module_manager.GetModuleByModuleIdentifier(module_in_memory.module_id());
    if (module_data == nullptr) {
      continue;
    }
    for (const std::string& orbit_api_get_function_table_address_prefix :
         orbit_api_get_function_table_address_prefixes) {
      for (size_t i = 0; i <= kOrbitApiVersion; ++i) {
        std::string function_name =
            absl::StrFormat("%s%u", orbit_api_get_function_table_address_prefix, i);
        const FunctionInfo* function_info = module_data->FindFunctionFromPrettyName(function_name);
        if (function_info == nullptr) {
          // Try both variants, with and without trailing parentheses, as the function name might or
          // might not have them depending on the symbol loading library.
          function_name.append("()");
          function_info = module_data->FindFunctionFromPrettyName(function_name);
        }
        if (function_info == nullptr) continue;

        const uint64_t absolute_address = orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
            function_info->address(), module_in_memory.start(), module_data->load_bias(),
            module_data->executable_segment_offset());

        ApiFunction api_function;
        api_function.set_module_path(function_info->module_path());
        api_function.set_module_build_id(function_info->module_build_id());
        api_function.set_relative_address(function_info->address());
        api_function.set_absolute_address(absolute_address);
        api_function.set_name(function_name);
        api_function.set_api_version(i);
        api_functions.emplace_back(api_function);
      }
    }
  }
  return api_functions;
}

[[nodiscard]] orbit_grpc_protos::CaptureOptions ToGrpcCaptureOptions(
    const ClientCaptureOptions& options, const orbit_client_data::ModuleManager& module_manager,
    const orbit_client_data::ProcessData& process_data) {
  CaptureOptions capture_options;
  capture_options.set_trace_context_switches(options.collect_scheduling_info);
  capture_options.set_pid(options.process_id);
  ORBIT_CHECK(options.unwinding_method != CaptureOptions::kUndefined);
  capture_options.set_unwinding_method(options.unwinding_method);
  capture_options.set_stack_dump_size(options.stack_dump_size);
  capture_options.set_thread_state_change_callstack_stack_dump_size(
      options.thread_state_change_callstack_stack_dump_size);
  capture_options.set_samples_per_second(options.samples_per_second);

  capture_options.set_collect_memory_info(options.collect_memory_info);
  constexpr const uint64_t kMsToNs = 1'000'000;
  capture_options.set_memory_sampling_period_ns(options.memory_sampling_period_ms * kMsToNs);

  capture_options.set_trace_thread_state(options.collect_thread_states);
  capture_options.set_trace_gpu_driver(options.collect_gpu_jobs);
  capture_options.set_max_local_marker_depth_per_command_buffer(
      options.max_local_marker_depth_per_command_buffer);

  for (const auto& [function_id, function] : options.selected_functions) {
    InstrumentedFunction* instrumented_function = capture_options.add_instrumented_functions();
    instrumented_function->set_file_path(function.module_path());
    const ModuleData* module = module_manager.GetModuleByModuleIdentifier(function.module_id());
    ORBIT_CHECK(module != nullptr);
    instrumented_function->set_file_offset(function.ComputeFileOffset(*module));
    instrumented_function->set_file_build_id(function.module_build_id());
    instrumented_function->set_function_id(function_id);
    instrumented_function->set_function_virtual_address(function.address());
    instrumented_function->set_function_size(function.size());
    instrumented_function->set_function_name(function.pretty_name());
    instrumented_function->set_is_hotpatchable(function.IsHotpatchable());
    instrumented_function->set_record_arguments(options.record_arguments);
    instrumented_function->set_record_return_value(options.record_return_values);
  }

  for (const auto& [function_id, function] : options.functions_to_record_additional_stack_on) {
    orbit_grpc_protos::FunctionToRecordAdditionalStackOn* function_to_record_stack_on =
        capture_options.add_functions_to_record_additional_stack_on();

    function_to_record_stack_on->set_file_path(function.module_path());
    const ModuleData* module = module_manager.GetModuleByModuleIdentifier(function.module_id());
    ORBIT_CHECK(module != nullptr);
    function_to_record_stack_on->set_file_offset(function.ComputeFileOffset(*module));
  }

  for (const auto& tracepoint : options.selected_tracepoints) {
    TracepointInfo* instrumented_tracepoint = capture_options.add_instrumented_tracepoint();
    instrumented_tracepoint->set_category(tracepoint.category());
    instrumented_tracepoint->set_name(tracepoint.name());
  }

  for (auto [absolute_address, size] :
       options.absolute_address_to_size_of_functions_to_stop_unwinding_at) {
    orbit_grpc_protos::FunctionToStopUnwindingAt* function_to_stop_unwinding_at =
        capture_options.add_functions_to_stop_unwinding_at();
    function_to_stop_unwinding_at->set_absolute_address(absolute_address);
    function_to_stop_unwinding_at->set_size(size);
  }

  capture_options.set_enable_api(options.enable_api);
  capture_options.set_enable_introspection(options.enable_introspection);
  ORBIT_CHECK(options.dynamic_instrumentation_method == CaptureOptions::kKernelUprobes ||
              options.dynamic_instrumentation_method == CaptureOptions::kUserSpaceInstrumentation);
  capture_options.set_dynamic_instrumentation_method(options.dynamic_instrumentation_method);

  auto api_functions = FindApiFunctions(module_manager, process_data);
  *(capture_options.mutable_api_functions()) = {api_functions.begin(), api_functions.end()};

  capture_options.set_thread_state_change_callstack_collection(
      options.thread_state_change_callstack_collection);

  return capture_options;
}

}  // namespace

orbit_base::Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> CaptureClient::Capture(
    orbit_base::ThreadPool* thread_pool,
    std::unique_ptr<CaptureEventProcessor> capture_event_processor,
    const orbit_client_data::ModuleManager& module_manager,
    const orbit_client_data::ProcessData& process_data,
    const ClientCaptureOptions& capture_options) {
  absl::MutexLock lock(&state_mutex_);
  if (state_ != State::kStopped) {
    return {
        ErrorMessage("Capture cannot be started, the previous capture is still "
                     "running/stopping.")};
  }

  state_ = State::kStarting;
  ORBIT_LOG("State is now kStarting");

  return thread_pool->Schedule([this, capture_event_processor = std::move(capture_event_processor),
                                grpc_capture_options = ToGrpcCaptureOptions(
                                    capture_options, module_manager, process_data)]() {
    return CaptureSync(grpc_capture_options, capture_event_processor.get());
  });
}

ErrorMessageOr<CaptureListener::CaptureOutcome> CaptureClient::CaptureSync(
    orbit_grpc_protos::CaptureOptions capture_options,
    CaptureEventProcessor* capture_event_processor) {
  ORBIT_SCOPE_FUNCTION;
  writes_done_failed_ = false;
  try_abort_ = false;
  {
    absl::WriterMutexLock lock{&context_and_stream_mutex_};
    ORBIT_CHECK(client_context_ == nullptr);
    ORBIT_CHECK(reader_writer_ == nullptr);
    client_context_ = std::make_unique<grpc::ClientContext>();
    reader_writer_ = capture_service_->Capture(client_context_.get());
  }

  CaptureRequest request;
  *request.mutable_capture_options() = std::move(capture_options);

  bool request_write_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    request_write_succeeded = reader_writer_->Write(request);
    if (!request_write_succeeded) {
      reader_writer_->WritesDone();
    }
  }
  if (!request_write_succeeded) {
    ORBIT_ERROR("Sending CaptureRequest on Capture's gRPC stream");
    ErrorMessageOr<void> finish_result = FinishCapture();
    std::string error_string =
        absl::StrFormat("Error sending capture request.%s",
                        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    return ErrorMessage{error_string};
  }
  ORBIT_LOG("Sent CaptureRequest on Capture's gRPC stream: asking to start capturing");

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
    ORBIT_LOG(
        "TryCancel on Capture's gRPC context was called: Read on Capture's gRPC stream failed");
    return CaptureListener::CaptureOutcome::kCancelled;
  }

  if (writes_done_failed_) {
    ORBIT_LOG(
        "WritesDone on Capture's gRPC stream failed: stop reading and try to finish the gRPC call");
    std::string error_string = absl::StrFormat(
        "Unable to finish the capture in orderly manner, performing emergency stop.%s",
        finish_result.has_error() ? ("\n" + finish_result.error().message()) : "");
    return ErrorMessage{error_string};
  }

  ORBIT_LOG("Finished reading from Capture's gRPC stream: all capture data has been received");
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
      ORBIT_LOG("StopCapture ignored, because it is starting and cannot be stopped at this stage.");
      return false;
    }

    if (state_ != State::kStarted) {
      ORBIT_LOG("StopCapture ignored, because it is already stopping or stopped");
      return false;
    }
    state_ = State::kStopping;
    ORBIT_LOG("State is now kStopping");
  }

  bool writes_done_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    ORBIT_CHECK(reader_writer_ != nullptr);
    writes_done_succeeded = reader_writer_->WritesDone();
  }
  if (!writes_done_succeeded) {
    // Normally the capture thread waits until service stops sending messages,
    // but in this case since we failed to notify the service we pull emergency
    // stop plug. Setting this flag forces capture thread to exit as soon
    // as it notices that it was set.
    ORBIT_ERROR(
        "WritesDone on Capture's gRPC stream failed: unable to finish the "
        "capture in orderly manner, initiating emergency stop");
    writes_done_failed_ = true;
  } else {
    ORBIT_LOG("Finished writing on Capture's gRPC stream: asking to stop capturing");
  }

  return true;
}

bool CaptureClient::AbortCaptureAndWait(int64_t max_wait_ms) {
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    if (client_context_ == nullptr) {
      ORBIT_LOG("AbortCaptureAndWait ignored: no ClientContext to TryCancel");
      return false;
    }
    ORBIT_LOG("Calling TryCancel on Capture's gRPC context: aborting the capture");
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
    ORBIT_CHECK(reader_writer_ != nullptr);
    status = reader_writer_->Finish();
    reader_writer_.reset();
    ORBIT_CHECK(client_context_ != nullptr);
    client_context_.reset();
  }

  {
    absl::MutexLock lock(&state_mutex_);
    state_ = State::kStopped;
    ORBIT_LOG("State is now kStopped");
  }

  if (!status.ok()) {
    ORBIT_ERROR("Finishing gRPC call to Capture: %s", status.error_message());
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
      ORBIT_LOG("State is now kStarted");
    }
  }
}

}  // namespace orbit_capture_client
