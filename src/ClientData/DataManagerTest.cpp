// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gtest/gtest.h>

#include <exception>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include "ClientData/DataManager.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/WineSyscallHandlingMethod.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/tracepoint.pb.h"

namespace orbit_client_data {

template <typename Method, typename... Args>
static void CallMethodOnDifferentThreadAndExpectDeath(DataManager& data_manager, Method method,
                                                      Args&&... args) {
  EXPECT_DEATH(std::thread{[&]() { (data_manager.*method)(std::forward<Args>(args)...); }}.join(),
               "Check failed");
}

TEST(DataManager, CanOnlyBeUsedFromTheMainThread) {
  DataManager data_manager;
  FunctionInfo function{"path/to/module", "buildid", /*address=*/12,
                        /*size=*/16,      "foo()",   /*is_hotpatchable=*/false};
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::SelectFunction, function);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::DeselectFunction, function);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::SelectFunction, function);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::ClearSelectedFunctions);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_visible_scope_ids,
                                            absl::flat_hash_set<ScopeId>{});
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_highlighted_scope_id,
                                            ScopeId(0));
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_highlighted_group_id,
                                            0);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_selected_thread_id, 0);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::set_selected_thread_state_slice, std::nullopt);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::set_hovered_thread_state_slice, std::nullopt);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_selected_timer,
                                            nullptr);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::SelectTracepoint,
                                            orbit_grpc_protos::TracepointInfo{});
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::DeselectTracepoint,
                                            orbit_grpc_protos::TracepointInfo{});
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::IsTracepointSelected,
                                            orbit_grpc_protos::TracepointInfo{});
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::selected_tracepoints);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::EnableFrameTrack, function);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::DisableFrameTrack,
                                            function);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::IsFrameTrackEnabled,
                                            function);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::ClearUserDefinedCaptureData);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::user_defined_capture_data);

  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_collect_scheduler_info,
                                            false);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::collect_scheduler_info);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_collect_thread_states,
                                            false);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::collect_thread_states);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_trace_gpu_submissions,
                                            false);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::trace_gpu_submissions);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_enable_api, false);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::enable_api);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_enable_introspection,
                                            false);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::enable_introspection);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::set_dynamic_instrumentation_method,
      orbit_grpc_protos::CaptureOptions::kDynamicInstrumentationMethodUnspecified);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::dynamic_instrumentation_method);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_samples_per_second,
                                            0.0);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::samples_per_second);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_stack_dump_size, 0);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::stack_dump_size);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::set_thread_state_change_callstack_stack_dump_size, 0);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::thread_state_change_callstack_stack_dump_size);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_unwinding_method,
                                            orbit_grpc_protos::CaptureOptions::kUndefined);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::unwinding_method);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::set_max_local_marker_depth_per_command_buffer, 0);
  CallMethodOnDifferentThreadAndExpectDeath(
      data_manager, &DataManager::max_local_marker_depth_per_command_buffer);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::set_collect_memory_info,
                                            false);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::collect_memory_info);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::set_memory_sampling_period_ms, 0);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager, &DataManager::memory_sampling_period_ms);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::set_memory_warning_threshold_kb, 0);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::memory_warning_threshold_kb);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::set_wine_syscall_handling_method,
                                            WineSyscallHandlingMethod::kNoSpecialHandling);
  CallMethodOnDifferentThreadAndExpectDeath(data_manager,
                                            &DataManager::wine_syscall_handling_method);
}

}  // namespace orbit_client_data
