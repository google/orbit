// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_DWARF_UTILS_H_
#define OBJECT_UTILS_DWARF_UTILS_H_

#include <llvm/DebugInfo/DWARF/DWARFDie.h>

#include <string>

namespace orbit_object_utils {

// For a given DWARF subprogram or subroutine, this function computes a string of the parameter list
// including opening and closing parentheses.
std::string DwarfParameterListToString(const llvm::DWARFDie& function_die);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_DWARF_UTILS_H_
