// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PROCESS_DATA_H_
#define ORBIT_GL_PROCESS_DATA_H_

#include "ModuleData.h"
#include "OrbitBase/Logging.h"
#include "absl/container/flat_hash_map.h"
#include "module.pb.h"
#include "process.pb.h"

// Contains current information about process
class ProcessData final {
 public:
  ProcessData(const ProcessData&) = delete;
  ProcessData& operator=(const ProcessData&) = delete;
  ProcessData(ProcessData&&) = default;
  ProcessData& operator=(ProcessData&&) = default;

  explicit ProcessData(const ProcessInfo& process_info)
      : process_info_(process_info) {}

  void SetProcessInfo(const ProcessInfo& process_info) {
    process_info_ = process_info;
  }

  void UpdateModuleInfos(const std::vector<ModuleInfo> module_infos) {
    current_module_list_.clear();
    for (const ModuleInfo& info : module_infos) {
      uint64_t module_id = info.address_start();
      auto it = modules_.find(module_id);
      if (it != modules_.end()) {
        it->second->SetModuleInfo(info);
        current_module_list_.push_back(it->second.get());
      } else {
        auto [inserted_it, success] =
            modules_.try_emplace(module_id, std::make_unique<ModuleData>(info));
        CHECK(success);
        current_module_list_.push_back(inserted_it->second.get());
      }
    }
  }

  const std::vector<ModuleData*> GetModules() const {
    return current_module_list_;
  }

 private:
  ProcessInfo process_info_;

  // This list omits unloaded modules, note that they are still stored
  // in the map.
  std::vector<ModuleData*> current_module_list_;
  absl::flat_hash_map<uint64_t, std::unique_ptr<ModuleData>> modules_;
};

#endif  // ORBIT_GL_PROCESS_DATA_H_
