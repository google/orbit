// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/PdbDiaUtils.h"

#include <absl/strings/str_join.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"

namespace {
    [[nodiscard]] ErrorMessageOr<std::string> GetSignedIntegerTypeFromSizeInBytes(const ULONGLONG& length) {
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

    [[nodiscard]] ErrorMessageOr<std::string> GetBaseTypeAsString(IDiaSymbol* type, const std::string& parent_pointer_type_str)
    {
        DWORD base_type;
        if (FAILED(type->get_baseType(&base_type))) {
            return ErrorMessage(orbit_base::GetLastErrorAsString());
        }

        std::string result_string;
        switch (base_type) {
        case btNoType:  // 0
            result_string += "<no type>";
            break;
        case btVoid:  // 1
            result_string += "void";
            break;
        case btChar:  // 2
            result_string += "char";
            break;
        case btWChar:  // 3
            result_string += "wchar_t";
            break;
        case btInt:  // 6
        {
            ULONGLONG length;
            if (FAILED(type->get_length(&length))) {
                return ErrorMessage(orbit_base::GetLastErrorAsString());
            }
            OUTCOME_TRY(auto&& integer_type, GetSignedIntegerTypeFromSizeInBytes(length));
   
            result_string += integer_type;
            break;
        }
        case btUInt:  // 7
        {
            ULONGLONG length;
            if (FAILED(type->get_length(&length))) {
                return ErrorMessage(orbit_base::GetLastErrorAsString());
            }

            result_string += "unsigned ";

            OUTCOME_TRY(auto&& integer_type, GetSignedIntegerTypeFromSizeInBytes(length));

            result_string += integer_type;
            break;
        }
        case btFloat:  // 8
        {
            ULONGLONG length;
            if (FAILED(type->get_length(&length))) {
                return ErrorMessage(orbit_base::GetLastErrorAsString());
            }

            switch (length) {
            case 4:
                result_string += "float";
                break;
            case 8:
                result_string += "double";
                break;
            default:
                ORBIT_UNREACHABLE();
            }
            break;
        }
        case btBCD:  // 9
            result_string += "<BCD>";
            break;
        case btBool:  // 10
            result_string += "bool";
            break;
        case btLong:  // 13
            result_string += "long";
            break;
        case btULong:  // 14
            result_string += "unsigned long";
            break;
        case btCurrency:  // 25
            result_string += "<currency>";
            break;
        case btDate:  // 26
            result_string += "<date>";
            break;
        case btVariant:  // 27
            result_string += "VARIANT";
            break;
        case btComplex:  // 28
            result_string += "<complex>";
            break;
        case btBit:  // 29
            result_string += "<bit>";
            break;
        case btBSTR:  // 30
            result_string += "BSTR";
            break;
        case btHresult:  // 31
            result_string += "HRESULT";
            break;
        case btChar16:  // 32
            result_string += "char16_t";
            break;
        case btChar32:  // 33
            result_string += "char32_t";
            break;
        case btChar8:  // 34
            result_string += "char8_t";
            break;
        default:
            return ErrorMessage(absl::StrFormat("Unexpected base type with id \"%d\". Returning \"???\" as a type name.",
                base_type));
        }
        result_string += parent_pointer_type_str;
        return result_string;
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

    [[nodiscard]] ErrorMessageOr<std::string> GetPointerTypeAsString(IDiaSymbol* type, const std::string& parent_pointer_type_str) {
        CComPtr<IDiaSymbol> base_type = nullptr;
        if (FAILED(type->get_type(&base_type))) {
            return ErrorMessage(orbit_base::GetLastErrorAsString());
        }
        if (base_type == nullptr) {
            return ErrorMessage("Unable to retrieve type symbol.");
        }

        std::string new_pointer_type_str;
        if (BOOL is_reference; SUCCEEDED(type->get_reference(&is_reference)) && is_reference) {
            new_pointer_type_str = "&";
        }
        else if (BOOL is_rvalue_reference;
            SUCCEEDED(type->get_RValueReference(&is_rvalue_reference)) &&
            is_rvalue_reference) {
            new_pointer_type_str = "&&";
        }
        else if (BOOL is_ptr_to_member_function;
            SUCCEEDED(type->get_isPointerToMemberFunction(&is_ptr_to_member_function)) &&
            is_ptr_to_member_function) {
            CComPtr<IDiaSymbol> class_parent = nullptr;
            if (FAILED(type->get_classParent(&class_parent))) {
                return ErrorMessage(orbit_base::GetLastErrorAsString());
            }
            if (class_parent == nullptr) {
                return ErrorMessage("Unable to retrieve class parent symbol.");
            }
            OUTCOME_TRY(auto&& class_parent_str, orbit_object_utils::PdbDiaTypeAsString(*&class_parent));
            new_pointer_type_str = class_parent_str + "::*";
        }
        else if (BOOL is_ptr_to_member_function;
            SUCCEEDED(type->get_isPointerToDataMember(&is_ptr_to_member_function)) &&
            is_ptr_to_member_function) {
            CComPtr<IDiaSymbol> class_parent = nullptr;
            if (FAILED(type->get_classParent(&class_parent))) {
                return ErrorMessage(orbit_base::GetLastErrorAsString());
            }
            if (class_parent == nullptr) {
                return ErrorMessage("Unable to retrieve class parent symbol.");
            }
            OUTCOME_TRY(auto&& class_parent_str, orbit_object_utils::PdbDiaTypeAsString(*&class_parent));
            new_pointer_type_str = class_parent_str + "::*";
        }
        else {
            new_pointer_type_str = "*";
        }

        std::string type_modifiers = GetTypeModifiersAsString(type);
        if (!type_modifiers.empty()) {
            new_pointer_type_str += " ";
            new_pointer_type_str += type_modifiers;
        }

        new_pointer_type_str += parent_pointer_type_str;
        return orbit_object_utils::PdbDiaTypeAsString(*&base_type, new_pointer_type_str);
    }


} //  namespace

namespace orbit_object_utils {

ErrorMessageOr<std::string> PdbDiaParameterListAsString(IDiaSymbol* function_or_function_type) {

  DWORD tag;
  if (FAILED(function_or_function_type->get_symTag(&tag))) {
      return ErrorMessage(orbit_base::GetLastErrorAsString());
  }
  if (tag == SymTagFunction) {
    CComPtr<IDiaSymbol> function_type = nullptr;
    if (FAILED(function_or_function_type->get_type(&function_type))) {
        return ErrorMessage(orbit_base::GetLastErrorAsString());
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
      return ErrorMessage(orbit_base::GetLastErrorAsString());
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
      result_string += ", ";
    }

    CComPtr<IDiaSymbol> parameter_type = nullptr;
    if (FAILED(parameter->get_type(&parameter_type))) {
        return ErrorMessage(orbit_base::GetLastErrorAsString());
    }
    if (parameter_type == nullptr) {
        return ErrorMessage("Unable to retrieve type symbol.");
    }

    OUTCOME_TRY(auto && parameter_type_str, orbit_object_utils::PdbDiaTypeAsString(parameter_type));
    result_string += parameter_type_str;

    is_first_parameter = false;
  }

  result_string += ")";

  return result_string;
}

ErrorMessageOr<std::string> PdbDiaTypeAsString(IDiaSymbol* type, const std::string& parent_pointer_type_str) {
  HRESULT result;
  DWORD tag;
  if (FAILED(type->get_symTag(&tag))) {
    return ErrorMessage("Found Dia symbol without a tag.");
  }

  std::string result_string;
  if (tag != SymTagPointerType) {
      std::string type_modifiers = GetTypeModifiersAsString(type);
      if (!type_modifiers.empty()) {
          result_string += type_modifiers;
          result_string += " ";
      }
  }

  BSTR type_name_bstr = {};
  std::string type_name;
  if (SUCCEEDED(type->get_name(&type_name_bstr)) && type_name_bstr != NULL) {
    std::wstring type_name_wstring{type_name_bstr};
    type_name = std::string(type_name_wstring.begin(), type_name_wstring.end());
    SysFreeString(type_name_bstr);
  }

  if (!type_name.empty()) {
    result_string += type_name;
    result_string += parent_pointer_type_str;
    return result_string;
  }

  switch (tag) {
    case SymTagArrayType: {
      // For now we only print "[]" for arrays. However there is room for improvement.
      // We could e.g. also print the size of the array if known.
      CComPtr<IDiaSymbol> base_type = nullptr;
      if (FAILED(type->get_type(&base_type))) {
         return ErrorMessage(orbit_base::GetLastErrorAsString());
      }
      if (base_type == nullptr) {
          return ErrorMessage("Unable to retrieve type symbol.");
      }
      std::string new_pointer_type_str = "[]" + parent_pointer_type_str;
      OUTCOME_TRY(auto&& type_str, PdbDiaTypeAsString(*&base_type, new_pointer_type_str));
      result_string += type_str;

      return result_string;
    }
    case SymTagBaseType: {
        OUTCOME_TRY(auto && type_str, GetBaseTypeAsString(type, parent_pointer_type_str));
        result_string += type_str;
        return result_string;
    }
    case SymTagPointerType: {
        OUTCOME_TRY(auto && type_str, GetPointerTypeAsString(type, parent_pointer_type_str));
      result_string += type_str;
      return result_string;
    }
    case SymTagFunctionType: {
      CComPtr<IDiaSymbol> return_type = nullptr;
      if (FAILED(type->get_type(&return_type))) {
          return ErrorMessage(orbit_base::GetLastErrorAsString());
      }
      if (return_type == nullptr) {
          return ErrorMessage("Unable to retrieve type symbol.");
      }
      OUTCOME_TRY(auto && return_type_str, PdbDiaTypeAsString(*&return_type));
      result_string += return_type_str;
      result_string += " (";
      result_string += parent_pointer_type_str;
      result_string += ")";
      OUTCOME_TRY(auto && parameter_list, PdbDiaParameterListAsString(*&type));
      result_string += parameter_list;
      return result_string;
    }
    default:
      return ErrorMessage(absl::StrFormat("Unexpected tag \"%d\".", tag));
  }
}

}  // namespace orbit_object_utils