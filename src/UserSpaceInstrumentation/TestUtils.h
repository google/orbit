// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_DUMP_DISASSEMBLY_H_
#define USER_SPACE_INSTRUMENTATION_DUMP_DISASSEMBLY_H_

#include <cstdint>
#include <string_view>
#include <vector>

#include "AddressRange.h"

namespace orbit_user_space_instrumentation {

// Returns the file offset of function `function_name` in the test executable.
AddressRange GetFunctionAddressRangeInFileOrDie(std::string_view function_name);

// Returns the virtual memory address range of function `function_name` in the test executable.
AddressRange GetFunctionAddressRangeInMemoryOrDie(std::string_view function_name);

// This is for debugging only. Disassembles the `code` and dumps it into the log. `start_address` is
// the address of the code in virtual memory. If this is not applicable or you don't have it just
// hand over zero.
void DumpDisassembly(const std::vector<uint8_t>& code, uint64_t start_address);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_DUMP_DISASSEMBLY_H_