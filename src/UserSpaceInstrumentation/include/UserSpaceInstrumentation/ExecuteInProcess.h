// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_EXECUTE_IN_PROCESS_H_
#define USER_SPACE_INSTRUMENTATION_EXECUTE_IN_PROCESS_H_

#include <absl/types/span.h>
#include <sys/types.h>

#include <cstdint>
#include <string_view>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Execute function at `function_address` with the given parameters inside process `pid`. `function`
// can be any function taking up to six integer parameter and might return an integer.
// Assumes that we are attached to the process `pid` using `AttachAndStopProcess`.
// `function_address` should be a result of `DlsymInTracee`.
[[nodiscard]] ErrorMessageOr<uint64_t> ExecuteInProcess(pid_t pid, void* function_address,
                                                        uint64_t param_1 = 0, uint64_t param_2 = 0,
                                                        uint64_t param_3 = 0, uint64_t param_4 = 0,
                                                        uint64_t param_5 = 0, uint64_t param_6 = 0);

// As above but the function to be called is identified by the handle to the library and its name.
// Assumes that the library identified by `library_handle` is loaded into this process using
// `DlopenInTracee`.
[[nodiscard]] ErrorMessageOr<uint64_t> ExecuteInProcess(
    pid_t pid, absl::Span<const orbit_grpc_protos::ModuleInfo> modules, void* library_handle,
    std::string_view function, uint64_t param_1 = 0, uint64_t param_2 = 0, uint64_t param_3 = 0,
    uint64_t param_4 = 0, uint64_t param_5 = 0, uint64_t param_6 = 0);

// Like ExecuteInProcess, but the function is assumed to have Microsoft x64 calling convention. This
// can be used when the target process was built for Windows (e.g., it is running under Wine).
// We only allow four parameters for now as the Microsoft x64 calling convention only passes four in
// registers, and none of the functions that we are going to call accept more than four.
[[nodiscard]] ErrorMessageOr<uint64_t> ExecuteInProcessWithMicrosoftCallingConvention(
    pid_t pid, void* function_address, uint64_t param_1 = 0, uint64_t param_2 = 0,
    uint64_t param_3 = 0, uint64_t param_4 = 0);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_EXECUTE_IN_PROCESS_H_