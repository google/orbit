// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataManager.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

void DataManager::UpdateProcessInfos(
    const std::vector<ProcessInfo>& process_infos) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  // Note that at this point the data manager does not remove old processes.
  // To do it correctly we may need to implement some callback logic here
  // since the ProcessData can be in use by some views.
  for (const ProcessInfo& info : process_infos) {
    int32_t process_id = info.pid();
    auto it = process_map_.find(process_id);
    if (it != process_map_.end()) {
      it->second->SetProcessInfo(info);
    } else {
      auto [inserted_it, success] = process_map_.try_emplace(
          process_id, std::make_unique<ProcessData>(info));
      CHECK(success);
    }
  }
}

void DataManager::UpdateModuleInfos(
    int32_t process_id, const std::vector<ModuleInfo>& module_infos) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  CHECK(it != process_map_.end());

  it->second->UpdateModuleInfos(module_infos);
}

void DataManager::SelectFunction(uint64_t function_address) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  CHECK(!selected_functions_.contains(function_address));
  selected_functions_.insert(function_address);
}

void DataManager::DeselectFunction(uint64_t function_address) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  CHECK(selected_functions_.contains(function_address));
  selected_functions_.erase(function_address);
}

void DataManager::set_selected_functions(
    absl::flat_hash_set<uint64_t> selected_functions) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_functions_ = std::move(selected_functions);
}

void DataManager::ClearSelectedFunctions() {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  selected_functions_ = absl::flat_hash_set<uint64_t>();
}

ProcessData* DataManager::GetProcessByPid(int32_t process_id) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  if (it == process_map_.end()) {
    return nullptr;
  }

  return it->second.get();
}

const std::vector<ModuleData*>& DataManager::GetModules(
    int32_t process_id) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  CHECK(it != process_map_.end());

  return it->second->GetModules();
}

ModuleData* DataManager::FindModuleByAddressStart(
    int32_t process_id, uint64_t address_start) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  CHECK(it != process_map_.end());

  return it->second->FindModuleByAddressStart(address_start);
}

bool DataManager::IsFunctionSelected(uint64_t function_address) const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_functions_.contains(function_address);
}

const absl::flat_hash_set<uint64_t>& DataManager::selected_functions() const {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  return selected_functions_;
}
