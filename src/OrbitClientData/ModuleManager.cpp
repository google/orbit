// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/ModuleManager.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <absl/meta/type_traits.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitClientData/ModuleData.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"

using orbit_client_protos::FunctionInfo;

namespace orbit_client_data {

std::vector<ModuleData*> ModuleManager::AddOrUpdateModules(
    absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  absl::MutexLock lock(&mutex_);

  std::vector<ModuleData*> unloaded_modules;

  for (const auto& module_info : module_infos) {
    auto module_id = std::make_pair(module_info.file_path(), module_info.build_id());
    auto module_it = module_map_.find(module_id);
    if (module_it == module_map_.end()) {
      const bool success = module_map_.try_emplace(module_id, module_info).second;
      CHECK(success);
    } else {
      ModuleData& module = module_it->second;
      if (module.UpdateIfChangedAndUnload(module_info)) {
        unloaded_modules.push_back(&module);
      }
    }
  }

  return unloaded_modules;
}

const ModuleData* ModuleManager::GetModuleByPathAndBuildId(const std::string& path,
                                                           const std::string& build_id) const {
  absl::MutexLock lock(&mutex_);

  auto it = module_map_.find(std::make_pair(path, build_id));
  if (it == module_map_.end()) return nullptr;

  return &it->second;
}

ModuleData* ModuleManager::GetMutableModuleByPathAndBuildId(const std::string& path,
                                                            const std::string& build_id) {
  absl::MutexLock lock(&mutex_);

  auto it = module_map_.find(std::make_pair(path, build_id));
  if (it == module_map_.end()) return nullptr;

  return &it->second;
}

std::vector<FunctionInfo> ModuleManager::GetOrbitFunctionsOfProcess(
    const ProcessData& process) const {
  absl::MutexLock lock(&mutex_);

  std::vector<FunctionInfo> result;

  for (const auto& [module_path, module_in_memory] : process.GetMemoryMap()) {
    auto it = module_map_.find(std::make_pair(module_path, module_in_memory.build_id()));
    CHECK(it != module_map_.end());
    const ModuleData* module = &it->second;
    CHECK(module != nullptr);
    if (!module->is_loaded()) continue;

    const std::vector<FunctionInfo>& orbit_functions = module->GetOrbitFunctions();
    result.insert(result.end(), orbit_functions.begin(), orbit_functions.end());
  }
  return result;
}

}  // namespace orbit_client_data