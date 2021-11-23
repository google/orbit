// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ModuleManager.h"

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientProtos/capture_data.pb.h"
#include "ObjectUtils/Address.h"
#include "OrbitBase/Logging.h"
#include "absl/synchronization/mutex.h"

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

std::vector<ModuleData*> ModuleManager::AddOrUpdateNotLoadedModules(
    absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  absl::MutexLock lock(&mutex_);

  std::vector<ModuleData*> not_updated_modules;

  for (const auto& module_info : module_infos) {
    auto module_id = std::make_pair(module_info.file_path(), module_info.build_id());
    auto module_it = module_map_.find(module_id);
    if (module_it == module_map_.end()) {
      const bool success = module_map_.try_emplace(module_id, module_info).second;
      CHECK(success);
    } else {
      ModuleData& module = module_it->second;
      if (!module.UpdateIfChangedAndNotLoaded(module_info)) {
        not_updated_modules.push_back(&module);
      }
    }
  }

  return not_updated_modules;
}

const ModuleData* ModuleManager::GetModuleByModuleInMemoryAndAbsoluteAddress(
    const ModuleInMemory& module_in_memory, uint64_t absolute_address) const {
  absl::MutexLock lock(&mutex_);
  auto it =
      module_map_.find(std::make_pair(module_in_memory.file_path(), module_in_memory.build_id()));
  if (it == module_map_.end()) return nullptr;

  // The valid absolute address should be >=
  // module_base_address + (executable_segment_offset % kPageSize)
  if (absolute_address < module_in_memory.start() + (it->second.executable_segment_offset() %
                                                     orbit_object_utils::kPageSize)) {
    return nullptr;
  }

  return &it->second;
}

ModuleData* ModuleManager::GetMutableModuleByModuleInMemoryAndAbsoluteAddress(
    const ModuleInMemory& module_in_memory, uint64_t absolute_address) {
  absl::MutexLock lock(&mutex_);
  auto it =
      module_map_.find(std::make_pair(module_in_memory.file_path(), module_in_memory.build_id()));
  if (it == module_map_.end()) return nullptr;

  // The valid absolute address should be >=
  // module_base_address + (executable_segment_offset % kPageSize)
  if (absolute_address < module_in_memory.start() + (it->second.executable_segment_offset() %
                                                     orbit_object_utils::kPageSize)) {
    return nullptr;
  }

  return &it->second;
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

std::vector<const ModuleData*> ModuleManager::GetAllModuleData() const {
  absl::MutexLock lock(&mutex_);
  std::vector<const ModuleData*> result;
  for (const auto& [unused_pair, module_data] : module_map_) {
    result.push_back(&module_data);
  }
  return result;
}

std::vector<const ModuleData*> ModuleManager::GetModulesByFilename(
    const std::string& filename) const {
  absl::MutexLock lock(&mutex_);
  std::vector<const ModuleData*> result;
  for (const auto& [path_build_id_pair, module_data] : module_map_) {
    const std::string& file_path = path_build_id_pair.first;
    if (std::filesystem::path(file_path).filename().string() == filename) {
      result.push_back(&module_data);
    }
  }
  return result;
}

}  // namespace orbit_client_data
