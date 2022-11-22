// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolProvider/StructuredDebugDirectorySymbolProvider.h"

#include <absl/strings/str_format.h>

#include <string>
#include <string_view>
#include <variant>

#include "OrbitBase/File.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"

namespace orbit_symbol_provider {

orbit_base::Future<SymbolLoadingOutcome> StructuredDebugDirectorySymbolProvider::RetrieveSymbols(
    const ModuleIdentifier& module_id, orbit_base::StopToken /*stop_token*/) const {
  return {FindSymbolFile(module_id.build_id)};
}

[[nodiscard]] SymbolLoadingOutcome StructuredDebugDirectorySymbolProvider::FindSymbolFile(
    std::string_view build_id) const {
  // Since the first two digits form the name of a sub-directory, we will need at least 3 digits to
  // generate a proper filename: build_id[0:2]/build_id[2:].debug
  if (build_id.size() < 3) {
    return ErrorMessage{absl::StrFormat("The build-id \"%s\" is malformed.", build_id)};
  }

  auto full_file_path = directory_ / ".build-id" / build_id.substr(0, 2) /
                        absl::StrFormat("%s.debug", build_id.substr(2));

  OUTCOME_TRY(const bool file_exists, orbit_base::FileOrDirectoryExists(full_file_path));

  if (file_exists) {
    return SymbolLoadingSuccessResult(
        std::move(full_file_path), symbol_source_,
        SymbolLoadingSuccessResult::SymbolFileSeparation::kDifferentFile);
  }

  return orbit_base::NotFound{
      absl::StrFormat("File does not exist: \"%s\"", full_file_path.string())};
}

}  // namespace orbit_symbol_provider