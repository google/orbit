// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOL_PROVIDER_MOCK_SYMBOL_PROVIDER_H_
#define SYMBOL_PROVIDER_MOCK_SYMBOL_PROVIDER_H_

#include <gmock/gmock.h>

#include "OrbitBase/Future.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "SymbolProvider/SymbolProvider.h"

namespace orbit_symbol_provider {

class MockSymbolProvider : public SymbolProvider {
 public:
  MOCK_METHOD(orbit_base::Future<SymbolLoadingOutcome>, RetrieveSymbols,
              (const ModuleIdentifier&, orbit_base::StopToken), (const, override));
};

}  // namespace orbit_symbol_provider

#endif