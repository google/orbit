// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_OS_VERSION_WINDOWS_H_
#define ORBIT_BASE_OS_VERSION_WINDOWS_H_

#include <stdint.h>

#include <string>

#include "OrbitBase/Result.h"

namespace orbit_base {

struct WindowsVersion {
  uint32_t major_version = 0;
  uint32_t minor_version = 0;
  uint32_t build_number = 0;
  uint32_t platform_id = 0;
  std::string service_pack_version;
};

[[nodiscard]] ErrorMessageOr<WindowsVersion> GetWindowsVersion();
[[nodiscard]] ErrorMessageOr<std::string> GetWindowsVersionAsString();

}  // namespace orbit_base

#endif  // ORBIT_BASE_OS_VERSION_WINDOWS_H_
