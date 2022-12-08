// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_DATA_MANAGER_H_
#define CLIENT_DATA_DATA_MANAGER_H_

#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/hash/hash.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <thread>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientData/TracepointCustom.h"
#include "ClientData/UserDefinedCaptureData.h"
#include "ClientData/WineSyscallHandlingMethod.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/ThreadConstants.h"

namespace orbit_client_data {

// This class is responsible for storing and navigating data on the client side.
// Note that every method of this class should be called on the main thread.
class DataManager final {
 public:
  explicit DataManager(std::thread::id thread_id = std::this_thread::get_id())
      : main_thread_id_(thread_id) {}

  void SelectFunction(const FunctionInfo& function);
  void DeselectFunction(const FunctionInfo& function);
  void ClearSelectedFunctions();
  void set_visible_scope_ids(absl::flat_hash_set<ScopeId> visible_scope_ids);
  void set_highlighted_scope_id(std::optional<ScopeId> highlighted_scope_id);
  void set_highlighted_group_id(uint64_t highlighted_group_id);
  void set_selected_thread_id(uint32_t thread_id);
  void set_selected_thread_state_slice(
      std::optional<ThreadStateSliceInfo> selected_thread_state_slice);
  void set_hovered_thread_state_slice(
      std::optional<ThreadStateSliceInfo> hovered_thread_state_slice);
  void set_selected_timer(const orbit_client_protos::TimerInfo* timer_info);

  [[nodiscard]] bool IsFunctionSelected(const FunctionInfo& function) const;
  [[nodiscard]] std::vector<FunctionInfo> GetSelectedFunctions() const;
  [[nodiscard]] bool IsScopeVisible(ScopeId scope_id) const;
  [[nodiscard]] std::optional<ScopeId> highlighted_scope_id() const;
  [[nodiscard]] uint64_t highlighted_group_id() const;
  [[nodiscard]] uint32_t selected_thread_id() const;
  [[nodiscard]] std::optional<ThreadStateSliceInfo> selected_thread_state_slice() const;
  [[nodiscard]] std::optional<ThreadStateSliceInfo> hovered_thread_state_slice() const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* selected_timer() const;

  void SelectTracepoint(const orbit_grpc_protos::TracepointInfo& info);
  void DeselectTracepoint(const orbit_grpc_protos::TracepointInfo& info);

  [[nodiscard]] bool IsTracepointSelected(const orbit_grpc_protos::TracepointInfo& info) const;
  [[nodiscard]] const TracepointInfoSet& selected_tracepoints() const;

  void EnableFrameTrack(const FunctionInfo& function);
  void DisableFrameTrack(const FunctionInfo& function);
  [[nodiscard]] bool IsFrameTrackEnabled(const FunctionInfo& function) const;
  void ClearUserDefinedCaptureData();

  [[nodiscard]] const UserDefinedCaptureData& user_defined_capture_data() const;

  void set_collect_scheduler_info(bool collect_scheduler_info);
  [[nodiscard]] bool collect_scheduler_info() const;

  void set_collect_thread_states(bool collect_thread_states);
  [[nodiscard]] bool collect_thread_states() const;

  void set_trace_gpu_submissions(bool trace_gpu_submissions);
  [[nodiscard]] bool trace_gpu_submissions() const;

  void set_enable_api(bool enable_api);
  [[nodiscard]] bool enable_api() const;

  void set_enable_introspection(bool enable_introspection);
  [[nodiscard]] bool enable_introspection() const;

  void set_thread_state_change_callstack_collection(
      orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
          thread_state_change_callstack_collection);
  [[nodiscard]] orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
  thread_state_change_callstack_collection() const;

  void set_dynamic_instrumentation_method(
      orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod method);
  [[nodiscard]] orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod
  dynamic_instrumentation_method() const;

  void set_samples_per_second(double samples_per_second);
  [[nodiscard]] double samples_per_second() const;

  void set_stack_dump_size(uint16_t stack_dump_size);
  [[nodiscard]] uint16_t stack_dump_size() const;

  void set_thread_state_change_callstack_stack_dump_size(uint16_t stack_dump_size);
  [[nodiscard]] uint16_t thread_state_change_callstack_stack_dump_size() const;

  void set_unwinding_method(orbit_grpc_protos::CaptureOptions::UnwindingMethod method);
  [[nodiscard]] orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method() const;

  void set_wine_syscall_handling_method(WineSyscallHandlingMethod method);
  [[nodiscard]] WineSyscallHandlingMethod wine_syscall_handling_method() const;

  void set_max_local_marker_depth_per_command_buffer(
      uint64_t max_local_marker_depth_per_command_buffer);
  [[nodiscard]] uint64_t max_local_marker_depth_per_command_buffer() const;

  void set_enable_auto_frame_track(bool enable_auto_frame_track);
  [[nodiscard]] bool enable_auto_frame_track() const;

  void set_collect_memory_info(bool collect_memory_info);
  [[nodiscard]] bool collect_memory_info() const;

  void set_memory_sampling_period_ms(uint64_t memory_sampling_period_ms);
  [[nodiscard]] uint64_t memory_sampling_period_ms() const;

  void set_memory_warning_threshold_kb(uint64_t memory_warning_threshold_kb);
  [[nodiscard]] uint64_t memory_warning_threshold_kb() const;

 private:
  const std::thread::id main_thread_id_;
  absl::flat_hash_set<FunctionInfo> selected_functions_;
  absl::flat_hash_set<ScopeId> visible_scope_ids_;
  std::optional<ScopeId> highlighted_scope_id_ = std::nullopt;
  uint64_t highlighted_group_id_ = kOrbitDefaultGroupId;

  TracepointInfoSet selected_tracepoints_;

  uint32_t selected_thread_id_ = orbit_base::kInvalidThreadId;
  const orbit_client_protos::TimerInfo* selected_timer_ = nullptr;
  std::optional<orbit_client_data::ThreadStateSliceInfo> selected_thread_state_slice_;
  std::optional<orbit_client_data::ThreadStateSliceInfo> hovered_thread_state_slice_;

  // DataManager needs a copy of this so that we can persist user choices like frame tracks between
  // captures.
  UserDefinedCaptureData user_defined_capture_data_;

  bool collect_scheduler_info_ = false;
  bool collect_thread_states_ = false;
  bool trace_gpu_submissions_ = false;
  bool enable_api_ = false;
  bool enable_introspection_ = false;
  orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod dynamic_instrumentation_method_{};
  orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
      thread_state_change_callstack_collection_{};
  WineSyscallHandlingMethod wine_syscall_handling_method_{};
  uint64_t max_local_marker_depth_per_command_buffer_ = std::numeric_limits<uint64_t>::max();
  double samples_per_second_ = 0;
  uint16_t stack_dump_size_ = 0;
  uint16_t thread_state_change_callstack_stack_dump_size_ = 0;
  orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method_{};

  bool enable_auto_frame_track_ = false;
  bool collect_memory_info_ = false;
  uint64_t memory_sampling_period_ms_ = 10;
  uint64_t memory_warning_threshold_kb_ = 8ULL * 1024 * 1024;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_DATA_MANAGER_H_
