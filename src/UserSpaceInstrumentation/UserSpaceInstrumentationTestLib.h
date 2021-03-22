// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TEST_LIB_H_
#define USER_SPACE_INSTRUMENTATION_TEST_LIB_H_

// This library is merely used in tests: The test  injects a binary produced by this code into its
// child.

// Returns 42.
extern "C" int TrivialFunction();

#endif  // USER_SPACE_INSTRUMENTATION_TEST_LIB_H_