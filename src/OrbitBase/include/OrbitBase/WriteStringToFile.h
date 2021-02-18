// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_WRITE_STRING_TO_FILE_H_
#define ORBIT_BASE_WRITE_STRING_TO_FILE_H_

#include <filesystem>
#include <string_view>

#include "OrbitBase/Result.h"

namespace orbit_base {
ErrorMessageOr<void> WriteStringToFile(const std::filesystem::path& file_name,
                                       std::string_view content);
}

#endif  // ORBIT_BASE_WRITE_STRING_TO_FILE_H_
