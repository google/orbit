// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_SYMBOL_LOADING_OUTCOME_H_
#define SYMBOL_PROVIDER_SYMBOL_LOADING_OUTCOME_H_

#include <filesystem>
#include <string>
#include <utility>
#include <variant>

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"

namespace orbit_symbol_provider {

struct SymbolLoadingSuccessResult {
  enum class SymbolSource {
    kUnknown,
    kOrbitCache,
    kLocalStadiaSdk,
    kStadiaInstance,
    kSymbolLocationsDialog,
    kAdditionalSymbolPathsFlag,
    kStadiaSymbolStore,
    kMicrosoftSymbolServer,
    kUserDefinedSymbolStore,
    kUsrLibDebugDirectory,
    kStadiaInstanceUsrLibDebug
  };
  enum class SymbolFileSeparation { kDifferentFile, kModuleFile };

  explicit SymbolLoadingSuccessResult(std::filesystem::path path, SymbolSource symbol_source,
                                      SymbolFileSeparation symbol_file_separation)
      : path(std::move(path)),
        symbol_source(symbol_source),
        symbol_file_separation(symbol_file_separation) {}
  std::filesystem::path path;
  SymbolSource symbol_source;
  SymbolFileSeparation symbol_file_separation;
};

using SymbolLoadingOutcome =
    ErrorMessageOr<orbit_base::CanceledOr<orbit_base::NotFoundOr<SymbolLoadingSuccessResult>>>;

[[nodiscard]] bool IsCanceled(const SymbolLoadingOutcome& outcome);
[[nodiscard]] bool IsNotFound(const SymbolLoadingOutcome& outcome);
[[nodiscard]] std::string GetNotFoundMessage(const SymbolLoadingOutcome& outcome);
[[nodiscard]] bool IsSuccessResult(const SymbolLoadingOutcome& outcome);
[[nodiscard]] SymbolLoadingSuccessResult GetSuccessResult(const SymbolLoadingOutcome& outcome);

}  // namespace orbit_symbol_provider

#endif  // SYMBOL_PROVIDER_SYMBOL_LOADING_OUTCOME_H_