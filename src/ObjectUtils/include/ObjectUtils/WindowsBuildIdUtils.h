// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OBJECT_UTILS_WINDOWS_BUILD_ID_UTILS_H_
#define OBJECT_UTILS_WINDOWS_BUILD_ID_UTILS_H_

#include <array>
#include <cstdint>
#include <string>

namespace orbit_object_utils {

[[nodiscard]] std::string ComputeWindowsBuildId(std::array<uint8_t, 16> guid, uint32_t age);

}  // namespace orbit_object_utils

#endif  // OBJECT_UTILS_WINDOWS_BUILD_ID_UTILS_H_
