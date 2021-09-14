// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_ADDRESS_H_
#define OBJECT_UTILS_ADDRESS_H_

#include <stdint.h>

namespace orbit_object_utils {

// Since this module is used on the client and on the service side and we do not currently
// report page size in capture this is hardcoded here.
static constexpr uint64_t kPageSize = 0x1000;

[[nodiscard]] uint64_t SymbolVirtualAddressToAbsoluteAddress(
    uint64_t symbol_address, uint64_t module_base_address, uint64_t module_load_bias,
    uint64_t module_executable_section_offset);

[[nodiscard]] uint64_t SymbolOffsetToAbsoluteAddress(uint64_t symbol_address,
                                                     uint64_t module_base_address,
                                                     uint64_t module_executable_section_offset);

[[nodiscard]] uint64_t SymbolAbsoluteAddressToOffset(uint64_t absolute_address,
                                                     uint64_t module_base_address,
                                                     uint64_t module_executable_section_offset);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_ADDRESS_H_
