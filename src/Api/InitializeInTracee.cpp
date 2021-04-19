// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Api/InitializeInTracee.h"

#include <absl/base/casts.h>

#include <functional>

#include "Api/Orbit.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

using orbit_grpc_protos::ApiFunction;
using orbit_grpc_protos::CaptureOptions;
using orbit_user_space_instrumentation::AttachAndStopProcess;
using orbit_user_space_instrumentation::DetachAndContinueProcess;
using orbit_user_space_instrumentation::DlopenInTracee;
using orbit_user_space_instrumentation::DlsymInTracee;
using orbit_user_space_instrumentation::ExecuteInProcess;

namespace orbit_api {
ErrorMessageOr<void> InitializeInTracee(const CaptureOptions& capture_options) {
  if (capture_options.api_functions().size() == 0) {
    return ErrorMessage("No api table to initilize.");
  }

  int32_t pid = capture_options.pid();

  OUTCOME_TRY(AttachAndStopProcess(pid));

  // Make sure we resume the target process, even on early-outs.
  orbit_base::unique_resource scope_exit{pid, [](int32_t pid) {
                                           if (DetachAndContinueProcess(pid).has_error())
                                             ERROR("Detaching from %i", pid);
                                         }};

  // Load liborbit.so and find api table initialization function.
  const std::string kLibPath = orbit_base::GetExecutableDir() / "liborbit.so";
  constexpr const char* kInitFunction = "orbit_api_initialize";
  OUTCOME_TRY(handle, DlopenInTracee(pid, kLibPath));
  OUTCOME_TRY(orbit_init_function, DlsymInTracee(pid, handle, kInitFunction));

  // Initialize all api function tables.
  for (const ApiFunction& api_function : capture_options.api_functions()) {
    // Get address of function table
    void* api_function_address = absl::bit_cast<void*>(api_function.file_offset());  // TODO: fix
    OUTCOME_TRY(function_table_address, ExecuteInProcess(pid, api_function_address));
    OUTCOME_TRY(ExecuteInProcess(pid, orbit_init_function, function_table_address,
                                 api_function.api_version()));
  }

  return outcome::success();
}

}  // namespace orbit_api
