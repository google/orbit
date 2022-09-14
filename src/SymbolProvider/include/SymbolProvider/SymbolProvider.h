// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_
#define SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_

#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"

namespace orbit_symbol_provider {

// "SymbolProvider" supports retrieving symbols for the provided module from its symbol source, and
// provides the local file path where symbols are cached if retrieval succeeded.
class SymbolProvider {
 public:
  virtual ~SymbolProvider() = default;

  [[nodiscard]] virtual orbit_base::Future<SymbolLoadingOutcome> RetrieveSymbols(
      const ModuleIdentifier& module_id, orbit_base::StopToken stop_token) const = 0;
};

}  // namespace orbit_symbol_provider

#endif  // SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_
