// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "ClientData/ModuleIdentifierProvider.h"

#include <absl/meta/type_traits.h>

#include <utility>

#include "ClientData/ModulePathAndBuildId.h"

namespace orbit_client_data {

ModuleIdentifier ModuleIdentifierProvider::CreateModuleIdentifier(
    const ModulePathAndBuildId& module_path_and_build_id) {
  absl::WriterMutexLock lock(&mutex_);

  // We are using the current size as next id. If insertion does not take place, size remains the
  // same and we are not wasting ids.
  ModuleIdentifier module_identifier{module_identifier_map_.size()};
  auto [module_identifier_it, insertion_happened] =
      module_identifier_map_.try_emplace(module_path_and_build_id, module_identifier);

  return module_identifier_it->second;
}

std::optional<ModuleIdentifier> ModuleIdentifierProvider::GetModuleIdentifier(
    const ModulePathAndBuildId& module_path_and_build_id) const {
  absl::ReaderMutexLock lock(&mutex_);
  const auto it = module_identifier_map_.find(module_path_and_build_id);
  if (it == module_identifier_map_.end()) return std::nullopt;
  return it->second;
}

std::optional<ModulePathAndBuildId> ModuleIdentifierProvider::GetModulePathAndBuildId(
    ModuleIdentifier module_identifier) const {
  absl::ReaderMutexLock lock(&mutex_);

  for (const auto& [current_module_path_and_build_id, current_module_identifier] :
       module_identifier_map_) {
    if (current_module_identifier == module_identifier) {
      return current_module_path_and_build_id;
    }
  }
  return std::nullopt;
}

}  // namespace orbit_client_data
