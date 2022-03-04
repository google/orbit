// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/SymbolsFile.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <llvm/Demangle/Demangle.h>

#include <filesystem>
#include <memory>
#include <string>

#include "Introspection/Introspection.h"
#include "ObjectUtils/ObjectFile.h"
#include "ObjectUtils/PdbFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateSymbolsFile(
    const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info) {
  std::string error_message{
      absl::StrFormat("Unable to create symbols file from \"%s\".", file_path.string())};

  OUTCOME_TRY(auto file_exists, orbit_base::FileExists(file_path));

  if (!file_exists) {
    error_message.append("\n* File does not exist.");
    return ErrorMessage{error_message};
  }

  ErrorMessageOr<std::unique_ptr<ObjectFile>> object_file_or_error = CreateObjectFile(file_path);
  if (object_file_or_error.has_value()) {
    if (object_file_or_error.value()->HasDebugSymbols()) {
      return std::move(object_file_or_error.value());
    }
    error_message.append("\n* File does not contain symbols.");
    return ErrorMessage{error_message};
  }

  error_message.append(absl::StrFormat("\n* File cannot be read as an object file, error: %s",
                                       object_file_or_error.error().message()));

  ErrorMessageOr<std::unique_ptr<PdbFile>> pdb_file_or_error =
      CreatePdbFile(file_path, object_file_info);

  if (pdb_file_or_error.has_value()) return std::move(pdb_file_or_error.value());

  error_message.append(absl::StrFormat("\n* File cannot be read as a pdb file, error: %s",
                                       pdb_file_or_error.error().message()));

  return ErrorMessage{error_message};
}

// Demangling is quite expensive. We demangle in bulk here to easily measure the run time. It's also
// a first step towards parallelization which could now be easily implemented.
void DemangleSymbols(std::vector<FunctionSymbol>& function_symbols) {
  ORBIT_SCOPE(absl::StrFormat("DemangleSymbols (%u)", function_symbols.size()).c_str());
  for (FunctionSymbol& function_symbol : function_symbols) {
    if (function_symbol.demangled_name.empty()) {
      function_symbol.demangled_name =
          absl::StrCat(llvm::demangle(function_symbol.name), function_symbol.argument_list);
    }
  }
}

// We centralize the creation of the ModuleSymbols proto, which is an expensive operation, so that
// further optimizations can be done at this single point rather than having to modify all
// SymbolsFile implementations.
ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> SymbolsFile::LoadDebugSymbols() {
  ORBIT_SCOPE(absl::StrFormat("LoadDebugSymbols (%s)", GetFilePath().string()).c_str());
  OUTCOME_TRY(DebugSymbols debug_symbols, LoadRawDebugSymbols());

  DemangleSymbols(debug_symbols.function_symbols);

  ORBIT_SCOPE("ModuleSymbols protobuf creation");
  orbit_grpc_protos::ModuleSymbols module_symbols;
  module_symbols.set_load_bias(debug_symbols.load_bias);
  module_symbols.set_symbols_file_path(debug_symbols.symbols_file_path);

  std::vector<FunctionSymbol>& function_symbols = debug_symbols.function_symbols;
  auto* symbol_infos = module_symbols.mutable_symbol_infos();
  symbol_infos->Reserve(function_symbols.size());

  for (FunctionSymbol& function_symbol : function_symbols) {
    orbit_grpc_protos::SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
    symbol_info->set_name(std::move(function_symbol.name));
    symbol_info->set_demangled_name(std::move(function_symbol.demangled_name));
    symbol_info->set_address(function_symbol.address);
    symbol_info->set_size(function_symbol.size);
  }

  return module_symbols;
}

}  // namespace orbit_object_utils
