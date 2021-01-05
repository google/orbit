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
#include "absl/flags/flag.h"
#include "module.pb.h"
#include "process.pb.h"
#include "services.pb.h"

// Hack: This is declared in a header we include here
// and the definition needs to take place somewhere.
ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");
ABSL_FLAG(bool, enable_frame_pointer_validator, false, "Enable validation of frame pointers");
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");
ABSL_FLAG(bool, enable_tracepoint_feature, false,
          "Enable the setting of the panel of kernel tracepoints");
ABSL_FLAG(bool, thread_state, false, "Collect thread states");
// TODO(170468590): Remove this flag when the new UI is finished
ABSL_FLAG(bool, enable_ui_beta, false, "Enable the new user interface");

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
