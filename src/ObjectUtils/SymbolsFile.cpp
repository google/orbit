// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/SymbolsFile.h"

#include <absl/strings/str_format.h>

#include <filesystem>
#include <memory>
#include <string>

#include "ObjectUtils/ObjectFile.h"
#include "ObjectUtils/PdbFile.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateSymbolsFile(
    const std::filesystem::path& file_path) {
  std::string error_message{
      absl::StrFormat("Unable to create symbols file from \"%s\".", file_path)};

  std::error_code error;
  bool exists = std::filesystem::exists(file_path, error);
  if (error) {
    error_message.append("\n* Unable to stat file.");
    return ErrorMessage{error_message};
  }
  if (!exists) {
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

  ErrorMessageOr<std::unique_ptr<PdbFile>> pdb_file_or_error = CreatePdbFile(file_path);

  if (pdb_file_or_error.has_value()) return std::move(pdb_file_or_error.value());

  error_message.append(absl::StrFormat("\n* File cannot be read as an pdb file, error: %s",
                                       pdb_file_or_error.error().message()));

  return ErrorMessage{error_message};
}

}  // namespace orbit_object_utils