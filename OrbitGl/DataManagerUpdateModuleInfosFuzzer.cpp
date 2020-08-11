// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include "DataManager.h"
#include "ModulesDataView.h"
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
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false,
          "Use frame pointers for unwinding");

DEFINE_PROTO_FUZZER(const GetModuleListResponse& module_list) {
  const auto range = module_list.modules();
  std::vector<ModuleInfo> modules{range.begin(), range.end()};

  ProcessInfo process_info{};
  process_info.set_pid(1);

  DataManager data_manager{};
  data_manager.UpdateProcessInfos(std::vector{process_info});
  data_manager.UpdateModuleInfos(1, modules);

  ModulesDataView modules_data_view{};
  modules_data_view.SetModules(1, data_manager.GetModules(1));
}