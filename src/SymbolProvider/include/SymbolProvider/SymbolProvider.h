// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_
#define SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_symbol_provider {

// "SymbolProvider" supports retrieving symbols for the provided module from its symbol source, and
// provides the local file path where symbols are cached if retrieval succeeded.
// It also allows to query whether module symbols can be found in its symbol source.
class SymbolProvider {
 public:
  virtual ~SymbolProvider() = default;

  // Search for symbols for the provided module in the SymbolProvider's symbol source. Return an
  // ErrorMessage if error occurs or symbols are not found, void otherwise.
  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<void>> FindSymbols(
      const ModuleIdentifier& module_id) = 0;

  // Retrieve symbols for the provided module from the SymbolProvider's symbol source. Return:
  // - A local file path if successfully retrieve symbols;
  // - A Canceled if the retrieve operation is canceled via the stop_token;
  // - An ErrorMassage if symbols are not found or error occurs while retrieving symbols.
  [[nodiscard]] virtual orbit_base::Future<
      ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>>
  RetrieveSymbols(const ModuleIdentifier& module_id, orbit_base::StopToken stop_token) = 0;
};

}  // namespace orbit_symbol_provider

#endif  // SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_
