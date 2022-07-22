// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_
#define USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_

#include <dlfcn.h>  // IWYU pragma: keep
#include <sys/types.h>

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Open, use, and close dynamic library in the tracee. The functions here resemble the respective
// functions offered by libdl as documented e.g. here: https://linux.die.net/man/3/dlopen. We rely
// on either libdl or libc being loaded into the tracee.
[[nodiscard]] ErrorMessageOr<void*> DlopenInTracee(
    pid_t pid, const std::vector<orbit_grpc_protos::ModuleInfo>& modules,
    const std::filesystem::path& path, uint32_t flag);
[[nodiscard]] ErrorMessageOr<void*> DlsymInTracee(
    pid_t pid, const std::vector<orbit_grpc_protos::ModuleInfo>& modules, void* handle,
    std::string_view symbol);
[[nodiscard]] ErrorMessageOr<void> DlcloseInTracee(
    pid_t pid, const std::vector<orbit_grpc_protos::ModuleInfo>& modules, void* handle);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_
