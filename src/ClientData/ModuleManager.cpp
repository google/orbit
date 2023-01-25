// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ModuleManager.h"

#include <absl/meta/type_traits.h>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ModuleIdentifier.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/Logging.h"

using orbit_client_data::ModuleIdentifier;

namespace orbit_client_data {

std::vector<ModuleData*> ModuleManager::AddOrUpdateModules(
    absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  absl::MutexLock lock(&mutex_);

  std::vector<ModuleData*> unloaded_modules;

  for (const auto& module_info : module_infos) {
    ModulePathAndBuildId module_path_and_build_id{.module_path = module_info.file_path(),
                                                  .build_id = module_info.build_id()};
    std::optional<ModuleIdentifier> module_id_opt =
        module_identifier_provider_->GetModuleIdentifier(module_path_and_build_id);
    if (module_id_opt.has_value()) {
      auto module_it = module_map_.find(module_id_opt.value());
      ORBIT_CHECK(module_it != module_map_.end());
      ModuleData* module = module_it->second.get();
      if (module->UpdateIfChangedAndUnload(module_info)) {
        unloaded_modules.push_back(module);
      }
    } else {
      ModuleIdentifier module_id =
          module_identifier_provider_->CreateModuleIdentifier(module_path_and_build_id);
      bool success =
          module_map_.try_emplace(module_id, std::make_unique<ModuleData>(module_info)).second;
      ORBIT_CHECK(success);
    }
  }

  return unloaded_modules;
}

std::vector<ModuleData*> ModuleManager::AddOrUpdateNotLoadedModules(
    absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  absl::MutexLock lock(&mutex_);

  std::vector<ModuleData*> not_updated_modules;

  for (const auto& module_info : module_infos) {
    ModulePathAndBuildId module_path_and_build_id{.module_path = module_info.file_path(),
                                                  .build_id = module_info.build_id()};
    std::optional<ModuleIdentifier> module_id_opt =
        module_identifier_provider_->GetModuleIdentifier(module_path_and_build_id);

    if (module_id_opt.has_value()) {
      auto module_it = module_map_.find(module_id_opt.value());
      ORBIT_CHECK(module_it != module_map_.end());
      ModuleData* module = module_it->second.get();
      if (!module->UpdateIfChangedAndNotLoaded(module_info)) {
        not_updated_modules.push_back(module);
      }
    } else {
      ModuleIdentifier module_id =
          module_identifier_provider_->CreateModuleIdentifier(module_path_and_build_id);
      bool success =
          module_map_.try_emplace(module_id, std::make_unique<ModuleData>(module_info)).second;
      ORBIT_CHECK(success);
    }
  }

  return not_updated_modules;
}

const ModuleData* ModuleManager::GetModuleByModuleInMemoryAndAbsoluteAddress(
    const ModuleInMemory& module_in_memory, uint64_t absolute_address) const {
  absl::MutexLock lock(&mutex_);
  auto cache_it = absolute_address_to_module_data_cache_.find(absolute_address);
  if (cache_it != absolute_address_to_module_data_cache_.end()) {
    return cache_it->second;
  }
  auto it = module_map_.find(module_in_memory.module_id());
  if (it == module_map_.end()) {
    absolute_address_to_module_data_cache_.emplace(absolute_address, nullptr);
    return nullptr;
  }

  // The valid absolute address should be >=
  // module_base_address + (executable_segment_offset % kPageSize)
  if (absolute_address < module_in_memory.start() + (it->second->executable_segment_offset() %
                                                     orbit_module_utils::kPageSize)) {
    absolute_address_to_module_data_cache_.emplace(absolute_address, nullptr);
    return nullptr;
  }

  ModuleData* result = it->second.get();
  absolute_address_to_module_data_cache_.emplace(absolute_address, result);
  return result;
}

ModuleData* ModuleManager::GetMutableModuleByModuleInMemoryAndAbsoluteAddress(
    const ModuleInMemory& module_in_memory, uint64_t absolute_address) {
  absl::MutexLock lock(&mutex_);
  auto cache_it = absolute_address_to_module_data_cache_.find(absolute_address);
  if (cache_it != absolute_address_to_module_data_cache_.end()) {
    return cache_it->second;
  }
  auto it = module_map_.find(module_in_memory.module_id());
  if (it == module_map_.end()) {
    absolute_address_to_module_data_cache_.emplace(absolute_address, nullptr);
    return nullptr;
  }

  // The valid absolute address should be >=
  // module_base_address + (executable_segment_offset % kPageSize)
  if (absolute_address < module_in_memory.start() + (it->second->executable_segment_offset() %
                                                     orbit_module_utils::kPageSize)) {
    absolute_address_to_module_data_cache_.emplace(absolute_address, nullptr);
    return nullptr;
  }

  ModuleData* result = it->second.get();
  absolute_address_to_module_data_cache_.emplace(absolute_address, result);
  return result;
}

const ModuleData* ModuleManager::GetModuleByModuleIdentifier(ModuleIdentifier module_id) const {
  absl::MutexLock lock(&mutex_);
  return GetModuleByModuleIdentifierInternal(module_id);
}

const ModuleData* ModuleManager::GetModuleByModuleIdentifierInternal(
    ModuleIdentifier module_id) const {
  auto it = module_map_.find(module_id);
  if (it == module_map_.end()) return nullptr;

  return it->second.get();
}

ModuleData* ModuleManager::GetMutableModuleByModuleIdentifier(ModuleIdentifier module_id) {
  absl::MutexLock lock(&mutex_);
  return GetMutableModuleByModuleIdentifierInternal(module_id);
}

ModuleData* ModuleManager::GetMutableModuleByModuleIdentifierInternal(ModuleIdentifier module_id) {
  auto it = module_map_.find(module_id);
  if (it == module_map_.end()) return nullptr;

  return it->second.get();
}

const ModuleData* ModuleManager::GetModuleByModulePathAndBuildId(
    const ModulePathAndBuildId& module_path_and_build_id) const {
  absl::MutexLock lock(&mutex_);
  std::optional<orbit_client_data::ModuleIdentifier> module_id =
      module_identifier_provider_->GetModuleIdentifier(module_path_and_build_id);
  if (!module_id.has_value()) return nullptr;
  return GetModuleByModuleIdentifierInternal(module_id.value());
}

ModuleData* ModuleManager::GetMutableModuleByModulePathAndBuildId(
    const ModulePathAndBuildId& module_path_and_build_id) {
  absl::MutexLock lock(&mutex_);
  std::optional<orbit_client_data::ModuleIdentifier> module_id =
      module_identifier_provider_->GetModuleIdentifier(module_path_and_build_id);
  if (!module_id.has_value()) return nullptr;
  return GetMutableModuleByModuleIdentifierInternal(module_id.value());
}

std::vector<const ModuleData*> ModuleManager::GetAllModuleData() const {
  absl::MutexLock lock(&mutex_);
  std::vector<const ModuleData*> result;
  for (const auto& [unused_module_id, module_data] : module_map_) {
    result.push_back(module_data.get());
  }
  return result;
}

std::vector<const ModuleData*> ModuleManager::GetModulesByFilename(
    std::string_view filename) const {
  absl::MutexLock lock(&mutex_);
  std::vector<const ModuleData*> result;
  for (const auto& [module_id, module_data] : module_map_) {
    if (std::filesystem::path(module_data->file_path()).filename().string() == filename) {
      result.push_back(module_data.get());
    }
  }
  return result;
}

}  // namespace orbit_client_data
