// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_
#define USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_

#include <sys/types.h>

#include <cstdint>
#include <filesystem>
#include <string_view>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Open, use, and close dynamic library in the tracee. The functions here resemble the respective
// functions offered by libdl as documented e.g. here: https://linux.die.net/man/3/dlopen. We rely
// on libc being loaded into the tracee.
[[nodiscard]] ErrorMessageOr<void*> DlopenInTracee(pid_t pid, std::filesystem::path path,
                                                   uint32_t flag);
[[nodiscard]] ErrorMessageOr<void*> DlsymInTracee(pid_t pid, void* handle, std::string symbol);
[[nodiscard]] ErrorMessageOr<void> DlcloseInTracee(pid_t pid, void* handle);

// Returns the absolute virtual address of a function in a module of a process as resolved by the
// dynsym section of the file that module is associated with.
// Matching of the module and the function names is done by checking whether the given paramter is
// contained in the actual name. The first matching function from the first matching module is
// returned (or an error).
[[nodiscard]] ErrorMessageOr<uint64_t> FindFunctionAddress(pid_t pid, std::string_view function,
                                                           std::string_view module);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_