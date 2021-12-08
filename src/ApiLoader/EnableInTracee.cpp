// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ApiLoader/EnableInTracee.h"

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>

#include <functional>

#include "ApiUtils/GetFunctionTableAddressPrefix.h"
#include "ObjectUtils/Address.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitBase/UniqueResource.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

using orbit_api_utils::kOrbitApiGetFunctionTableAddressPrefix;
using orbit_api_utils::kOrbitApiGetFunctionTableAddressWinPrefix;
using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ModuleInfo;
using orbit_user_space_instrumentation::AttachAndStopNewThreadsOfProcess;
using orbit_user_space_instrumentation::AttachAndStopProcess;
using orbit_user_space_instrumentation::DetachAndContinueProcess;
using orbit_user_space_instrumentation::DlopenInTracee;
using orbit_user_space_instrumentation::DlsymInTracee;
using orbit_user_space_instrumentation::ExecuteInProcess;
using orbit_user_space_instrumentation::ExecuteInProcessWithMicrosoftCallingConvention;

namespace {

ErrorMessageOr<absl::flat_hash_map<std::string, ModuleInfo>> GetModulesByPathForPid(int32_t pid) {
  OUTCOME_TRY(auto&& module_infos, orbit_object_utils::ReadModules(pid));
  absl::flat_hash_map<std::string, ModuleInfo> result;
  for (ModuleInfo& module_info : module_infos) {
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

  return ErrorMessage("liborbit.so not found on system.");
}

ErrorMessageOr<void> SetApiEnabledInTracee(const CaptureOptions& capture_options, bool enabled) {
  SCOPED_TIMED_LOG("%s Api in tracee", enabled ? "Enabling" : "Disabling");
  if (capture_options.api_functions().empty()) {
    LOG("No api table to initialize");
    return outcome::success();
  }

  int32_t pid = orbit_base::ToNativeProcessId(capture_options.pid());

  OUTCOME_TRY(auto&& already_attached_tids, AttachAndStopProcess(pid));

  // Make sure we resume the target process, even on early-outs.
  orbit_base::unique_resource scope_exit{pid, [](int32_t pid) {
                                           if (DetachAndContinueProcess(pid).has_error()) {
                                             ERROR("Detaching from %i", pid);
                                           }
                                         }};

  // Load liborbit.so and find api table initialization function.
  OUTCOME_TRY(auto&& liborbit_path, GetLibOrbitPath());
  OUTCOME_TRY(auto&& handle, DlopenInTracee(pid, liborbit_path, RTLD_NOW));
  constexpr const char* kSetEnabledFunction = "orbit_api_set_enabled";
  OUTCOME_TRY(auto&& orbit_api_set_enabled_function,
              DlsymInTracee(pid, handle, kSetEnabledFunction));
  constexpr const char* kSetEnabledWineFunction = "orbit_api_set_enabled_wine";
  OUTCOME_TRY(auto&& orbit_api_set_enabled_wine_function,
              DlsymInTracee(pid, handle, kSetEnabledWineFunction));

  // Initialize all api function tables.
  OUTCOME_TRY(auto&& modules_by_path, GetModulesByPathForPid(pid));
  for (const ApiFunction& api_function : capture_options.api_functions()) {
    // Filter api functions.
    CHECK(absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressPrefix) ||
          absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix));

    // Get ModuleInfo associated with function.
    const ModuleInfo* module_info = FindModuleInfoForApiFunction(api_function, modules_by_path);
    if (module_info == nullptr) continue;

    // Get address of function table by calling "orbit_api_get_function_table_address_vN" in tracee.
    // Note that the address start is always page_aligned and we need to account for that by
    // aligning executable_segment_offset as well.
    void* api_function_address =
        absl::bit_cast<void*>(orbit_object_utils::SymbolVirtualAddressToAbsoluteAddress(
            api_function.address(), module_info->address_start(), module_info->load_bias(),
            module_info->executable_segment_offset()));
    uint64_t function_table_address = 0;
    if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressPrefix)) {
      // The target is a native Linux binary.
      OUTCOME_TRY(function_table_address, ExecuteInProcess(pid, api_function_address));
    } else if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix)) {
      // The target is a Windows binary running on Wine.
      OUTCOME_TRY(function_table_address,
                  ExecuteInProcessWithMicrosoftCallingConvention(pid, api_function_address));
    } else {
      UNREACHABLE();
    }

    // Call "orbit_api_set_enabled" in tracee.
    if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressPrefix)) {
      // Again, Linux binary.
      OUTCOME_TRY(ExecuteInProcess(pid, orbit_api_set_enabled_function, function_table_address,
                                   api_function.api_version(), enabled ? 1 : 0));
    } else if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix)) {
      // Windows binary running on Wine.
      OUTCOME_TRY(ExecuteInProcess(pid, orbit_api_set_enabled_wine_function, function_table_address,
                                   api_function.api_version(), enabled ? 1 : 0));
    } else {
      UNREACHABLE();
    }

    // `orbit_api_set_enabled` could spawn new threads (and will, the first time it's called). Stop
    // those too, as this loop could be executed again and the assumption is that the target process
    // is completely stopped.
    OUTCOME_TRY(already_attached_tids,
                AttachAndStopNewThreadsOfProcess(pid, already_attached_tids));
  }

  return outcome::success();
}

}  // namespace

namespace orbit_api_loader {

ErrorMessageOr<void> EnableApiInTracee(const orbit_grpc_protos::CaptureOptions& capture_options) {
  return ::SetApiEnabledInTracee(capture_options, /*enabled*/ true);
}

ErrorMessageOr<void> DisableApiInTracee(const orbit_grpc_protos::CaptureOptions& capture_options) {
  return ::SetApiEnabledInTracee(capture_options, /*enabled*/ false);
}

}  // namespace orbit_api_loader
