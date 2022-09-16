// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_FUNCTION_SYMBOLS_H_
#define MIZAR_BASE_FUNCTION_SYMBOLS_H_

#include <string>

#include "MizarBase/BaselineOrComparison.h"

namespace orbit_mizar_base {

struct FunctionSymbol {
  std::string function_name;
  std::string module_file_name;
};

struct BaselineAndComparisonFunctionSymbols {
  Baseline<FunctionSymbol> baseline_function_symbol;
  Comparison<FunctionSymbol> comparison_function_symbol;
};

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_FUNCTION_SYMBOLS_H_