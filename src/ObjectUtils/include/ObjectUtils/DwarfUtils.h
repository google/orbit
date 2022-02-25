// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_DWARF_UTILS_H_
#define OBJECT_UTILS_DWARF_UTILS_H_

#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>

#include <deque>
#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_object_utils {

// This function converts a type `DWARFDie` node into a string representation. Currently, it only
// handles types that may occur as a type attribute of a DW_TAG_formal_parameter and NOT all
// possible type DIEs from the specification (https://dwarfstd.org/doc/DWARF5.pdf).
template <class DWARFDie>
[[nodiscard]] std::string DwarfTypeAsString(const DWARFDie& type_die,
                                            std::deque<std::string> pre_type_modifiers = {},
                                            std::string post_type_modifier_str = "");

// For a given DWARF subprogram or subroutine, this function computes a string of the parameter list
// including opening and closing parentheses.
template <class DWARFDie>
[[nodiscard]] std::string DwarfParameterListAsString(const DWARFDie& function_die);

template <class DWARFDie>
std::string HandlePreTypeModifier(const std::deque<std::string>& pre_type_modifiers,
                                  const std::string& post_type_modifier_str,
                                  const DWARFDie& type_attribute) {
  if (type_attribute.isValid()) {
    return DwarfTypeAsString(type_attribute, pre_type_modifiers, post_type_modifier_str);
  }
  if (pre_type_modifiers.empty()) {
    return absl::StrCat("void", post_type_modifier_str);
  }
  std::string pre_type_modifiers_str = absl::StrJoin(pre_type_modifiers, " ");
  return absl::StrCat(pre_type_modifiers_str, " ", "void", post_type_modifier_str);
}

template <class DWARFDie>
std::string HandlePostModifier(const std::deque<std::string>& pre_type_modifiers,
                               const std::string& post_type_modifier_str,
                               const DWARFDie& type_attribute) {
  if (type_attribute.isValid()) {
    std::string type_string = DwarfTypeAsString(type_attribute, {}, post_type_modifier_str);
    if (pre_type_modifiers.empty()) {
      return type_string;
    }
    std::string pre_type_modifiers_str = absl::StrJoin(pre_type_modifiers, " ");
    return absl::StrCat(type_string, " ", pre_type_modifiers_str);
  }
  if (pre_type_modifiers.empty()) {
    return absl::StrCat("void", post_type_modifier_str);
  }
  std::string pre_type_modifiers_str = absl::StrJoin(pre_type_modifiers, " ");
  std::string type_str = absl::StrCat("void", post_type_modifier_str);

  return absl::StrJoin({type_str, pre_type_modifiers_str}, " ");
}

