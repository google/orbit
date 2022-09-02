// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_MOCK_SYMBOL_CACHE_INTERFACE_H_
#define SYMBOLS_MOCK_SYMBOL_CACHE_INTERFACE_H_

#include "Symbols/SymbolCacheInterface.h"

namespace orbit_symbols {
class MockSymbolCacheInterface : public SymbolCacheInterface {
 public:
  MOCK_METHOD(std::filesystem::path, GenerateCachedFileName, (const std::filesystem::path&),
              (const, override));
};
}  // namespace orbit_symbols

#endif  // SYMBOLS_MOCK_SYMBOL_CACHE_INTERFACE_H_