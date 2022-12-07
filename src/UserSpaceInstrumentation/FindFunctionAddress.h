// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_FIND_FUNCTION_ADDRESS_H_
#define USER_SPACE_INSTRUMENTATION_FIND_FUNCTION_ADDRESS_H_

#include <absl/types/span.h>
#include <sys/types.h>

#include <cstdint>
#include <string_view>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Returns the absolute virtual address of a function in a module of a process as resolved by the
// dynsym section of the file that module is associated with.
// The function name has to match the symbol name exactly. The module name needs match the soname
// (compare https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html) of the module
// exactly.
[[nodiscard]] ErrorMessageOr<uint64_t> FindFunctionAddress(
    absl::Span<const orbit_grpc_protos::ModuleInfo> modules, std::string_view module_soname,
    std::string_view function_name);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_FIND_FUNCTION_ADDRESS_H_