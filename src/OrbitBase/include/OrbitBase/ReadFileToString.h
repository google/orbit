// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_READ_FILE_TO_STRING_H_
#define ORBIT_BASE_READ_FILE_TO_STRING_H_

#include <filesystem>
#include <string>

#include "OrbitBase/Result.h"

namespace orbit_base {
ErrorMessageOr<std::string> ReadFileToString(const std::filesystem::path& file_name);
}

#endif  // ORBIT_BASE_READ_FILE_TO_STRING_H_
