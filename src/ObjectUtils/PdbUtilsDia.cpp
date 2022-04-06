// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/PdbUtilsDia.h"

#include <Windows.h>
#include <atlbase.h>
#include <cguid.h>
// Don't allow "shlwapi.h" to rename StrCat. See:
// https://github.com/abseil/abseil-cpp/issues/377
#undef StrCat

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "OrbitBase/GetLastError.h"
#include "OrbitBase/StringConversion.h"

namespace {
[[nodiscard]] ErrorMessageOr<std::string> GetSignedIntegerTypeFromSizeInBytes(IDiaSymbol* type) {
  ULONGLONG length;
  if (FAILED(type->get_length(&length))) {
    return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_length");
  }

  switch (length) {
    case 1:
      return "char";
    case 2:
      return "short";
    case 4:
      return "int";
    case 8:
      return "__int64";
    default:
      return ErrorMessage(absl::StrFormat("Unexpected size of integer: %lu", length));
  }
}

[[nodiscard]] ErrorMessageOr<std::string> GetBaseTypeAsString(IDiaSymbol* type) {
  DWORD base_type;
  if (FAILED(type->get_baseType(&base_type))) {
    return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_baseType");
  }

  switch (base_type) {
    case btNoType:
      return "<no type>";  // 0
    case btVoid:
      return "void";  // 1
    case btChar:
      return "char";  // 2
    case btWChar:
      return "wchar_t";  // 3
    case btInt:          // 6
    {
      OUTCOME_TRY(auto&& integer_type, GetSignedIntegerTypeFromSizeInBytes(type));
      return integer_type;
    }
    case btUInt:  // 7
    {
      OUTCOME_TRY(auto&& integer_type, GetSignedIntegerTypeFromSizeInBytes(type));
      return absl::StrCat("unsigned ", integer_type);
    }
    case btFloat:  // 8
    {
      ULONGLONG length;
      if (FAILED(type->get_length(&length))) {
        return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_length");
      }

      switch (length) {
        case 4:
          return "float";
        case 8:
          return "double";
        default:
          ORBIT_UNREACHABLE();
      }
    }
    case btBCD:
      return "<BCD>";  // 9
    case btBool:
      return "bool";  // 10
    case btLong:
      return "long";  // 13
    case btULong:
      return "unsigned long";  // 14
    case btCurrency:
      return "<currency>";  // 25
    case btDate:
      return "<date>";  // 26
    case btVariant:
      return "VARIANT";  // 27
    case btComplex:
      return "<complex>";  // 28
    case btBit:
      return "<bit>";  // 29
    case btBSTR:
      return "BSTR";  // 30
    case btHresult:
      return "HRESULT";  // 31
    case btChar16:
      return "char16_t";  // 32
    case btChar32:
      return "char32_t";  // 33
    case btChar8:
      return "char8_t";  // 34
    default:
      return ErrorMessage(absl::StrFormat("Unexpected base type with id \"%d\".", base_type));
  }
}

[[nodiscard]] std::string GetTypeModifiersAsString(IDiaSymbol* type) {
  std::vector<std::string_view> type_modifiers;
  if (BOOL is_const; SUCCEEDED(type->get_constType(&is_const)) && is_const) {
    type_modifiers.push_back("const");
  }
  if (BOOL is_volatile; SUCCEEDED(type->get_volatileType(&is_volatile)) && is_volatile) {
    type_modifiers.push_back("volatile");
  }
  if (BOOL is_unaligned; SUCCEEDED(type->get_unalignedType(&is_unaligned)) && is_unaligned) {
    type_modifiers.push_back("__unaligned");
  }
  if (BOOL is_restricted; SUCCEEDED(type->get_restrictedType(&is_restricted)) && is_restricted) {
    type_modifiers.push_back("restricted");
  }

  return absl::StrJoin(type_modifiers, " ");
}

[[nodiscard]] ErrorMessageOr<std::string> GetPointerTypeAsString(
    IDiaSymbol* type, std::string_view parent_pointer_type_str) {
  CComPtr<IDiaSymbol> base_type = nullptr;
  if (FAILED(type->get_type(&base_type))) {
    return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_type");
  }
  if (base_type == nullptr) {
    return ErrorMessage("Unable to retrieve type symbol.");
  }

  std::string new_pointer_type_str;
  if (BOOL is_reference; SUCCEEDED(type->get_reference(&is_reference)) && is_reference) {
    new_pointer_type_str = "&";
  } else if (BOOL is_rvalue_reference;
             SUCCEEDED(type->get_RValueReference(&is_rvalue_reference)) && is_rvalue_reference) {
    new_pointer_type_str = "&&";
  } else if (BOOL is_ptr_to_member_function;
             SUCCEEDED(type->get_isPointerToMemberFunction(&is_ptr_to_member_function)) &&
             is_ptr_to_member_function) {
    CComPtr<IDiaSymbol> class_parent = nullptr;
    if (FAILED(type->get_classParent(&class_parent))) {
      return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_classParent");
    }
    if (class_parent == nullptr) {
      return ErrorMessage("Unable to retrieve class parent symbol.");
    }
    OUTCOME_TRY(auto&& class_parent_str, orbit_object_utils::PdbDiaTypeAsString(*&class_parent));
    new_pointer_type_str = class_parent_str + "::*";
  } else if (BOOL is_ptr_to_member_function;
             SUCCEEDED(type->get_isPointerToDataMember(&is_ptr_to_member_function)) &&
             is_ptr_to_member_function) {
    CComPtr<IDiaSymbol> class_parent = nullptr;
    if (FAILED(type->get_classParent(&class_parent))) {
      return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_classParent");
    }
    if (class_parent == nullptr) {
      return ErrorMessage("Unable to retrieve class parent symbol.");
    }
    OUTCOME_TRY(auto&& class_parent_str, orbit_object_utils::PdbDiaTypeAsString(*&class_parent));
    new_pointer_type_str = class_parent_str + "::*";
  } else {
    new_pointer_type_str = "*";
  }

  std::string type_modifiers = GetTypeModifiersAsString(type);
  if (!type_modifiers.empty()) {
    absl::StrAppend(&new_pointer_type_str, " ", type_modifiers);
  }

  absl::StrAppend(&new_pointer_type_str, parent_pointer_type_str);
  return orbit_object_utils::PdbDiaTypeAsString(*&base_type, new_pointer_type_str);
}

}  //  namespace

