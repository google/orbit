// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Api/InitializeInTracee.h"

#include <functional>

#include "Api/Orbit.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/UniqueResource.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

using namespace orbit_user_space_instrumentation;
using orbit_grpc_protos::ApiTableInfo;
using orbit_grpc_protos::CaptureOptions;

namespace orbit_api {
ErrorMessageOr<void> InitializeInTracee(const CaptureOptions& capture_options) {
  if (capture_options.api_table_infos().size() == 0) {
    return ErrorMessage("No api table to initilize.");
  }

  ErrorMessageOr<void> result = outcome::success();
  int32_t pid = capture_options.pid();

  OUTCOME_TRY(AttachAndStopProcess(pid));
  {
    // Make sure we resume the target process, even on early-outs.
    orbit_base::unique_resource scope_exit{
        pid, [&result](int32_t pid) { result = DetachAndContinueProcess(pid); }};

    // Load liborbit.so and find api table initialization function.
    const std::string kLibPath = orbit_base::GetExecutableDir() / "liborbit.so";
    const char* kInitFunction = "orbit_api_initialize";
    OUTCOME_TRY(handle, DlopenInTracee(pid, kLibPath, RTLD_NOW));
    OUTCOME_TRY(address, DlsymInTracee(pid, handle, kInitFunction));

    // Initialize all api function tables.
    for (const ApiTableInfo& info : capture_options.api_table_infos()) {
      OUTCOME_TRY(ExecuteInProcess(pid, address, info.api_table_address(), info.api_version()));
    }
  }

  return result;
}

}  // namespace orbit_api
