// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_MODULE_MANAGER_H_
#define ORBIT_CLIENT_DATA_MODULE_MANAGER_H_

#include <vector>

#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ProcessData.h"
#include "absl/container/node_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"
#include "module.pb.h"

namespace OrbitClientData {

class ModuleManager final {
 public:
  explicit ModuleManager() = default;

  [[nodiscard]] const ModuleData* GetModuleByPath(const std::string& path) const;
  [[nodiscard]] ModuleData* GetMutableModuleByPath(const std::string& path);
  void AddOrUpdateModules(const std::vector<orbit_grpc_protos::ModuleInfo>& module_infos);
  [[nodiscard]] std::vector<orbit_client_protos::FunctionInfo> GetOrbitFunctionsOfProcess(
      const ProcessData& process) const;

 private:
  mutable absl::Mutex mutex_;
  // We are sharing pointers to that entries and ensure reference stability by using node_hash_map
  absl::node_hash_map<std::string, ModuleData> module_map_;
};

}  // namespace OrbitClientData

#endif