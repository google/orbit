// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include "ApiLoader/EnableInTracee.h"
// clang-format on

#include <absl/strings/match.h>

#include "ApiUtils/ApiEnableInfo.h"
#include "ApiUtils/GetFunctionTableAddressPrefix.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "WindowsUtils/DllInjection.h"

using orbit_api_utils::kOrbitApiGetFunctionTableAddressWinPrefix;
using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_windows_utils::InjectDllIfNotLoaded;

namespace {

ErrorMessageOr<std::filesystem::path> GetLibOrbitPath() {
  constexpr const char* kLibOrbitName = "OrbitApi.dll";
  const std::filesystem::path dll_path = orbit_base::GetExecutableDir() / kLibOrbitName;
  if (std::filesystem::exists(dll_path)) {
    return dll_path;
  }
  return ErrorMessage("OrbitApi.dll not found on system.");
}

ErrorMessageOr<void> SetApiEnabledInTracee(const CaptureOptions& capture_options, bool enabled) {
  ORBIT_SCOPED_TIMED_LOG("%s Api in tracee", enabled ? "Enabling" : "Disabling");
  if (capture_options.api_functions().empty()) {
    ORBIT_LOG("No api table to initialize");
    return outcome::success();
  }

  // Inject OrbitApi.dll into target process if it's not already loaded.
  OUTCOME_TRY(std::filesystem::path liborbit_path, GetLibOrbitPath());
  OUTCOME_TRY(InjectDllIfNotLoaded(capture_options.pid(), liborbit_path.string()));

  for (const ApiFunction& api_function : capture_options.api_functions()) {
    // Filter api functions.
    ORBIT_CHECK(absl::StartsWith(api_function.name(), kOrbitApiGetFunctionTableAddressWinPrefix));

    // Set up ApiEnableInfo to be passed as thread function parameter.
    orbit_api::ApiEnableInfo enable_info{};
    enable_info.orbit_api_function_address = api_function.absolute_address();
    enable_info.api_version = api_function.api_version();
    enable_info.api_enabled = enabled;

    // Encode ApiEnableInfo into a buffer that "CreateRemoteThread" will write in target process.
    std::vector<char> parameter(sizeof(enable_info), 0);
    std::memcpy(parameter.data(), &enable_info, sizeof(enable_info));

    // Call "orbit_api_set_enabled_from_struct" in the remote process.
    constexpr const char* kSetEnabledFunction = "orbit_api_set_enabled_from_struct";
    OUTCOME_TRY(orbit_windows_utils::CreateRemoteThread(
        capture_options.pid(), liborbit_path.filename().string(), kSetEnabledFunction, parameter));
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
