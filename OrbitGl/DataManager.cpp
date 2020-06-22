// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataManager.h"

#include "OrbitBase/Logging.h"

void DataManager::UpdateProcessInfos(
    const std::vector<ProcessInfo>& process_infos) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  // Note that at this point the data manager does not remove old processes
  // to do it correctly we may need to implement some callback logic here
  // since the ProcessData can be in use by some views.
  for (const ProcessInfo& info : process_infos) {
    uint32_t process_id = info.pid();
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
    uint32_t process_id, const std::vector<ModuleInfo>& module_infos) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  CHECK(it != process_map_.end());

  it->second->UpdateModuleInfos(module_infos);
}

const std::vector<ModuleData*> DataManager::GetModules(uint32_t process_id) {
  CHECK(std::this_thread::get_id() == main_thread_id_);

  auto it = process_map_.find(process_id);
  CHECK(it != process_map_.end());

  return it->second->GetModules();
}
