// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_MODULE_MANAGER_H_
#define ORBIT_CLIENT_DATA_MODULE_MANAGER_H_

#include <string>
#include <vector>

#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ProcessData.h"
#include "absl/container/node_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"
#include "module.pb.h"

namespace orbit_client_data {

class ModuleManager final {
 public:
  explicit ModuleManager() = default;

  [[nodiscard]] const ModuleData* GetModuleByPath(const std::string& path) const;
  [[nodiscard]] ModuleData* GetMutableModuleByPath(const std::string& path);
  [[nodiscard]] std::vector<ModuleData*> GetMutableModulesByName(const std::string& path);
  // Add new modules for the module_infos that do not exist yet, and update the modules that do
  // exist. If the update changed the module in a way that symbols were not valid anymore, the
  // symbols are discarded aka the module is not loaded anymore. This method returns the list of
  // modules that used to be loaded before the call and are not loaded anymore after the call.
  std::vector<ModuleData*> AddOrUpdateModules(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);
  [[nodiscard]] std::vector<orbit_client_protos::FunctionInfo> GetOrbitFunctionsOfProcess(
      const ProcessData& process) const;

 private:
  mutable absl::Mutex mutex_;
  // We are sharing pointers to that entries and ensure reference stability by using node_hash_map
  absl::node_hash_map<std::string, ModuleData> module_map_;
};

}  // namespace orbit_client_data

#endif
