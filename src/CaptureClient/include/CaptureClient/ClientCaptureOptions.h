// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_CLIENT_CAPTURE_OPTIONS_H_
#define CAPTURE_CLIENT_CLIENT_CAPTURE_OPTIONS_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>

#include "ClientData/FunctionInfo.h"
#include "ClientData/TracepointCustom.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {

struct ClientCaptureOptions {
  uint32_t process_id = 0;

  absl::flat_hash_map<uint64_t, orbit_client_data::FunctionInfo> selected_functions;
  absl::flat_hash_map<uint64_t, orbit_client_data::FunctionInfo>
      functions_to_record_additional_stack_on;
  orbit_client_data::TracepointInfoSet selected_tracepoints;
  std::map<uint64_t, uint64_t> absolute_address_to_size_of_functions_to_stop_unwinding_at;

  orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod dynamic_instrumentation_method =
      orbit_grpc_protos::CaptureOptions::CaptureOptions::kDynamicInstrumentationMethodUnspecified;

  orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method =
      orbit_grpc_protos::CaptureOptions::UnwindingMethod::CaptureOptions_UnwindingMethod_kUndefined;

  orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
      thread_state_change_callstack_collection =
          orbit_grpc_protos::CaptureOptions::kThreadStateChangeCallStackCollectionUnspecified;

  uint16_t stack_dump_size = 0;
  uint16_t thread_state_change_callstack_stack_dump_size = 0;
  uint64_t max_local_marker_depth_per_command_buffer = 0;
  uint64_t memory_sampling_period_ms = 0;
  double samples_per_second = 0;

  bool collect_gpu_jobs = false;
  bool collect_memory_info = false;
  bool collect_scheduling_info = false;
  bool collect_thread_states = false;
  bool enable_api = false;
  bool enable_introspection = false;
  bool record_arguments = false;
  bool record_return_values = false;
  bool enable_auto_frame_track = false;
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_CLIENT_CAPTURE_OPTIONS_H_