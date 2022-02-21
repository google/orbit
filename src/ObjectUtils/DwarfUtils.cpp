// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/DwarfUtils.h"

#include <absl/strings/str_cat.h>

#include "OrbitBase/Logging.h"

namespace orbit_object_utils {

namespace {
// This function converts a type `DWARFDie` node into a string representation. Currently, it only
// handles types that may occur as a type attribute of a DW_TAG_formal_parameter and NOT all
// possible type Dies from the specification (https://dwarfstd.org/doc/DWARF5.pdf).
std::string DwarfTypeDieToString(const llvm::DWARFDie& type_die) {
  ORBIT_CHECK(type_die.isValid());

  // Some Die nodes contain a name directly (e.g. base types or typedefs). We can use that name.
  if (const char* name = type_die.getName(llvm::DINameKind::LinkageName); name != nullptr) {
    return {name};
  }

  std::string result{};
  const llvm::dwarf::Tag tag = type_die.getTag();

  // Add the type modifiers that occur as prefix, such as "const" or "volatile". We need to identify
  // those by their tag.
  // Note that we only handle the modifiers that can occur in C/C++ code here (for further details
  // see section 5.3 in the DWARF spec: https://dwarfstd.org/doc/DWARF5.pdf).
  switch (tag) {
    case llvm::dwarf::DW_TAG_atomic_type:
      absl::StrAppend(&result, "atomic ");
      break;
    case llvm::dwarf::DW_TAG_const_type:
      absl::StrAppend(&result, "const ");
      break;
    case llvm::dwarf::DW_TAG_volatile_type:
      absl::StrAppend(&result, "volatile ");
      break;
    default:
      break;
  }

  // Check if the current Die is a container of another type (like const T, T*, T&). If so, we
  // compute the string representation of the Die at the "type" attribute.
  const llvm::DWARFDie type_attribute =
      type_die.getAttributeValueAsReferencedDie(llvm::dwarf::DW_AT_type);
  if (type_attribute.isValid()) {
    absl::StrAppend(&result, DwarfTypeDieToString(type_attribute));
  }

  // Add the postfixes for type modifiers and subroutines. Note, that we only handle types that
  // may occur as formal parameter.
  switch (tag) {
    case llvm::dwarf::DW_TAG_array_type:
      type_die.dump();
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
      // void subroutines does not have a type attribute, which would have been appended above,
      // so we add "void" explicitly.
      if (!type_attribute.isValid()) {
        absl::StrAppend(&result, "void");
      }
      absl::StrAppend(&result, DwarfParameterListToString(type_die));

      break;
    default:
      break;
  }

  return result;
}
}  // namespace

std::string DwarfParameterListToString(const llvm::DWARFDie& function_die) {
  ORBIT_CHECK(function_die.getTag() == llvm::dwarf::DW_TAG_subprogram ||
              function_die.getTag() == llvm::dwarf::DW_TAG_inlined_subroutine ||
              function_die.getTag() == llvm::dwarf::DW_TAG_subroutine_type);
  std::string result;

  absl::StrAppend(&result, "(");

  bool is_first_parameter = true;

  for (const llvm::DWARFDie& child : function_die.children()) {
    if (child.getTag() != llvm::dwarf::DW_TAG_formal_parameter) continue;
    if (!is_first_parameter) {
      absl::StrAppend(&result, ", ");
    }
    is_first_parameter = false;

    const llvm::DWARFDie parameter_type =
        child.getAttributeValueAsReferencedDie(llvm::dwarf::DW_AT_type);
    if (!parameter_type.isValid()) {
      // For some C functions, we don't get the type of the parameter. There is not a lot that we
      // can do about this.
      absl::StrAppend(&result, "???");
      continue;
    }
    absl::StrAppend(&result, DwarfTypeDieToString(parameter_type));
  }

  absl::StrAppend(&result, ")");

  return result;
}
}  // namespace orbit_object_utils