// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TEST_UTILS_H_
#define USER_SPACE_INSTRUMENTATION_TEST_UTILS_H_

#include <absl/types/span.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "UserSpaceInstrumentation/AddressRange.h"

namespace orbit_user_space_instrumentation {

struct FunctionLocation {
  std::string module_file_path;
  AddressRange relative_address_range;
};

// Returns the relative address of the function `function_name` and the corresponding module
// file path in the test process.
[[nodiscard]] FunctionLocation FindFunctionOrDie(std::string_view function_name);

// Returns the absolute virtual memory address range of function `function_name` in the test
// executable.
AddressRange GetFunctionAbsoluteAddressRangeOrDie(std::string_view function_name);

// This is for debugging only. Disassembles the `code` and dumps it into the log. `start_address` is
// the address of the code in virtual memory. If this is not applicable or you don't have it just
// hand over zero.
void DumpDisassembly(absl::Span<const uint8_t> code, uint64_t start_address);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_TEST_UTILS_H_