// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TEST_LIB_H_
#define USER_SPACE_INSTRUMENTATION_TEST_LIB_H_

#include <cstdint>

// This library is merely used in tests: The test  injects a binary produced by this code into its
// child.

// Returns 42.
extern "C" int TrivialFunction();

// Returns sum of the parameters.
extern "C" uint64_t TrivialSum(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4,
                               uint64_t p5);

#endif  // USER_SPACE_INSTRUMENTATION_TEST_LIB_H_