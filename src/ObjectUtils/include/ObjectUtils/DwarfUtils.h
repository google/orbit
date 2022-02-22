// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_DWARF_UTILS_H_
#define OBJECT_UTILS_DWARF_UTILS_H_

#include <absl/strings/str_cat.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>

#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_object_utils {

// This function converts a type `DWARFDie` node into a string representation. Currently, it only
// handles types that may occur as a type attribute of a DW_TAG_formal_parameter and NOT all
// possible type DIEs from the specification (https://dwarfstd.org/doc/DWARF5.pdf).
template <class DWARFDie>
[[nodiscard]] std::string DwarfTypeAsString(const DWARFDie& type_die);

// For a given DWARF subprogram or subroutine, this function computes a string of the parameter list
// including opening and closing parentheses.
template <class DWARFDie>
[[nodiscard]] std::string DwarfParameterListAsString(const DWARFDie& function_die);

template <class DWARFDie>
std::string DwarfTypeAsString(const DWARFDie& type_die) {
  ORBIT_CHECK(type_die.isValid());

  // Some DIEs nodes contain a name directly (e.g. base types or typedefs). We can use that name.
  if (const char* name = type_die.getName(llvm::DINameKind::LinkageName); name != nullptr) {
    return {name};
  }

  std::string result{};

  // Check if the current DIEs is a container of another type (like T const, T*, T&). If so, we
  // compute the string representation of the DIEs at the "type" attribute.
  const DWARFDie type_attribute =
      type_die.getAttributeValueAsReferencedDie(llvm::dwarf::DW_AT_type);
  if (type_attribute.isValid()) {
    absl::StrAppend(&result, DwarfTypeAsString(type_attribute));
  }

  // Note, we are using "East const" notation, to reduce complexity and errors
  // (see: https://hackingcpp.com/cpp/design/east_vs_west_const.html). Therefore, we are simply
  // appending the type modifiers, like "const", "*", or "&".
  // Also see the example in the DWARF specification in chapter 5.3
  // (https://dwarfstd.org/doc/DWARF5.pdf).
  // Further, note hat we only handle types that may occur as formal parameter.
  switch (type_die.getTag()) {
    case llvm::dwarf::DW_TAG_atomic_type:
      absl::StrAppend(&result, " _Atomic");
      break;
    case llvm::dwarf::DW_TAG_const_type:
      absl::StrAppend(&result, " const");
      break;
    case llvm::dwarf::DW_TAG_volatile_type:
      absl::StrAppend(&result, " volatile");
      break;
    case llvm::dwarf::DW_TAG_restrict_type:
      absl::StrAppend(&result, " restrict");
      break;
    case llvm::dwarf::DW_TAG_array_type:
      // We could do better for array types, as e.g. the exact size might be known here.
      absl::StrAppend(&result, "[]");
      break;
    case llvm::dwarf::DW_TAG_pointer_type:
      absl::StrAppend(&result, "*");
      break;
    case llvm::dwarf::DW_TAG_reference_type:
      absl::StrAppend(&result, "&");
      break;
    case llvm::dwarf::DW_TAG_rvalue_reference_type:
      absl::StrAppend(&result, "&&");
      break;
    case llvm::dwarf::DW_TAG_subroutine_type:
      // void subroutines do not have a type attribute, which would have been appended above, so we
      // add "void" explicitly.
      if (!type_attribute.isValid()) {
        absl::StrAppend(&result, "void");
      }
      absl::StrAppend(&result, DwarfParameterListAsString(type_die));
      break;
    default:
      break;
  }

  return result;
}

template <class DWARFDie>
std::string DwarfParameterListAsString(const DWARFDie& function_die) {
  ORBIT_CHECK(function_die.getTag() == llvm::dwarf::DW_TAG_subprogram ||
              function_die.getTag() == llvm::dwarf::DW_TAG_inlined_subroutine ||
              function_die.getTag() == llvm::dwarf::DW_TAG_subroutine_type);
  std::string result;

  absl::StrAppend(&result, "(");

  bool is_first_parameter = true;

  for (const DWARFDie& child : function_die.children()) {
    if (child.getTag() != llvm::dwarf::DW_TAG_formal_parameter) continue;
    if (!is_first_parameter) {
      absl::StrAppend(&result, ", ");
    }
    is_first_parameter = false;

    const DWARFDie parameter_type = child.getAttributeValueAsReferencedDie(llvm::dwarf::DW_AT_type);
    if (!parameter_type.isValid()) {
      // For some C functions, we don't get the type of the parameter. There is not a lot that we
      // can do about this.
      absl::StrAppend(&result, "???");
      continue;
    }
    absl::StrAppend(&result, DwarfTypeAsString(parameter_type));
  }

  absl::StrAppend(&result, ")");

  return result;
}

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_DWARF_UTILS_H_
