// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_
#define USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_

#include <sys/types.h>

#include <cstdint>
#include <filesystem>
#include <string_view>

#include "MachineCode.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Open, use, and close dynamic library in the tracee. The functions here resemble the respective
// functions offered by libdl as documented e.g. here: https://linux.die.net/man/3/dlopen. We rely
// on either libdl or libc being loaded into the tracee.
[[nodiscard]] ErrorMessageOr<void*> DlopenInTracee(pid_t pid, std::filesystem::path path,
                                                   uint32_t flag);
[[nodiscard]] ErrorMessageOr<void*> DlsymInTracee(pid_t pid, void* handle, std::string_view symbol);
[[nodiscard]] ErrorMessageOr<void> DlcloseInTracee(pid_t pid, void* handle);

// Returns the absolute virtual address of a function in a module of a process as resolved by the
// dynsym section of the file that module is associated with.
// The function name has to match the symbol name exactly. The module name needs match the soname
// (compare https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html) of the module
// exactly.
[[nodiscard]] ErrorMessageOr<uint64_t> FindFunctionAddress(pid_t pid,
                                                           std::string_view function_name,
                                                           std::string_view module_soname);

// Copies `code` to `address_code` in the tracee and executes it. The memory at `address_code` needs
// to be allocated using AllocateInTracee. The code segment has to end with an `int3`. Takes care of
// backup and restore of register state in the tracee and also deallocates the memory at
// `address_code` afterwards. The return value is the content of rax after the execution finished.
[[nodiscard]] ErrorMessageOr<uint64_t> ExecuteMachineCode(pid_t pid, uint64_t address_code,
                                                          uint64_t memory_size,
                                                          const MachineCode& code);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_INJECT_LIBRARY_IN_TRACEE_H_