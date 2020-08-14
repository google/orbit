// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "SymbolHelper.h"

extern "C" int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) {
  const SymbolHelper symbol_helper;
  (void)symbol_helper.LoadSymbolsCollector(std::string{reinterpret_cast<const char*>(buf), len});
  return 0;
}