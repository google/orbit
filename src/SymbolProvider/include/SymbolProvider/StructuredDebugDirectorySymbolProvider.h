// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_STRUCTURED_DEBUG_DIRECTORY_SYMBOL_PROVIDER_H_
#define SYMBOL_PROVIDER_STRUCTURED_DEBUG_DIRECTORY_SYMBOL_PROVIDER_H_

#include <filesystem>
#include <string_view>
#include <utility>

#include "OrbitBase/Future.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "SymbolProvider/SymbolProvider.h"

namespace orbit_symbol_provider {

// Check out GDB's documentation for how a debug directory is structured:
// https://sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html
class StructuredDebugDirectorySymbolProvider : public SymbolProvider {
 public:
  StructuredDebugDirectorySymbolProvider(std::filesystem::path directory,
                                         SymbolLoadingSuccessResult::SymbolSource symbol_source)
      : directory_(std::move(directory)), symbol_source_(symbol_source) {}

  [[nodiscard]] orbit_base::Future<SymbolLoadingOutcome> RetrieveSymbols(
      const ModuleIdentifier& module_id, orbit_base::StopToken stop_token) const override;

 private:
  [[nodiscard]] SymbolLoadingOutcome FindSymbolFile(std::string_view build_id) const;

  std::filesystem::path directory_;
  SymbolLoadingSuccessResult::SymbolSource symbol_source_;
};

}  // namespace orbit_symbol_provider

#endif