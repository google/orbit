// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_PDB_DIA_UTILS_H_
#define OBJECT_UTILS_PDB_DIA_UTILS_H_

#include <Windows.h>
#include <atlbase.h>
#include <cguid.h>
#include <dia2.h>

#include <string>

#include "OrbitBase/Logging.h"

// Provides utility functions to retrieve a functions parameter types as string.
namespace orbit_object_utils {

// Retrieves a string representing the parameter list of the given function or
// function type. For functions with function type "<no type>" this returns an
// empty string.
std::string PdbDiaParameterListAsString(IDiaSymbol* function);

// Retrieves a string representation of the give type symbol. It will prepend the
// given `pointer_type_string` to the result string, or in case of a function pointer,
// add it inside the paranthesis (e.g. void(*)(int, int)).
// If the type can't be computed properly, "???" will be returned.
std::string PdbDiaTypeAsString(IDiaSymbol* type, std::string pointer_type_str = "");

std::string PdbDiaParameterListAsString(IDiaSymbol* function_or_function_type) {
  HRESULT result;

  DWORD tag;
  ORBIT_CHECK(SUCCEEDED(function_or_function_type->get_symTag(&tag)));
  if (tag == SymTagFunction) {
    IDiaSymbol* function_type = nullptr;
    result = function_or_function_type->get_type(&function_type);
    ORBIT_CHECK(result == S_OK);
    CComPtr<IDiaSymbol> function_type_com_ptr = function_type;
    return PdbDiaParameterListAsString(function_type);
  }
  // Some functions don't have a type (<no type>), which is a base type.
  // In this case, we don't show a parameter list (as this happens on C functions).
  if (tag == SymTagBaseType) {
    return "";
  }
  ORBIT_CHECK(tag == SymTagFunctionType);

  IDiaSymbol* function_type = function_or_function_type;

  IDiaEnumSymbols* parameter_enumeration = nullptr;
  result = function_type->findChildren(SymTagNull, nullptr, nsNone, &parameter_enumeration);
  ORBIT_CHECK(result == S_OK && parameter_enumeration != nullptr);
  CComPtr<IDiaEnumSymbols> parameter_enumeration_com_ptr = parameter_enumeration;

  std::string result_string = "(";

  IDiaSymbol* parameter = nullptr;
  ULONG celt = 0;
  BOOL is_first_parameter = true;
  while (SUCCEEDED(parameter_enumeration->Next(1, &parameter, &celt)) && (celt == 1) &&
         parameter != nullptr) {
    CComPtr<IDiaSymbol> parameter_com_ptr = parameter;
    if (!is_first_parameter) {
      result_string += ", ";
    }

    IDiaSymbol* parameter_type = nullptr;
    result = parameter->get_type(&parameter_type);
    ORBIT_CHECK(result == S_OK && parameter_type != nullptr);
    CComPtr<IDiaSymbol> paramater_type_com_ptr = parameter_type;

    result_string += PdbDiaTypeAsString(parameter_type);

    is_first_parameter = false;
  }

  result_string += ")";

  return result_string;
}

std::string PdbDiaTypeAsString(IDiaSymbol* type, std::string parent_pointer_type_str) {
  HRESULT result;
  DWORD tag;
  if (FAILED(type->get_symTag(&tag))) {
    ORBIT_ERROR("Found Dia symbol without a tag. Returning \"???\" as a type name.");
    return "???";
  }

  std::string result_string;
  if (tag != SymTagPointerType) {
    if (BOOL is_const; SUCCEEDED(type->get_constType(&is_const)) && is_const) {
      result_string += "const ";
    }
    if (BOOL is_volatile; SUCCEEDED(type->get_volatileType(&is_volatile)) && is_volatile) {
      result_string += "volatile ";
    }
    if (BOOL is_unaligned; SUCCEEDED(type->get_unalignedType(&is_unaligned)) && is_unaligned) {
      result_string += "__unaligned ";
    }
    if (BOOL is_restricted; SUCCEEDED(type->get_restrictedType(&is_restricted)) && is_restricted) {
      result_string += "restricted ";
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
      IDiaSymbol* base_type = nullptr;
      if (FAILED(type->get_type(&base_type)) || base_type == nullptr) {
        return "???";
      }
      CComPtr<IDiaSymbol> base_type_comp = base_type;
      std::string new_pointer_type_str = "[]" + parent_pointer_type_str;
      result_string += PdbDiaTypeAsString(base_type, new_pointer_type_str);

      return result_string;
    }
    case SymTagBaseType: {
      DWORD base_type;
      if (FAILED(type->get_baseType(&base_type))) {
        return "???";
      }
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
            return "???";
          }
          switch (length) {
            case 1:
              result_string += "char";
              break;
            case 2:
              result_string += "short";
              break;
            case 4:
              result_string += "int";
              break;
            case 8:
              result_string += "__int64";
              break;
            default:
              ORBIT_UNREACHABLE();
          }
          break;
        }
        case btUInt:  // 7
        {
          ULONGLONG length;
          if (FAILED(type->get_length(&length))) {
            return "???";
          }

          result_string += "unsigned ";

          switch (length) {
            case 1:
              result_string += "char";
              break;
            case 2:
              result_string += "short";
              break;
            case 4:
              result_string += "int";
              break;
            case 8:
              result_string += "__int64";
              break;
            default:
              ORBIT_UNREACHABLE();
          }
          break;
        }
        case btFloat:  // 8
        {
          ULONGLONG length;
          if (FAILED(type->get_length(&length))) {
            return "???";
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
          ORBIT_ERROR("Unexpected base type with id \"%d\". Returning \"???\" as a type name.",
                      base_type);
          return "???";
      }
      result_string += parent_pointer_type_str;
      return result_string;
    }
    case SymTagPointerType: {
      IDiaSymbol* base_type = nullptr;
      if (FAILED(type->get_type(&base_type)) || base_type == nullptr) {
        return "???";
      }
      CComPtr<IDiaSymbol> base_type_comp = base_type;

      std::string new_pointer_type_str;
      if (BOOL is_reference; SUCCEEDED(type->get_reference(&is_reference)) && is_reference) {
        new_pointer_type_str = "&";
      } else if (BOOL is_rvalue_reference;
                 SUCCEEDED(type->get_RValueReference(&is_rvalue_reference)) &&
                 is_rvalue_reference) {
        new_pointer_type_str = "&&";
      } else if (BOOL is_ptr_to_member_function;
                 SUCCEEDED(type->get_isPointerToMemberFunction(&is_ptr_to_member_function)) &&
                 is_ptr_to_member_function) {
        IDiaSymbol* class_parent = nullptr;
        if (FAILED(type->get_classParent(&class_parent)) || class_parent == nullptr) {
          return "???";
        }
        CComPtr<IDiaSymbol> class_parent_comp = class_parent;
        std::string class_parent_str = PdbDiaTypeAsString(class_parent);
        new_pointer_type_str = class_parent_str + "::*";
      } else if (BOOL is_ptr_to_member_function;
                 SUCCEEDED(type->get_isPointerToDataMember(&is_ptr_to_member_function)) &&
                 is_ptr_to_member_function) {
        IDiaSymbol* class_parent = nullptr;
        if (FAILED(type->get_classParent(&class_parent)) || class_parent == nullptr) {
          return "???";
        }
        CComPtr<IDiaSymbol> class_parent_comp = class_parent;
        std::string class_parent_str = PdbDiaTypeAsString(class_parent);
        new_pointer_type_str = class_parent_str + "::*";
      } else {
        new_pointer_type_str = "*";
      }

      if (BOOL is_const; SUCCEEDED(type->get_constType(&is_const)) && is_const) {
        new_pointer_type_str += " const";
      }
      if (BOOL is_volatile; SUCCEEDED(type->get_volatileType(&is_volatile)) && is_volatile) {
        new_pointer_type_str += " volatile";
      }
      if (BOOL is_unaligned; SUCCEEDED(type->get_unalignedType(&is_unaligned)) && is_unaligned) {
        new_pointer_type_str += " __unaligned";
      }
      if (BOOL is_restricted;
          SUCCEEDED(type->get_restrictedType(&is_restricted)) && is_restricted) {
        new_pointer_type_str += " restricted";
      }

      new_pointer_type_str += parent_pointer_type_str;
      result_string += PdbDiaTypeAsString(base_type, new_pointer_type_str);

      return result_string;
    }
    case SymTagFunctionType: {
      IDiaSymbol* return_type = nullptr;
      if (FAILED(type->get_type(&return_type)) || return_type == nullptr) {
        return "???";
      }
      CComPtr<IDiaSymbol> return_type_comp = return_type;
      result_string += PdbDiaTypeAsString(return_type);
      result_string += " (";
      result_string += parent_pointer_type_str;
      result_string += ")";
      result_string += PdbDiaParameterListAsString(type);
      return result_string;
    }
    default:
      ORBIT_ERROR("Unexpected tag \"%d\". Returning \"???\" as a type name.", tag);
      return "???";
  }
}

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_PDB_DIA_UTILS_H_