template <class DWARFDie>
std::string DwarfTypeAsString(const DWARFDie& type_die, std::deque<std::string> pre_type_modifiers,
                              std::string post_type_modifier_str) {
  type_die.dump();
  ORBIT_CHECK(type_die.isValid());

  // Some DIEs nodes contain a name directly (e.g. base types or typedefs). We can use that name.
  if (const char* name = type_die.getName(llvm::DINameKind::LinkageName); name != nullptr) {
    if (pre_type_modifiers.empty()) {
      return absl::StrCat({name}, post_type_modifier_str);
    }
    std::string pre_type_modifiers_str = absl::StrJoin(pre_type_modifiers, " ");
    return absl::StrCat(pre_type_modifiers_str, " ", name, post_type_modifier_str);
  }

  const DWARFDie type_attribute =
      type_die.getAttributeValueAsReferencedDie(llvm::dwarf::DW_AT_type);

  // Note, we are using "west const" notation, so we need to distinguish between type modifiers like
  // "const", that get prepended to the type string, and modifiers like pointers, that get appended
  // as well as define a group, such that the previous "pre" modifiers get prepended and the get
  // cleared in the next level (see: https://hackingcpp.com/cpp/design/east_vs_west_const.html).
  // Also see the example in the DWARF specification in chapter 5.3
  // (https://dwarfstd.org/doc/DWARF5.pdf).
  // Further, note hat we only handle types that may occur as formal parameter.
  switch (type_die.getTag()) {
    case llvm::dwarf::DW_TAG_atomic_type:
      pre_type_modifiers.push_back("_Atomic");
      return HandlePreTypeModifier(pre_type_modifiers, post_type_modifier_str, type_attribute);
    case llvm::dwarf::DW_TAG_const_type:
      pre_type_modifiers.push_back("const");
      return HandlePreTypeModifier(pre_type_modifiers, post_type_modifier_str, type_attribute);
    case llvm::dwarf::DW_TAG_volatile_type:
      pre_type_modifiers.push_back("volatile");
      return HandlePreTypeModifier(pre_type_modifiers, post_type_modifier_str, type_attribute);
    case llvm::dwarf::DW_TAG_restrict_type: {
      pre_type_modifiers.push_back("restrict");
      return HandlePreTypeModifier(pre_type_modifiers, post_type_modifier_str, type_attribute);
    }
    case llvm::dwarf::DW_TAG_pointer_type:
      return HandlePostModifier(pre_type_modifiers, absl::StrCat("*", post_type_modifier_str),
                                type_attribute);
    case llvm::dwarf::DW_TAG_array_type:
      // We could do better for array types, as e.g. the exact size might be known here.
      return HandlePostModifier(pre_type_modifiers, absl::StrCat("[]", post_type_modifier_str),
                                type_attribute);
    case llvm::dwarf::DW_TAG_ptr_to_member_type: {
      std::string pointer_str;
      if (DWARFDie containing_type =
              type_die.getAttributeValueAsReferencedDie(llvm::dwarf::DW_AT_containing_type);
          containing_type.isValid()) {
        pointer_str = absl::StrCat(DwarfTypeAsString(containing_type), "::");
      }
      absl::StrAppend(&pointer_str, "*");
      return HandlePostModifier(pre_type_modifiers,
                                absl::StrCat(pointer_str, post_type_modifier_str), type_attribute);
    }
    case llvm::dwarf::DW_TAG_reference_type:
      return HandlePostModifier(pre_type_modifiers, absl::StrCat("&", post_type_modifier_str),
                                type_attribute);
    case llvm::dwarf::DW_TAG_rvalue_reference_type: {
      return HandlePostModifier(pre_type_modifiers, absl::StrCat("&&", post_type_modifier_str),
                                type_attribute);
    }
    case llvm::dwarf::DW_TAG_subroutine_type: {
      // void subroutines do not have a type attribute, which would have been appended above, so
      // we add "void" explicitly.
      std::string return_type =
          (type_attribute.isValid()) ? DwarfTypeAsString(type_attribute) : "void";
      std::string parameter_list = DwarfParameterListAsString(type_die);
      if (pre_type_modifiers.empty()) {
        return absl::StrCat(return_type, "(", post_type_modifier_str, ")", parameter_list);
      }
      std::string pre_type_modifiers_str = absl::StrJoin(pre_type_modifiers, " ");
      return absl::StrCat(return_type, "(", post_type_modifier_str, " ", pre_type_modifiers_str,
                          ")", parameter_list);
    }
    default:
      ORBIT_UNREACHABLE();
  }
}

template <class DWARFDie>
std::string DwarfParameterListAsString(const DWARFDie& function_die) {
  function_die.dump();
  ORBIT_CHECK(function_die.getTag() == llvm::dwarf::DW_TAG_subprogram ||
              function_die.getTag() == llvm::dwarf::DW_TAG_inlined_subroutine ||
              function_die.getTag() == llvm::dwarf::DW_TAG_subroutine_type);
  std::string result;

  absl::StrAppend(&result, "(");

  bool is_first_parameter = true;

  for (const DWARFDie& child : function_die.children()) {
    child.dump();
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
