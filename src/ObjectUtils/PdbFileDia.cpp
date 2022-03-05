// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PdbFileDia.h"

#include <absl/memory/memory.h>
#include <diacreate.h>
#include <llvm/Demangle/Demangle.h>
#include <winerror.h>

<<<<<<< HEAD
#include "ObjectUtils/PdbUtilsDia.h"
=======
#include "Introspection/Introspection.h"
>>>>>>> 5f7357592 (Load debug symbols as raw struct instead of proto)
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
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

ErrorMessageOr<DebugSymbols> PdbFileDia::LoadRawDebugSymbols() {
  ORBIT_SCOPE_FUNCTION;
  DebugSymbols debug_symbols;
  debug_symbols.load_bias = object_file_info_.load_bias;
  debug_symbols.symbols_file_path = file_path_.string();

  OUTCOME_TRY(LoadProcSymbols(SymTagFunction, debug_symbols));

  return debug_symbols;
}

ErrorMessageOr<void> PdbFileDia::LoadProcSymbols(const enum SymTagEnum symbol_tag,
                                                 DebugSymbols& debug_symbols) {
  HRESULT result = 0;
  CComPtr<IDiaEnumSymbols> dia_enum_symbols;
  result = dia_global_scope_symbol_->findChildren(symbol_tag, /*name=*/nullptr,
                                                  NameSearchOptions::nsNone, &dia_enum_symbols);
  if (result != S_OK) {
    return ErrorMessage{
        absl::StrFormat("findChildren failed for %s (%u)", file_path_.string(), result)};
  }

  IDiaSymbol* dia_symbol = nullptr;
  ULONG celt = 0;

  while (SUCCEEDED(dia_enum_symbols->Next(1, &dia_symbol, &celt)) && (celt == 1)) {
    CComPtr<IDiaSymbol> dia_symbol_com_ptr = dia_symbol;

    BSTR function_name = {};
    if (dia_symbol->get_name(&function_name) != S_OK) continue;
    symbol_info.set_demangled_name(orbit_base::ToStdString(function_name));
    ErrorMessageOr<std::string> parameter_list_or_error = PdbDiaParameterListAsString(dia_symbol);
    if (parameter_list_or_error.has_value()) {
      symbol_info.set_demangled_name(symbol_info.demangled_name() +
                                     parameter_list_or_error.value());
    } else {
      ORBIT_ERROR("Unable to retrieve parameter types of function %s. Error: %s",
                  symbol_info.demangled_name(), parameter_list_or_error.error().message());
    }
    SysFreeString(function_name);

    FunctionSymbol& function_symbol = debug_symbols.function_symbols.emplace_back();
    function_symbol.demangled_name = std::string(name.begin(), name.end());

    DWORD relative_virtual_address = 0;
    if (dia_symbol->get_relativeVirtualAddress(&relative_virtual_address) != S_OK) continue;
    function_symbol.relative_virtual_address =
        relative_virtual_address + object_file_info_.load_bias;

    ULONGLONG length = 0;
    if (dia_symbol->get_length(&length) != S_OK) continue;
    function_symbol.size = length;
  }

  return outcome::success();
}

ErrorMessageOr<std::unique_ptr<PdbFile>> PdbFileDia::CreatePdbFile(
    const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info) {
  auto pdb_file_dia = absl::WrapUnique<PdbFileDia>(new PdbFileDia(file_path, object_file_info));
  auto result = pdb_file_dia->LoadDataForPDB();
  if (result.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to load PDB file %s with error: %s",
                                        file_path.string(), result.error().message()));
  }
  return std::move(pdb_file_dia);
}

}  // namespace orbit_object_utils
