// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/DataManager.h"

#include <absl/container/flat_hash_set.h>

#include <utility>

#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"

using orbit_client_data::TracepointInfoSet;
using orbit_grpc_protos::TracepointInfo;

namespace orbit_client_data {

void DataManager::SelectFunction(const FunctionInfo& function) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  if (!selected_functions_.contains(function) && function.IsFunctionSelectable()) {
    selected_functions_.insert(function);
  }
}

void DataManager::DeselectFunction(const FunctionInfo& function) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_functions_.erase(function);
}

void DataManager::ClearSelectedFunctions() {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_functions_.clear();
}

void DataManager::set_visible_scope_ids(absl::flat_hash_set<ScopeId> visible_scope_ids) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  visible_scope_ids_ = std::move(visible_scope_ids);
}

void DataManager::set_highlighted_scope_id(std::optional<ScopeId> highlighted_scope_id) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  highlighted_scope_id_ = highlighted_scope_id;
}

void DataManager::set_highlighted_group_id(uint64_t highlighted_group_id) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  highlighted_group_id_ = highlighted_group_id;
}

void DataManager::set_selected_thread_id(uint32_t thread_id) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_thread_id_ = thread_id;
}

void DataManager::set_selected_thread_state_slice(
    std::optional<ThreadStateSliceInfo> selected_thread_state_slice) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_thread_state_slice_ = selected_thread_state_slice;
}

void DataManager::set_hovered_thread_state_slice(
    std::optional<ThreadStateSliceInfo> hovered_thread_state_slice) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  hovered_thread_state_slice_ = hovered_thread_state_slice;
}

void DataManager::set_selected_timer(const orbit_client_protos::TimerInfo* timer_info) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_timer_ = timer_info;
}

bool DataManager::IsFunctionSelected(const FunctionInfo& function) const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_functions_.contains(function);
}

std::vector<FunctionInfo> DataManager::GetSelectedFunctions() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return {selected_functions_.begin(), selected_functions_.end()};
}

bool DataManager::IsScopeVisible(ScopeId scope_id) const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return visible_scope_ids_.contains(scope_id);
}

std::optional<ScopeId> DataManager::highlighted_scope_id() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return highlighted_scope_id_;
}

uint64_t DataManager::highlighted_group_id() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return highlighted_group_id_;
}

uint32_t DataManager::selected_thread_id() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_thread_id_;
}

std::optional<ThreadStateSliceInfo> DataManager::selected_thread_state_slice() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_thread_state_slice_;
}

std::optional<ThreadStateSliceInfo> DataManager::hovered_thread_state_slice() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return hovered_thread_state_slice_;
}

const orbit_client_protos::TimerInfo* DataManager::selected_timer() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_timer_;
}

void DataManager::SelectTracepoint(const TracepointInfo& info) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  if (!IsTracepointSelected(info)) {
    selected_tracepoints_.emplace(info);
  }
}

void DataManager::DeselectTracepoint(const TracepointInfo& info) {
  ORBIT_CHECK(IsTracepointSelected(info));
  selected_tracepoints_.erase(info);
}

bool DataManager::IsTracepointSelected(const TracepointInfo& info) const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_tracepoints_.contains(info);
}

const TracepointInfoSet& DataManager::selected_tracepoints() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_tracepoints_;
}

void DataManager::EnableFrameTrack(const FunctionInfo& function) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  user_defined_capture_data_.InsertFrameTrack(function);
}

void DataManager::DisableFrameTrack(const FunctionInfo& function) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  user_defined_capture_data_.EraseFrameTrack(function);
}

[[nodiscard]] bool DataManager::IsFrameTrackEnabled(const FunctionInfo& function) const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return user_defined_capture_data_.ContainsFrameTrack(function);
}

void DataManager::ClearUserDefinedCaptureData() {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  user_defined_capture_data_.Clear();
}

const UserDefinedCaptureData& DataManager::user_defined_capture_data() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return user_defined_capture_data_;
}

void DataManager::set_collect_scheduler_info(bool collect_scheduler_info) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  collect_scheduler_info_ = collect_scheduler_info;
}

