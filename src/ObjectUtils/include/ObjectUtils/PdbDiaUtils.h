// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_PDB_DIA_UTILS_H_
#define OBJECT_UTILS_PDB_DIA_UTILS_H_

#include <dia2.h>

#include <string>

#include "OrbitBase/Logging.h"

// Provides utility functions to retrieve a functions parameter types as string.
namespace orbit_object_utils {

// Retrieves a string representing the parameter list of the given function or
// function type. For functions with function type "<no type>" this returns an
// empty string.
[[nodiscard]] ErrorMessageOr<std::string> PdbDiaParameterListAsString(
    IDiaSymbol* function_or_function_type);

// Retrieves a string representation of the given type symbol. It will prepend the
// given `pointer_type_string` to the result string, or in case of a function pointer,
// add it inside the parentheses (e.g. void(*)(int, int)).
// If the type can't be computed properly, "???" will be returned.
[[nodiscard]] ErrorMessageOr<std::string> PdbDiaTypeAsString(
    IDiaSymbol* type, std::string_view pointer_type_str = "");

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_PDB_DIA_UTILS_H_
