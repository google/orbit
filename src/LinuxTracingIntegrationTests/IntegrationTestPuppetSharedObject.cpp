// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include <cmath>

// extern "C" to have unmangled name in the symbol table.
extern "C" double function_that_works_for_a_considerable_amount_of_time() {
  double result = 2;

  for (size_t i = 0; i < 10'000'000; ++i) {
    result = std::sqrt(result + 1.0);
  }

  return result;
}