namespace orbit_object_utils {

ErrorMessageOr<std::string> PdbDiaParameterListAsString(IDiaSymbol* function_or_function_type) {
  DWORD tag;
  if (FAILED(function_or_function_type->get_symTag(&tag))) {
    return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_symTag");
  }
  if (tag == SymTagFunction) {
    CComPtr<IDiaSymbol> function_type = nullptr;
    if (FAILED(function_or_function_type->get_type(&function_type))) {
      return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_type");
    }
    if (function_type == nullptr) {
      return ErrorMessage("Unable to retrieve type symbol.");
    }
    return PdbDiaParameterListAsString(*&function_type);
  }
  // Some functions don't have a type (<no type>), which is a base type.
  // In this case, we don't show a parameter list (as this happens on C functions).
  if (tag == SymTagBaseType) {
    return "";
  }
  ORBIT_CHECK(tag == SymTagFunctionType);

  IDiaSymbol* function_type = function_or_function_type;

  CComPtr<IDiaEnumSymbols> parameter_enumeration = nullptr;
  if (FAILED(function_type->findChildren(SymTagNull, nullptr, nsNone, &parameter_enumeration))) {
    return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::findChildren");
  }
  if (parameter_enumeration == nullptr) {
    return ErrorMessage("Unable to find child symbols.");
  }

  std::string result_string = "(";

  CComPtr<IDiaSymbol> parameter = nullptr;
  ULONG celt = 0;
  bool is_first_parameter = true;
  while (SUCCEEDED(parameter_enumeration->Next(1, &parameter, &celt)) && (celt == 1) &&
         parameter != nullptr) {
    if (!is_first_parameter) {
      absl::StrAppend(&result_string, ", ");
    }

    CComPtr<IDiaSymbol> parameter_type = nullptr;
    if (FAILED(parameter->get_type(&parameter_type))) {
      return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_type");
    }
    if (parameter_type == nullptr) {
      return ErrorMessage("Unable to retrieve type symbol.");
    }

    OUTCOME_TRY(auto&& parameter_type_str, orbit_object_utils::PdbDiaTypeAsString(parameter_type));
    absl::StrAppend(&result_string, parameter_type_str);

    is_first_parameter = false;
  }

  return absl::StrCat(result_string, ")");
}

ErrorMessageOr<std::string> PdbDiaTypeAsString(IDiaSymbol* type,
                                               std::string_view parent_pointer_type_str) {
  HRESULT result;
  DWORD tag;
  if (FAILED(type->get_symTag(&tag))) {
    return ErrorMessage("Found Dia symbol without a tag.");
  }

  std::string result_string;
  if (tag != SymTagPointerType) {
    std::string type_modifiers = GetTypeModifiersAsString(type);
    if (!type_modifiers.empty()) {
      absl::StrAppend(&result_string, type_modifiers, " ");
    }
  }

  BSTR type_name_bstr = {};
  std::string type_name;
  if (SUCCEEDED(type->get_name(&type_name_bstr)) && type_name_bstr != NULL) {
    type_name = orbit_base::ToStdString(type_name_bstr);
    SysFreeString(type_name_bstr);
  }

  if (!type_name.empty()) {
    return absl::StrCat(result_string, type_name, parent_pointer_type_str);
  }

  switch (tag) {
    case SymTagArrayType: {
      // For now we only print "[]" for arrays. However there is room for improvement.
      // We could e.g. also print the size of the array if known.
      CComPtr<IDiaSymbol> base_type = nullptr;
      if (FAILED(type->get_type(&base_type))) {
        return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_type");
      }
      if (base_type == nullptr) {
        return ErrorMessage("Unable to retrieve type symbol.");
      }
      std::string new_pointer_type_str = absl::StrCat("[]", parent_pointer_type_str);
      OUTCOME_TRY(auto&& type_str, PdbDiaTypeAsString(*&base_type, new_pointer_type_str));
      return absl::StrCat(result_string, type_str);
    }
    case SymTagBaseType: {
      OUTCOME_TRY(auto&& type_str, GetBaseTypeAsString(type));
      return absl::StrCat(result_string, type_str, parent_pointer_type_str);
    }
    case SymTagPointerType: {
      OUTCOME_TRY(auto&& type_str, GetPointerTypeAsString(type, parent_pointer_type_str));
      return absl::StrCat(result_string, type_str);
    }
    case SymTagFunctionType: {
      CComPtr<IDiaSymbol> return_type = nullptr;
      if (FAILED(type->get_type(&return_type))) {
        return orbit_base::GetLastErrorAsErrorMessage("IDiaSymbol::get_type");
      }
      if (return_type == nullptr) {
        return ErrorMessage("Unable to retrieve type symbol.");
      }
      OUTCOME_TRY(auto&& return_type_str, PdbDiaTypeAsString(*&return_type));
      absl::StrAppend(&result_string, return_type_str, " (", parent_pointer_type_str, ")");
      OUTCOME_TRY(auto&& parameter_list, PdbDiaParameterListAsString(*&type));
      return absl::StrCat(result_string, parameter_list);
    }
    default:
      return ErrorMessage(absl::StrFormat("Unexpected tag \"%d\".", tag));
  }
}

}  // namespace orbit_object_utils
