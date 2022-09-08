// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYMBOLS_SYMBOL_CACHE_INTERFACE_H_
#define SYMBOLS_SYMBOL_CACHE_INTERFACE_H_

#include <filesystem>

namespace orbit_symbols {
class SymbolCacheInterface {
 public:
  virtual ~SymbolCacheInterface() = default;

  [[nodiscard]] virtual std::filesystem::path GenerateCachedFilePath(
      const std::filesystem::path& file_path) const = 0;
};
}  // namespace orbit_symbols

#endif  // SYMBOLS_SYMBOL_CACHE_INTERFACE_H_
