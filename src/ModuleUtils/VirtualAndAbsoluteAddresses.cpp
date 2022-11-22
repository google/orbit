// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"

#include "OrbitBase/Align.h"
#include "OrbitBase/Logging.h"

namespace orbit_module_utils {

uint64_t SymbolVirtualAddressToAbsoluteAddress(uint64_t symbol_address,
                                               uint64_t module_base_address,
                                               uint64_t module_load_bias,
                                               uint64_t module_executable_section_offset) {
  ORBIT_CHECK((module_base_address % kPageSize) == 0);
  ORBIT_CHECK((module_load_bias % kPageSize) == 0);
  return symbol_address + module_base_address - module_load_bias -
         orbit_base::AlignDown<kPageSize>(module_executable_section_offset);
}

uint64_t SymbolAbsoluteAddressToVirtualAddress(uint64_t absolute_address,
                                               uint64_t module_base_address,
                                               uint64_t module_load_bias,
                                               uint64_t module_executable_section_offset) {
  ORBIT_CHECK((module_base_address % kPageSize) == 0);
  ORBIT_CHECK((module_load_bias % kPageSize) == 0);
  ORBIT_CHECK(absolute_address >=
              (module_base_address + module_executable_section_offset % kPageSize));
  return absolute_address - module_base_address + module_load_bias +
         orbit_base::AlignDown<kPageSize>(module_executable_section_offset);
}

}  // namespace orbit_module_utils