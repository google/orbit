// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PdbFileDia.h"

#include <absl/container/flat_hash_set.h>
#include <absl/memory/memory.h>
#include <cvconst.h>
#include <diacreate.h>
#include <llvm/Demangle/Demangle.h>
#include <winerror.h>

#include "Introspection/Introspection.h"
#include "ObjectUtils/PdbUtilsDia.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StringConversion.h"

using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_object_utils {

PdbFileDia::PdbFileDia(std::filesystem::path file_path, const ObjectFileInfo& object_file_info)
    : file_path_(std::move(file_path)), object_file_info_(object_file_info) {}

ErrorMessageOr<CComPtr<IDiaDataSource>> PdbFileDia::CreateDiaDataSource() {
  if (!SUCCEEDED(com_initializer_.result)) {
    return ErrorMessage{absl::StrFormat("CoInitialize failed (%u)", com_initializer_.result)};
  }
  // Create instance of dia data source.
  CComPtr<IDiaDataSource> dia_data_source = nullptr;
  constexpr const wchar_t* kDiaDllFileName = L"msdia140.dll";
  HRESULT cocreate_result = NoRegCoCreate(kDiaDllFileName, CLSID_DiaSource, IID_IDiaDataSource,
                                          reinterpret_cast<LPVOID*>(&dia_data_source));
  if (cocreate_result != S_OK) {
    return ErrorMessage{absl::StrFormat("NoRegCoCreate failed (%u)", cocreate_result)};
  }

  return dia_data_source;
}

ErrorMessageOr<void> PdbFileDia::LoadDataForPDB() {
  OUTCOME_TRY(dia_data_source_, CreateDiaDataSource());

  // Open and prepare a program database (.pdb) file as a debug data source.
  HRESULT result = dia_data_source_->loadDataFromPdb(file_path_.wstring().c_str());
  if (result != S_OK) {
    return ErrorMessage{
        absl::StrFormat("loadDataFromPdb failed for %s (%u)", file_path_.string(), result)};
  }

  // Open DIA session.
  result = dia_data_source_->openSession(&dia_session_);
  if (result != S_OK) {
    return ErrorMessage{
        absl::StrFormat("openSession failed for %s (%u)", file_path_.string(), result)};
  }

  // Retrieve a reference to the global scope.
  result = dia_session_->get_globalScope(&dia_global_scope_symbol_);
  if (result != S_OK) {
    return ErrorMessage{
        absl::StrFormat("get_globalScope failed for %s (%u)", file_path_.string(), result)};
  }

  // Get age.
  DWORD age = 0;
  result = dia_global_scope_symbol_->get_age(&age);
  if (result != S_OK) {
    return ErrorMessage{absl::StrFormat("get_age failed for %s (%u)", file_path_.string(), result)};
  }
  age_ = age;

  // Get guid.
  GUID guid = {};
  result = dia_global_scope_symbol_->get_guid(&guid);
  if (result != S_OK) {
    return ErrorMessage{
        absl::StrFormat("get_guid failed for %s (%u)", file_path_.string(), result)};
  }
  static_assert(sizeof(guid_) == sizeof(GUID));
  std::memcpy(guid_.data(), &guid, sizeof(GUID));

  return outcome::success();
}

namespace {
// This enum represents the possible flags to undecorate (demangle) a name of
// a public symbol. They are not defined in the DIA SDK headers. The values
// are defined here:
// https://docs.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/idiasymbol-get-undecoratednameex
enum UNDNAME_FLAGS : DWORD {
  UNDNAME_COMPLETE = 0x0000,  // Enables full undecoration.
  UNDNAME_NO_LEADING_UNDERSCORES =
      0x0001,                       // Removes leading underscores from Microsoft extended keywords.
  UNDNAME_NO_MS_KEYWORDS = 0x0002,  // Disables expansion of Microsoft extended keywords.
  UNDNAME_NO_FUNCTION_RETURNS =
      0x0004,  // Disables expansion of return type for primary declaration.
  UNDNAME_NO_ALLOCATION_MODEL = 0x0008,  // Disables expansion of the declaration model.
  UNDNAME_NO_ALLOCATION_LANGUAGE =
      0x0010,                    // Disables expansion of the declaration language specifier.
  UNDNAME_RESERVED1 = 0x0020,    // RESERVED.
  UNDNAME_RESERVED2 = 0x0040,    // RESERVED.
  UNDNAME_NO_THISTYPE = 0x0060,  // Disables all modifiers on the this type.
  UNDNAME_NO_ACCESS_SPECIFIERS = 0x0080,  // Disables expansion of access specifiers for members.
  UNDNAME_NO_THROW_SIGNATURES = 0x0100,   // Disables expansion of "throw-signatures" for
                                          // functions and pointers to functions.
  UNDNAME_NO_MEMBER_TYPE = 0x0200,        // Disables expansion of static or virtual members.
  UNDNAME_NO_RETURN_UDT_MODEL =
      0x0400,                      // Disables expansion of the Microsoft model for UDT returns.
  UNDNAME_32_BIT_DECODE = 0x0800,  // Undecorates 32-bit decorated names.
  UNDNAME_NAME_ONLY = 0x1000,      // Gets only the name for primary declaration, returns
                                   // just[scope::] name.Expands template params.
  UNDNAME_TYPE_ONLY = 0x2000,  // Input is just a type encoding, composes an abstract declarator.
  UNDNAME_HAVE_PARAMETERS = 0x4000,       // The real template parameters are available.
  UNDNAME_NO_ECSU = 0x8000,               // Suppresses enum/class/struct/union.
  UNDNAME_NO_IDENT_CHAR_CHECK = 0x10000,  // Suppresses check for valid identifier characters.
  UNDNAME_NO_PTR64 = 0x20000,             // Does not include ptr64 in output.
};
template <typename Consumer>
ErrorMessageOr<void> ForEachSymbolWithSymTag(const enum SymTagEnum& sym_tag,
                                             IDiaSymbol* dia_global_scope_symbol,
                                             std::string_view file_path, Consumer&& consumer) {
  CComPtr<IDiaEnumSymbols> dia_enum_symbols;
  HRESULT result = dia_global_scope_symbol->findChildren(sym_tag, NULL, nsNone, &dia_enum_symbols);
  if (result != S_OK) {
    return ErrorMessage{absl::StrFormat("findChildren failed for %s (%u)", file_path, result)};
  }

  IDiaSymbol* dia_symbol = nullptr;
  ULONG celt = 0;

  while (SUCCEEDED(dia_enum_symbols->Next(1, &dia_symbol, &celt)) && (celt == 1)) {
    CComPtr<IDiaSymbol> dia_symbol_com_ptr = dia_symbol;
    std::invoke(consumer, dia_symbol);
  }

  return outcome::success();
}
}  // namespace

ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> PdbFileDia::LoadDebugSymbols() {
  ModuleSymbols module_symbols;
  absl::flat_hash_set<uint64_t> addresses_from_module_info_stream;

  // Find the function symbols in the module info stream. For now, we ignore "blocks" and
  // "thunks". "Thunks" (which are 5 byte long jumps from incremental linking) don't even
  // have a name, and while "blocks" (nested scopes inside functions) may have names
  // according to the documentation, we have never observed that in real PDB files.
  OUTCOME_TRY(ForEachSymbolWithSymTag(
      SymTagFunction, dia_global_scope_symbol_.p, file_path_.string(),
      [&module_symbols, &addresses_from_module_info_stream, this](IDiaSymbol* dia_symbol) {
        SymbolInfo symbol_info;

        BSTR function_name = {};
        if (dia_symbol->get_name(&function_name) != S_OK) return;
        symbol_info.set_demangled_name(orbit_base::ToStdString(function_name));
        ErrorMessageOr<std::string> parameter_list_or_error =
            PdbDiaParameterListAsString(dia_symbol);
        if (parameter_list_or_error.has_value()) {
          symbol_info.set_demangled_name(symbol_info.demangled_name() +
                                         parameter_list_or_error.value());
        } else {
          ORBIT_ERROR("Unable to retrieve parameter types of function %s. Error: %s",
                      symbol_info.demangled_name(), parameter_list_or_error.error().message());
        }
        SysFreeString(function_name);

        DWORD relative_virtual_address = 0;
        if (dia_symbol->get_relativeVirtualAddress(&relative_virtual_address) != S_OK) return;
        symbol_info.set_address(relative_virtual_address + object_file_info_.load_bias);

        ULONGLONG length = 0;
        if (dia_symbol->get_length(&length) != S_OK) return;
        symbol_info.set_size(length);

        // We currently only support hotpatchable functions in elf files.
        symbol_info.set_is_hotpatchable(false);

        addresses_from_module_info_stream.insert(symbol_info.address());
        *module_symbols.add_symbol_infos() = std::move(symbol_info);
      }));

  // Check the public symbol stream for additional function symbols. Many public
  // symbols are already defined in the module info stream, so we will skip those
  // whose address we have already seen.
  OUTCOME_TRY(ForEachSymbolWithSymTag(
      SymTagPublicSymbol, dia_global_scope_symbol_.p, file_path_.string(),
      [&module_symbols, &addresses_from_module_info_stream, this](IDiaSymbol* dia_symbol) {
        // Is this public symbol actually a function?
        BOOL is_function = false;
        if (dia_symbol->get_function(&is_function) != S_OK || !is_function) return;
        SymbolInfo symbol_info;

        DWORD relative_virtual_address = 0;
        if (dia_symbol->get_relativeVirtualAddress(&relative_virtual_address) != S_OK) return;
        symbol_info.set_address(relative_virtual_address + object_file_info_.load_bias);

        if (addresses_from_module_info_stream.contains(symbol_info.address())) return;

        // Public symbols my have decorated (mangled names), where the decoration contains
        // much more information/noise than on elf files, such as "static", "virtual", return
        // types, or access modifiers like "public". We remove the unnecessary information
        // to reduce the noice and foster function matchin in Mizar.
        BSTR undecorated_function_name = {};
        DWORD undecorate_options = UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_FUNCTION_RETURNS |
                                   UNDNAME_NO_THISTYPE | UNDNAME_NO_ACCESS_SPECIFIERS |
                                   UNDNAME_NO_MEMBER_TYPE | UNDNAME_NO_THROW_SIGNATURES |
                                   UNDNAME_NO_ECSU | UNDNAME_NO_PTR64;
        if (dia_symbol->get_undecoratedNameEx(undecorate_options, &undecorated_function_name) ==
            S_OK) {
          symbol_info.set_demangled_name(orbit_base::ToStdString(undecorated_function_name));
          SysFreeString(undecorated_function_name);
        }

        // If there was no undecorated function name, we try the normal "name".
        if (symbol_info.demangled_name().empty()) {
          BSTR function_name = {};
          if (dia_symbol->get_name(&function_name) != S_OK) return;
          symbol_info.set_demangled_name(orbit_base::ToStdString(function_name));
          SysFreeString(function_name);
        }

        if (symbol_info.demangled_name().empty()) return;

        ULONGLONG length = 0;
        if (dia_symbol->get_length(&length) != S_OK) return;
        symbol_info.set_size(length);

        // We currently only support hotpatchable elf files.
        symbol_info.set_is_hotpatchable(false);

        *module_symbols.add_symbol_infos() = std::move(symbol_info);
      }));

  return module_symbols;
}

ErrorMessageOr<std::unique_ptr<PdbFile>> PdbFileDia::CreatePdbFile(
    const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info) {
  ORBIT_SCOPE_FUNCTION;
  auto pdb_file_dia = absl::WrapUnique<PdbFileDia>(new PdbFileDia(file_path, object_file_info));
  auto result = pdb_file_dia->LoadDataForPDB();
  if (result.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to load PDB file %s with error: %s",
                                        file_path.string(), result.error().message()));
  }
  return std::move(pdb_file_dia);
}

}  // namespace orbit_object_utils
