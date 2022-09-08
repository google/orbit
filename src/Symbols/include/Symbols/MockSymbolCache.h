// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_MOCK_SYMBOL_CACHE_H_
#define SYMBOLS_MOCK_SYMBOL_CACHE_H_

#include <gmock/gmock.h>

#include "Symbols/SymbolCacheInterface.h"

namespace orbit_symbols {
class MockSymbolCache : public SymbolCacheInterface {
 public:
  MOCK_METHOD(std::filesystem::path, GenerateCachedFilePath, (const std::filesystem::path&),
              (const, override));
};
}  // namespace orbit_symbols

#endif  // SYMBOLS_MOCK_SYMBOL_CACHE_H_