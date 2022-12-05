// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_MODULE_MANAGER_H_
#define CLIENT_DATA_MODULE_MANAGER_H_

#include <absl/container/node_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <string>
#include <string_view>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/module.pb.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "absl/container/node_hash_map.h"
#include "absl/synchronization/mutex.h"

namespace orbit_client_data {

class ModuleManager final {
 public:
  explicit ModuleManager() = default;

  [[nodiscard]] const ModuleData* GetModuleByModuleInMemoryAndAbsoluteAddress(
      const ModuleInMemory& module_in_memory, uint64_t absolute_address) const;

  [[nodiscard]] ModuleData* GetMutableModuleByModuleInMemoryAndAbsoluteAddress(
      const ModuleInMemory& module_in_memory, uint64_t absolute_address);

  [[nodiscard]] const ModuleData* GetModuleByModuleIdentifier(
      const orbit_symbol_provider::ModuleIdentifier& module_id) const;
  [[nodiscard]] ModuleData* GetMutableModuleByModuleIdentifier(
      const orbit_symbol_provider::ModuleIdentifier& module_id);
  // Add new modules for the module_infos that do not exist yet, and update the modules that do
  // exist. If the update changed the module in a way that symbols were not valid anymore, the
  // symbols are discarded, i.e., the module is no longer loaded. This method returns the list of
  // modules that used to be loaded before the call and are no longer loaded after the call.
  [[nodiscard]] std::vector<ModuleData*> AddOrUpdateModules(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);

  // Similar to AddOrUpdateModules, except that it does not update modules that already have
  // symbols. Returns the list of modules that it did not update.
  [[nodiscard]] std::vector<ModuleData*> AddOrUpdateNotLoadedModules(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);

  [[nodiscard]] std::vector<const ModuleData*> GetAllModuleData() const;

  [[nodiscard]] std::vector<const ModuleData*> GetModulesByFilename(
      std::string_view filename) const;

 private:
  mutable absl::Mutex mutex_;
  // We are sharing pointers to that entries and ensure reference stability by using node_hash_map
  // Map of ModuleIdentifier -> ModuleData (ModuleIdentifier is file_path and build_id)
  absl::node_hash_map<orbit_symbol_provider::ModuleIdentifier, ModuleData> module_map_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_MODULE_MANAGER_H_
