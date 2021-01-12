// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/ExecutablePath.h"

#include <filesystem>

namespace orbit_base {

std::filesystem::path GetExecutableDir() { return GetExecutablePath().parent_path(); }

}  // namespace orbit_base
