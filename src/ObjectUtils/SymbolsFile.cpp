// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/SymbolsFile.h"

#include <absl/strings/str_format.h>
#include <stddef.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "Introspection/Introspection.h"
#include "ObjectUtils/ObjectFile.h"
#include "ObjectUtils/PdbFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateSymbolsFile(
    const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info) {
  ORBIT_SCOPE_FUNCTION;
  std::string error_message{
      absl::StrFormat("Unable to create symbols file from \"%s\": ", file_path.string())};

  OUTCOME_TRY(auto file_exists, orbit_base::FileOrDirectoryExists(file_path));

  if (!file_exists) {
    error_message.append("File does not exist.");
    return ErrorMessage{error_message};
  }

  ErrorMessageOr<std::unique_ptr<ObjectFile>> object_file_or_error = CreateObjectFile(file_path);
  if (object_file_or_error.has_value()) {
    if (object_file_or_error.value()->HasDebugSymbols()) {
      return std::move(object_file_or_error.value());
    }
    error_message.append("File does not contain symbols.");
    return ErrorMessage{error_message};
  }

  error_message.append(absl::StrFormat("File cannot be read as an object file: %s",
                                       object_file_or_error.error().message()));

  ErrorMessageOr<std::unique_ptr<PdbFile>> pdb_file_or_error =
      CreatePdbFile(file_path, object_file_info);

  if (pdb_file_or_error.has_value()) return std::move(pdb_file_or_error.value());

  error_message.append(absl::StrFormat(" File also cannot be read as a PDB file: %s",
                                       pdb_file_or_error.error().message()));

  return ErrorMessage{error_message};
}

// Comparator to sort SymbolInfos by address, and perform the corresponding binary searches.
bool SymbolsFile::SymbolInfoLessByAddress(const orbit_grpc_protos::SymbolInfo& lhs,
                                          const orbit_grpc_protos::SymbolInfo& rhs) {
  return lhs.address() < rhs.address();
}

void SymbolsFile::DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol(
    std::vector<orbit_grpc_protos::SymbolInfo>* symbol_infos) {
  // There might be functions for which we don't have sizes in the symbol information (like COFF
  // symbol table, or PDB public symbols). For these, compute the size as the distance from the
  // address of the next function.
  std::sort(symbol_infos->begin(), symbol_infos->end(), &SymbolInfoLessByAddress);

  for (size_t i = 0; i < symbol_infos->size(); ++i) {
    orbit_grpc_protos::SymbolInfo& symbol_info = symbol_infos->at(i);
    if (symbol_info.size() != kUnknownSymbolSize) {
      // This function symbol already has a size.
      continue;
    }

    if (i < symbol_infos->size() - 1) {
      // Deduce the size as the distance from the next function's address.
      symbol_info.set_size(symbol_infos->at(i + 1).address() - symbol_info.address());
    } else {
      // If the last symbol doesn't have a size, we can't deduce it, and we just set it to zero.
      symbol_info.set_size(0);
    }
  }
}

}  // namespace orbit_object_utils