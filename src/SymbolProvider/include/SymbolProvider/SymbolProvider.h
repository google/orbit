// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_
#define SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_

#include "ClientData/ModulePathAndBuildId.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"

namespace orbit_symbol_provider {

// "SymbolProvider" supports retrieving symbols for the provided module from its symbol source, and
// provides the local file path where symbols are cached if retrieval succeeded.
class SymbolProvider {
 public:
  virtual ~SymbolProvider() = default;

  [[nodiscard]] virtual orbit_base::Future<SymbolLoadingOutcome> RetrieveSymbols(
      const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id,
      orbit_base::StopToken stop_token) = 0;
};

}  // namespace orbit_symbol_provider

#endif  // SYMBOL_PROVIDER_SYMBOL_PROVIDER_H_
