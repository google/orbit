// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_FUNCTION_SYMBOL_TO_KEY_EXACT_NAME_MATCH_H_
#define MIZAR_DATA_FUNCTION_SYMBOL_TO_KEY_EXACT_NAME_MATCH_H_

#include "MizarBase/FunctionSymbols.h"

namespace orbit_mizar_data {

class FunctionSymbolToKeyExactNameMatch {
 public:
  [[nodiscard]] std::string GetKey(const orbit_mizar_base::FunctionSymbol& symbol) const {
    return symbol.function_name;
  }
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_FUNCTION_SYMBOL_TO_KEY_EXACT_NAME_MATCH_H_