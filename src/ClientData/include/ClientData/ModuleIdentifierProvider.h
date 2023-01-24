// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_MODULE_IDENTIFIER_PROVIDER_H_
#define CLIENT_DATA_MODULE_IDENTIFIER_PROVIDER_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/strings/substitute.h>
#include <absl/synchronization/mutex.h>

#include <optional>
#include <string>
#include <string_view>

#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModulePathAndBuildId.h"
#include "OrbitBase/Result.h"

namespace orbit_client_data {

// A module is uniquely identified by a file path and a build id. In order to avoid storing and
// passing those two strings around (with potentially expensive copies), we use the
// `ModuleIdentifier` class to uniquely identify a module. In particular, that class can act as a
// key in maps.
// `ModuleIdentifierProvider` manages the creation and retrieval of `ModuleIdentifier` instances for
// a given module path a build id. As long as you use the same instance of this class, you are
// guaranteed uniqueness of the identifiers for the same path and build id.
//
// `ModuleIdentifier` objects are created using the `CreateModuleIdentifier` method, while existing
// identifiers can be queried using `GetModuleIdentifier` method.
// To get a path or build id of a concrete `ModuleIdentifier`, this class provides
// `GetModulePathAndBuildId`.
//
// Thread-safety: This class is thread-safe.
class ModuleIdentifierProvider {
 public:
  // Creates and returns a new module identifier for the given module path and build id. If there
  // already exists a module identifier with that path and build id, the existing module identifier
  // is returned.
  ModuleIdentifier CreateModuleIdentifier(const ModulePathAndBuildId& module_path_and_build_id);

  // Returns the unique module identifier for the given module path and build id, or std::nullopt
  // the module is unknown.
  [[nodiscard]] std::optional<ModuleIdentifier> GetModuleIdentifier(
      const ModulePathAndBuildId& module_path_and_build_id) const;

  // Returns the module path and build id associated with the given module identifier, or
  // std::nullopt if the module is unknown.
  [[nodiscard]] std::optional<ModulePathAndBuildId> GetModulePathAndBuildId(
      ModuleIdentifier module_identifier) const;

 private:
  mutable absl::Mutex mutex_;
  absl::flat_hash_map<ModulePathAndBuildId, ModuleIdentifier> module_identifier_map_
      ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_MODULE_IDENTIFIER_PROVIDER_H_
