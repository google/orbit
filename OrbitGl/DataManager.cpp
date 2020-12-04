// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataManager.h"

#include "OrbitBase/Logging.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ProcessData.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

using orbit_client_protos::FunctionInfo;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::TracepointInfo;

void DataManager::UpdateProcessInfos(const std::vector<ProcessInfo>& process_infos) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  // Note that at this point the data manager does not remove old processes.
  // To do it correctly we may need to implement some callback logic here
  // since the ProcessData can be in use by some views.
  for (const ProcessInfo& info : process_infos) {
    int32_t process_id = info.pid();
    auto it = process_map_.find(process_id);
    if (it != process_map_.end()) {
      it->second.SetProcessInfo(info);
    } else {
      auto [inserted_it, success] = process_map_.try_emplace(process_id, info);
      CHECK(success);
    }
  }
}

void DataManager::SelectFunction(const FunctionInfo& function) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  if (!selected_functions_.contains(function)) {
    selected_functions_.insert(function);
  }
}

void DataManager::DeselectFunction(const FunctionInfo& function) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_functions_.erase(function);
}

void DataManager::set_visible_functions(absl::flat_hash_set<uint64_t> visible_functions) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  visible_functions_ = std::move(visible_functions);
}

void DataManager::set_highlighted_function(uint64_t highlighted_function_address) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  highlighted_function_ = highlighted_function_address;
}

void DataManager::set_selected_thread_id(int32_t thread_id) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_thread_id_ = thread_id;
}

bool DataManager::IsFunctionVisible(uint64_t function_address) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return visible_functions_.contains(function_address);
}

const uint64_t DataManager::kInvalidFunctionAddress = std::numeric_limits<uint64_t>::max();

uint64_t DataManager::highlighted_function() const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return highlighted_function_;
}

int32_t DataManager::selected_thread_id() const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_thread_id_;
}

const TextBox* DataManager::selected_text_box() const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_text_box_;
}

void DataManager::set_selected_text_box(const TextBox* text_box) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_text_box_ = text_box;
}

void DataManager::ClearSelectedFunctions() {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_functions_.clear();
}

ProcessData* DataManager::GetMutableProcessByPid(int32_t process_id) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  if (it == process_map_.end()) return nullptr;

  return &it->second;
}

const ProcessData* DataManager::GetProcessByPid(int32_t process_id) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  if (it == process_map_.end()) return nullptr;

  return &it->second;
}

bool DataManager::IsFunctionSelected(const FunctionInfo& function) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_functions_.contains(function);
}

std::vector<FunctionInfo> DataManager::GetSelectedFunctions() const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return std::vector<FunctionInfo>(selected_functions_.begin(), selected_functions_.end());
}

void DataManager::set_selected_process(int32_t pid) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  const ProcessData* process = GetProcessByPid(pid);
  CHECK(process != nullptr);
  selected_process_ = process;
}

const ProcessData* DataManager::selected_process() const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_process_;
}

void DataManager::SelectTracepoint(const TracepointInfo& info) {
  if (!IsTracepointSelected(info)) selected_tracepoints_.emplace(info);
}

void DataManager::DeselectTracepoint(const TracepointInfo& info) {
  CHECK(IsTracepointSelected(info));
  selected_tracepoints_.erase(info);
}

bool DataManager::IsTracepointSelected(const TracepointInfo& info) const {
  return selected_tracepoints_.contains(info);
}

const TracepointInfoSet& DataManager::selected_tracepoints() const { return selected_tracepoints_; }

void DataManager::EnableFrameTrack(const FunctionInfo& function) {
  user_defined_capture_data_.InsertFrameTrack(function);
}

void DataManager::DisableFrameTrack(const FunctionInfo& function) {
  user_defined_capture_data_.EraseFrameTrack(function);
}

[[nodiscard]] bool DataManager::IsFrameTrackEnabled(const FunctionInfo& function) const {
  return user_defined_capture_data_.ContainsFrameTrack(function);
}

void DataManager::ClearUserDefinedCaptureData() { user_defined_capture_data_.Clear(); }
