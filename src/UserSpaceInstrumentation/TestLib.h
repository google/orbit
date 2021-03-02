// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TEST_LIB_H_
#define USER_SPACE_INSTRUMENTATION_TEST_LIB_H_

#include <string>

namespace orbit_user_space_instrumentation {

// Library functions for some trivial logging. This library is merely used in tests: A binary
// produced by this code is checked in into the testdata folder. The test injects this binary into
// its child.

// Call first to initialize the library.
void InitTestLib();

// Log a string into a temporary file.
void UseTestLib(const std::string& s);

// Call to end using the library. Prints the entire log to standard out and removes the temporary
// log file.
void CloseTestLib();

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_TEST_LIB_H_