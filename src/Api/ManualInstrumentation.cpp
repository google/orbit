// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Api/ManualInstrumentation.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <dlfcn.h>

#include <functional>

#include "AccessTraceesMemory.h"
#include "Api/Orbit.h"
#include "Attach.h"
#include "InjectLibraryInTracee.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

using namespace orbit_user_space_instrumentation;

struct ScopeExit {
  ScopeExit(std::function<void(void)> exit_function) : exit_function(exit_function){};
  ~ScopeExit() {
    if (exit_function) exit_function();
  }
  std::function<void(void)> exit_function;
};

namespace orbit_api {
ErrorMessageOr<void> InitializeManualInstrumentationForProcess(int pid) {
  // auto orbit_api_info = orbit_base::ReadFileToString(absl::StrFormat("/tmp/orbit/%d", pid));
  // if (orbit_api_info.has_error()) {
  //   return ErrorMessage(absl::StrFormat("Target process [%i] did not initialize Orbit Api",
  //   pid));
  // }

  auto attach_result = AttachAndStopProcess(pid);
  if (attach_result.has_error()) {
    return attach_result.error();
  }

  {
    // Make sure we resume the target process even on early-outs.
    ScopeExit scope_exit([pid]() {
      auto detach_result = DetachAndContinueProcess(pid);
      if (detach_result.has_error()) {
        ERROR("Resuming target process [%i]: %s", pid, detach_result.error().message());
      }
    });

    const std::string kLibName = "liborbit.so";
    auto maps_before = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));

    if (maps_before.has_error()) {
      return maps_before.error();
    }

    if (absl::StrContains(maps_before.value(), kLibName)) {
      return ErrorMessage(
          absl::StrFormat("Target process [%i] has already loaded liborbit.so", pid));
    }

    // Load liborbit.so into target process.
    auto result_dlopen = DlopenInTracee(pid, orbit_base::GetExecutableDir() / kLibName, RTLD_NOW);
    if (result_dlopen.has_error()) {
      return result_dlopen.error();
    }

    auto maps_after_open = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
    if (maps_after_open.has_error()) {
      return maps_after_open.error();
    }

    if (absl::StrContains(maps_after_open.value(), kLibName) == false) {
      return ErrorMessage(
          absl::StrFormat("Dynamic loading of liborbit.so into target process [%i] failed", pid));
    }

    const char* kInitFunction = "orbit_initialize_manual_instrumentation";
    auto result_dlsym = DlsymInTracee(pid, result_dlopen.value(), kInitFunction);
    if (result_dlsym.has_value()) {
      auto init_function = reinterpret_cast<void (*)()>(result_dlsym.value());
      (void)(init_function);
      // TODO: call init_function in tracee.
    }
  }

  return outcome::success();
}

}  // namespace orbit_api
