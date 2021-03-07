// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TEST_LIB_H_
#define USER_SPACE_INSTRUMENTATION_TEST_LIB_H_

#include <string_view>

// Library functions for some trivial logging. This library is merely used in tests: The test
// injects a binary produced by this code into its child.

// Call first to initialize the library.
extern "C" void InitTestLib();

// Log a string into a temporary file.
extern "C" void UseTestLib(std::string_view s);

// Call to end using the library. Prints the entire log to standard output and removes the
// temporary log file.
extern "C" void CloseTestLib();

// Returns 42.
extern "C" int TrivialFunction();

#endif  // USER_SPACE_INSTRUMENTATION_TEST_LIB_H_