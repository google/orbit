// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_MODULE_IDENTIFIER_PROVIDER_H_
#define CLIENT_DATA_MODULE_IDENTIFIER_PROVIDER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/strings/substitute.h>
#include <absl/synchronization/mutex.h>

#include <optional>
#include <string>
#include <string_view>

#include "ClientData/ModuleIdentifier.h"
#include "OrbitBase/Result.h"

namespace orbit_client_data {

// We consider a module being unique if it has the same file path and build id. In order to avoid
// storing and passing those two strings around (with potentially expensive copies), we use the
// `ModuleIdentifier` class, to uniquely identify a module. In particular, that class can act as a
// key in maps.
// This class (`ModuleIdentifierProvider`) manages the creation and retreaval of `ModuleIdentifier`
// instances for a given module path an build id. As long you use the same instance of this class,
// it will ensure uniqueness of the identifiers for the same path and build id.
//
// `ModuleIdentifier` objects be created using the `CreateModuleIdentifier` method, while existing
// identifiers can be querried using `GetModuleIdentifier` method.
// In order to get a path or build id of a concrete `ModuleIdentifier`, this class also provides
// accessor methods.
class ModuleIdentifierProvider {
 public:
  // Creates and returns a new module identifier for the given module path and build id. If there
  // already exists a module identifier with that path and build id, the existing module identifier
  // is return.
  ModuleIdentifier CreateModuleIdentifier(std::string_view module_path, std::string_view build_id) {
    absl::WriterMutexLock lock(&mutex_);

    // We are using the current size as next id. If insertion does not take place, size remains the
    // same and we are not wasting ids.
    ModuleIdentifier module_identifier{module_identifier_map_.size()};
    auto [module_identifier_it, insertion_happened] = module_identifier_map_.try_emplace(
        std::pair<std::string, std::string>{module_path, build_id}, module_identifier);

    return module_identifier_it->second;
  }

  // Returns the unquie module identifier for the given module path and build id, or std::nullopt,
  // the module is yet unknown.
  [[nodiscard]] std::optional<ModuleIdentifier> GetModuleIdentifier(
      std::string_view module_path, std::string_view build_id) const {
    absl::ReaderMutexLock lock(&mutex_);
    const auto it =
        module_identifier_map_.find(std::pair<std::string, std::string>{module_path, build_id});
    if (it == module_identifier_map_.end()) return std::nullopt;
    return it->second;
  }

  // Returns the module path associated with the given module identifier, or std::nullopt if the
  // module is yet unknown.
  [[nodiscard]] std::optional<std::string> GetModulePath(ModuleIdentifier module_identifier) const {
    absl::ReaderMutexLock lock(&mutex_);

    for (const auto& [current_module_path_and_build_id, current_module_identifier] :
         module_identifier_map_) {
      if (current_module_identifier == module_identifier) {
        return current_module_path_and_build_id.first;
      }
    }
    return std::nullopt;
  }

  // Returns the build id associated with the given module identifier, or std::nullopt if the
  // module is yet unknown.
  [[nodiscard]] std::optional<std::string> GetModuleBuildId(
      ModuleIdentifier module_identifier) const {
    absl::ReaderMutexLock lock(&mutex_);

    for (const auto& [current_module_path_and_build_id, current_module_identifier] :
         module_identifier_map_) {
      if (current_module_identifier == module_identifier) {
        return current_module_path_and_build_id.second;
      }
    }
    return std::nullopt;
  }

  // Returns the module path and build id associated with the given module identifier, or
  // std::nullopt if the module is yet unknown.
  [[nodiscard]] std::optional<std::pair<std::string, std::string>> GetModulePathAndBuildId(
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

 private:
  mutable absl::Mutex mutex_;
  absl::flat_hash_map<std::pair<std::string, std::string>, ModuleIdentifier> module_identifier_map_
      ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_MODULE_IDENTIFIER_PROVIDER_H_
