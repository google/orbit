// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_PATH_H_
#define ORBIT_BASE_PATH_H_

#include <filesystem>
#include <vector>

#include "OrbitBase/Result.h"

namespace orbit_base {

[[nodiscard]] std::filesystem::path GetExecutablePath();
[[nodiscard]] std::filesystem::path GetExecutableDir();

#if defined(__linux)
[[nodiscard]] ErrorMessageOr<std::filesystem::path> GetExecutablePath(int32_t pid);
#endif

}  // namespace orbit_base

#endif  // ORBIT_BASE_PATH_H_
