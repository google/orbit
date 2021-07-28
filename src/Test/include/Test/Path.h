// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TEST_PATH_H_
#define ORBIT_TEST_PATH_H_

#include <filesystem>

namespace orbit_test {

// Returns the absolute path to the current `testdata` subdirectory.
// This function is meant to be used in conjunction with the
// `register_test` CMake-function (see `:/cmake/tests.cmake` for details).
//
// Each file from the testdata subdirectory (`:/src/<module>/testdata/<file>`)
// can be accessed through `orbit_test::GetTestdataDir() / "<file>"` in all test
// targets registered in the `CMakeLists.txt` file from the same module directory
// (`:/src/<module>/CMakeLists.txt`).
//
// Note that testdata files from other modules are NOT accessible this way.
// If you need a testdata file from a different module, you will have to copy it
// into your local `testdata` subdirectory.
//
// The result of `GetTestdataDir` can be overwritten by setting the environment
// variable `ORBIT_OVERRIDE_TESTDATA_PATH`. This makes sense for non-CMake based
// builds.
[[nodiscard]] std::filesystem::path GetTestdataDir();

}  // namespace orbit_test

#endif  // ORBIT_TEST_PATH_H_