bool DataManager::collect_scheduler_info() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return collect_scheduler_info_;
}

void DataManager::set_collect_thread_states(bool collect_thread_states) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  collect_thread_states_ = collect_thread_states;
}

bool DataManager::collect_thread_states() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return collect_thread_states_;
}

void DataManager::set_trace_gpu_submissions(bool trace_gpu_submissions) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  trace_gpu_submissions_ = trace_gpu_submissions;
}

bool DataManager::trace_gpu_submissions() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return trace_gpu_submissions_;
}

void DataManager::set_enable_api(bool enable_api) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  enable_api_ = enable_api;
}

bool DataManager::enable_api() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return enable_api_;
}

void DataManager::set_enable_introspection(bool enable_introspection) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  enable_introspection_ = enable_introspection;
}

bool DataManager::enable_introspection() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return enable_introspection_;
}

void DataManager::set_dynamic_instrumentation_method(
    orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod method) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  dynamic_instrumentation_method_ = method;
}

orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod
DataManager::dynamic_instrumentation_method() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return dynamic_instrumentation_method_;
}

void DataManager::set_wine_syscall_handling_method(WineSyscallHandlingMethod method) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  wine_syscall_handling_method_ = method;
}

WineSyscallHandlingMethod DataManager::wine_syscall_handling_method() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return wine_syscall_handling_method_;
}

void DataManager::set_samples_per_second(double samples_per_second) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  samples_per_second_ = samples_per_second;
}
double DataManager::samples_per_second() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return samples_per_second_;
}

void DataManager::set_stack_dump_size(uint16_t stack_dump_size) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  stack_dump_size_ = stack_dump_size;
}

uint16_t DataManager::stack_dump_size() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return stack_dump_size_;
}

void DataManager::set_thread_state_change_callstack_stack_dump_size(uint16_t stack_dump_size) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  thread_state_change_callstack_stack_dump_size_ = stack_dump_size;
}

uint16_t DataManager::thread_state_change_callstack_stack_dump_size() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return thread_state_change_callstack_stack_dump_size_;
}

void DataManager::set_unwinding_method(orbit_grpc_protos::CaptureOptions::UnwindingMethod method) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  unwinding_method_ = method;
}

orbit_grpc_protos::CaptureOptions::UnwindingMethod DataManager::unwinding_method() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return unwinding_method_;
}

void DataManager::set_max_local_marker_depth_per_command_buffer(
    uint64_t max_local_marker_depth_per_command_buffer) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  max_local_marker_depth_per_command_buffer_ = max_local_marker_depth_per_command_buffer;
}

uint64_t DataManager::max_local_marker_depth_per_command_buffer() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return max_local_marker_depth_per_command_buffer_;
}

void DataManager::set_enable_auto_frame_track(bool enable_auto_frame_track) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  enable_auto_frame_track_ = enable_auto_frame_track;
}

bool DataManager::enable_auto_frame_track() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return enable_auto_frame_track_;
}

void DataManager::set_collect_memory_info(bool collect_memory_info) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  collect_memory_info_ = collect_memory_info;
}

bool DataManager::collect_memory_info() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return collect_memory_info_;
}

void DataManager::set_memory_sampling_period_ms(uint64_t memory_sampling_period_ms) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  memory_sampling_period_ms_ = memory_sampling_period_ms;
}

uint64_t DataManager::memory_sampling_period_ms() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return memory_sampling_period_ms_;
}

void DataManager::set_memory_warning_threshold_kb(uint64_t memory_warning_threshold_kb) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  memory_warning_threshold_kb_ = memory_warning_threshold_kb;
}

uint64_t DataManager::memory_warning_threshold_kb() const {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  return memory_warning_threshold_kb_;
}

void DataManager::set_thread_state_change_callstack_collection(
    orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
        thread_state_change_callstack_collection) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);
  thread_state_change_callstack_collection_ = thread_state_change_callstack_collection;
}

orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
DataManager::thread_state_change_callstack_collection() const {
  return thread_state_change_callstack_collection_;
}

}  // namespace orbit_client_data
