// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Api/EnableInTracee.h"

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>

#include <functional>

#include "Api/Orbit.h"
#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ModuleInfo;
using orbit_user_space_instrumentation::AttachAndStopProcess;
using orbit_user_space_instrumentation::DetachAndContinueProcess;
using orbit_user_space_instrumentation::DlopenInTracee;
using orbit_user_space_instrumentation::DlsymInTracee;
using orbit_user_space_instrumentation::ExecuteInProcess;

namespace {

ErrorMessageOr<absl::flat_hash_map<std::string, ModuleInfo>> GetModulesByPathForPid(int32_t pid) {
  OUTCOME_TRY(module_infos, orbit_elf_utils::ReadModules(pid));
  absl::flat_hash_map<std::string, ModuleInfo> result;
  for (const ModuleInfo& module_info : module_infos) {
    result.emplace(module_info.file_path(), std::move(module_info));
  }
  return result;
}

[[nodiscard]] const ModuleInfo* FindModuleInfoForApiFunction(
    const ApiFunction& api_function,
    const absl::flat_hash_map<std::string, ModuleInfo>& modules_by_path) {
  auto module_info_it = modules_by_path.find(api_function.module_path());
  if (module_info_it == modules_by_path.end()) {
    ERROR("Could not find module \"%s\" when initializing Orbit Api.", api_function.module_path());
    return nullptr;
  }
  const ModuleInfo& module_info = module_info_it->second;
  if (module_info.build_id() != api_function.module_build_id()) {
    ERROR("Build-id mismatch for \"%s\" when initializing Orbit Api", api_function.module_path());
    return nullptr;
  }

  return &module_info;
}

ErrorMessageOr<std::string> GetLibOrbitPath() {
  // When packaged, liborbit.so is found alongside OrbitService.  In development, it is found in
  // "../lib", relative to OrbitService.
  constexpr const char* kLibOrbitName = "liborbit.so";
  const std::filesystem::path exe_dir = orbit_base::GetExecutableDir();
  std::vector<std::filesystem::path> potential_paths = {exe_dir / kLibOrbitName,
                                                        exe_dir / "../lib" / kLibOrbitName};
  for (const auto& path : potential_paths) {
    if (std::filesystem::exists(path)) {
      return path;
    }
  }

  return ErrorMessage("Liborbit.so not found on system.");
}

ErrorMessageOr<void> SetApiEnabledInTracee(const CaptureOptions& capture_options, bool enabled) {
  SCOPED_TIMED_LOG("%s Api in tracee", enabled ? "Enabling" : "Disabling");
  if (capture_options.api_functions().size() == 0) {
    return ErrorMessage("No api table to initialize.");
  }

  int32_t pid = capture_options.pid();

  OUTCOME_TRY(AttachAndStopProcess(pid));

  // Make sure we resume the target process, even on early-outs.
  orbit_base::unique_resource scope_exit{pid, [](int32_t pid) {
                                           if (DetachAndContinueProcess(pid).has_error()) {
                                             ERROR("Detaching from %i", pid);
                                           }
                                         }};

  // Load liborbit.so and find api table initialization function.
  OUTCOME_TRY(liborbit_path, GetLibOrbitPath());
  constexpr const char* kSetEnabledFunction = "orbit_api_set_enabled";
  OUTCOME_TRY(handle, DlopenInTracee(pid, liborbit_path, RTLD_NOW));
  OUTCOME_TRY(orbit_api_set_enabled_function, DlsymInTracee(pid, handle, kSetEnabledFunction));

  // Initialize all api function tables.
  OUTCOME_TRY(modules_by_path, GetModulesByPathForPid(pid));
  for (const ApiFunction& api_function : capture_options.api_functions()) {
    // Filter api functions.
    constexpr const char* kOrbitApiGetAddressPrefix = "orbit_api_get_function_table_address_v";
    if (!absl::StrContains(api_function.name(), kOrbitApiGetAddressPrefix)) continue;

    // Get ModuleInfo associated with function.
    const ModuleInfo* module_info = FindModuleInfoForApiFunction(api_function, modules_by_path);
    if (module_info == nullptr) continue;

    // Get address of function table by calling "orbit_api_get_function_table_address_vN" in tracee.
    void* api_function_address = absl::bit_cast<void*>(
        module_info->address_start() + api_function.address() - module_info->load_bias());
    OUTCOME_TRY(function_table_address, ExecuteInProcess(pid, api_function_address));

    // Call "orbit_api_set_enabled" in tracee.
    OUTCOME_TRY(ExecuteInProcess(pid, orbit_api_set_enabled_function, function_table_address,
                                 api_function.api_version(), enabled ? 1 : 0));
  }

  return outcome::success();
}

}  // namespace

namespace orbit_api {

ErrorMessageOr<void> EnableApiInTracee(const orbit_grpc_protos::CaptureOptions& capture_options) {
  return ::SetApiEnabledInTracee(capture_options, /*enabled*/ true);
}

ErrorMessageOr<void> DisableApiInTracee(const orbit_grpc_protos::CaptureOptions& capture_options) {
  return ::SetApiEnabledInTracee(capture_options, /*enabled*/ false);
}

}  // namespace orbit_api
