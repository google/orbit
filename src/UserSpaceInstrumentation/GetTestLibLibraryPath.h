// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_GET_TEST_LIB_LIBRARY_PATH
#define USER_SPACE_INSTRUMENTATION_GET_TEST_LIB_LIBRARY_PATH

#include <filesystem>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

ErrorMessageOr<std::filesystem::path> GetTestLibLibraryPath();

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_GET_TEST_LIB_LIBRARY_PATH