// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include <cstdint>
#include <string>
#include <vector>

#include "DataManager.h"
#include "ModulesDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitFlags/Declarations.h"
#include "absl/flags/flag.h"
#include "module.pb.h"
#include "process.pb.h"
#include "services.pb.h"

using orbit_client_data::ModuleManager;
using orbit_grpc_protos::GetModuleListResponse;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

DEFINE_PROTO_FUZZER(const GetModuleListResponse& module_list) {
  const auto range = module_list.modules();
  std::vector<ModuleInfo> modules{range.begin(), range.end()};

  ModuleManager module_manager;
  module_manager.AddOrUpdateModules(modules);

  int32_t pid = 1;
  ProcessInfo process_info{};
  process_info.set_pid(pid);

  DataManager data_manager{};
  data_manager.UpdateProcessInfos(std::vector{process_info});

  ProcessData* process = data_manager.GetMutableProcessByPid(pid);
  CHECK(process != nullptr);
  process->UpdateModuleInfos(modules);

  ModulesDataView modules_data_view{nullptr};
  modules_data_view.UpdateModules(process);
}
