// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include "ApiLoader/EnableInTracee.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"
// clang-format on

#include <absl/base/casts.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/match.h>
#include <dlfcn.h>
#include <stdint.h>
#include <sys/types.h>

#include <filesystem>
#include <string>
#include <type_traits>
#include <vector>

#include "ApiUtils/GetFunctionTableAddressPrefix.h"
#include "ModuleUtils/ReadLinuxModules.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitBase/UniqueResource.h"
#include "UserSpaceInstrumentation/AnyThreadIsInStrictSeccompMode.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

using orbit_api_utils::kOrbitApiGetFunctionTableAddressPrefix;
using orbit_api_utils::kOrbitApiGetFunctionTableAddressWinPrefix;
using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_user_space_instrumentation::AnyThreadIsInStrictSeccompMode;
using orbit_user_space_instrumentation::AttachAndStopNewThreadsOfProcess;
using orbit_user_space_instrumentation::AttachAndStopProcess;
using orbit_user_space_instrumentation::DetachAndContinueProcess;
using orbit_user_space_instrumentation::DlmopenInTracee;
using orbit_user_space_instrumentation::DlsymInTracee;
using orbit_user_space_instrumentation::ExecuteInProcess;
using orbit_user_space_instrumentation::ExecuteInProcessWithMicrosoftCallingConvention;
using orbit_user_space_instrumentation::LinkerNamespace;

namespace {

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
  ORBIT_SCOPED_TIMED_LOG("%s Api in tracee", enabled ? "Enabling" : "Disabling");
  if (capture_options.api_functions().empty()) {
    ORBIT_LOG("No api table to initialize");
    return outcome::success();
  }

  int32_t pid = orbit_base::ToNativeProcessId(capture_options.pid());

  OUTCOME_TRY(auto&& already_attached_tids, AttachAndStopProcess(pid));

  // Make sure we resume the target process, even on early-outs.
  orbit_base::unique_resource scope_exit{pid, [](int32_t pid) {
                                           if (DetachAndContinueProcess(pid).has_error()) {
                                             ORBIT_ERROR("Detaching from %i", pid);
                                           }
                                         }};

  if (AnyThreadIsInStrictSeccompMode(pid)) {
    return ErrorMessage("At least one thread of the target process is in strict seccomp mode.");
  }

  // Load liborbit.so and find api table initialization function.
  OUTCOME_TRY(auto&& liborbit_path, GetLibOrbitPath());
  ORBIT_LOG("Injecting library \"%s\" into process %d", liborbit_path, pid);
  OUTCOME_TRY(auto&& modules, orbit_module_utils::ReadModules(pid));
  OUTCOME_TRY(auto&& handle, DlmopenInTracee(pid, modules, liborbit_path, RTLD_NOW,
                                             LinkerNamespace::kUseInitialNamespace));
  ORBIT_LOG("Resolving function pointers in injected library");
  constexpr const char* kSetEnabledFunction = "orbit_api_set_enabled";
  OUTCOME_TRY(auto&& orbit_api_set_enabled_function,
              DlsymInTracee(pid, modules, handle, kSetEnabledFunction));
  constexpr const char* kSetEnabledWineFunction = "orbit_api_set_enabled_wine";
  OUTCOME_TRY(auto&& orbit_api_set_enabled_wine_function,
              DlsymInTracee(pid, modules, handle, kSetEnabledWineFunction));

  // Initialize all api function tables.
  for (const ApiFunction& api_function : capture_options.api_functions()) {
    // Filter api functions.
    ORBIT_CHECK(absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressPrefix) ||
                absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix));

    // Get address of function table by calling "orbit_api_get_function_table_address_vN" in tracee.
    // Note that the address start is always page_aligned and we need to account for that by
    // aligning executable_segment_offset as well.
    void* api_function_address = absl::bit_cast<void*>(api_function.absolute_address());
    uint64_t function_table_address = 0;
    if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressPrefix)) {
      // The target is a native Linux binary.
      ORBIT_LOG("Getting function table address from native Linux binary");
      OUTCOME_TRY(function_table_address, ExecuteInProcess(pid, api_function_address));
    } else if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix)) {
      // The target is a Windows binary running on Wine.
      ORBIT_LOG("Getting function table address from Wine binary");
      OUTCOME_TRY(function_table_address,
                  ExecuteInProcessWithMicrosoftCallingConvention(pid, api_function_address));
    } else {
      ORBIT_UNREACHABLE();
    }

    // Call "orbit_api_set_enabled" in tracee.
    if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressPrefix)) {
      // Again, Linux binary.
      ORBIT_LOG("%s Orbit API in native Linux binary", enabled ? "Enabling" : "Disabling");
      OUTCOME_TRY(ExecuteInProcess(pid, orbit_api_set_enabled_function, function_table_address,
                                   api_function.api_version(), enabled ? 1 : 0));
    } else if (absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix)) {
      // Windows binary running on Wine.
      ORBIT_LOG("%s Orbit API in Wine binary", enabled ? "Enabling" : "Disabling");
      OUTCOME_TRY(ExecuteInProcess(pid, orbit_api_set_enabled_wine_function, function_table_address,
                                   api_function.api_version(), enabled ? 1 : 0));
    } else {
      ORBIT_UNREACHABLE();
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
