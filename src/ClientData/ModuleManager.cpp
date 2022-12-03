// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ModuleManager.h"

#include <absl/container/node_hash_map.h>
#include <absl/meta/type_traits.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Logging.h"
#include "SymbolProvider/ModuleIdentifier.h"

using orbit_symbol_provider::ModuleIdentifier;

namespace orbit_client_data {

std::vector<ModuleData*> ModuleManager::AddOrUpdateModules(
    absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  absl::MutexLock lock(&mutex_);

  std::vector<ModuleData*> unloaded_modules;

  for (const auto& module_info : module_infos) {
    auto module_id = ModuleIdentifier{module_info.file_path(), module_info.build_id()};
    auto module_it = module_map_.find(module_id);
    if (module_it == module_map_.end()) {
      const bool success = module_map_.try_emplace(module_id, module_info).second;
      ORBIT_CHECK(success);
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
    auto module_id = ModuleIdentifier{module_info.file_path(), module_info.build_id()};
    auto module_it = module_map_.find(module_id);
    if (module_it == module_map_.end()) {
      const bool success = module_map_.try_emplace(module_id, module_info).second;
      ORBIT_CHECK(success);
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
      module_map_.find(ModuleIdentifier{module_in_memory.file_path(), module_in_memory.build_id()});
  if (it == module_map_.end()) return nullptr;

  // The valid absolute address should be >=
  // module_base_address + (executable_segment_offset % kPageSize)
  if (absolute_address < module_in_memory.start() + (it->second.executable_segment_offset() %
                                                     orbit_module_utils::kPageSize)) {
    return nullptr;
  }

  return &it->second;
}

ModuleData* ModuleManager::GetMutableModuleByModuleInMemoryAndAbsoluteAddress(
    const ModuleInMemory& module_in_memory, uint64_t absolute_address) {
  absl::MutexLock lock(&mutex_);
  auto it =
      module_map_.find(ModuleIdentifier{module_in_memory.file_path(), module_in_memory.build_id()});
  if (it == module_map_.end()) return nullptr;

  // The valid absolute address should be >=
  // module_base_address + (executable_segment_offset % kPageSize)
  if (absolute_address < module_in_memory.start() + (it->second.executable_segment_offset() %
                                                     orbit_module_utils::kPageSize)) {
    return nullptr;
  }

  return &it->second;
}

const ModuleData* ModuleManager::GetModuleByModuleIdentifier(
    const ModuleIdentifier& module_id) const {
  absl::MutexLock lock(&mutex_);

  auto it = module_map_.find(module_id);
  if (it == module_map_.end()) return nullptr;

  return &it->second;
}

ModuleData* ModuleManager::GetMutableModuleByModuleIdentifier(const ModuleIdentifier& module_id) {
  absl::MutexLock lock(&mutex_);

  auto it = module_map_.find(module_id);
  if (it == module_map_.end()) return nullptr;

  return &it->second;
}

std::vector<const ModuleData*> ModuleManager::GetAllModuleData() const {
  absl::MutexLock lock(&mutex_);
  std::vector<const ModuleData*> result;
  for (const auto& [unused_module_id, module_data] : module_map_) {
    result.push_back(&module_data);
  }
  return result;
}

std::vector<const ModuleData*> ModuleManager::GetModulesByFilename(
    std::string_view filename) const {
  absl::MutexLock lock(&mutex_);
  std::vector<const ModuleData*> result;
  for (const auto& [module_id, module_data] : module_map_) {
    if (std::filesystem::path(module_id.file_path).filename().string() == filename) {
      result.push_back(&module_data);
    }
  }
  return result;
}

}  // namespace orbit_client_data
