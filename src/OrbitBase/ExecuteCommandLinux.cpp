// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "OrbitBase/Logging.h"

namespace orbit_base {

std::optional<std::string> ExecuteCommand(std::string_view cmd) {
  std::unique_ptr<FILE, int (*)(FILE*)> pipe{popen(std::string{cmd}.c_str(), "r"), pclose};
  if (!pipe) {
    ORBIT_ERROR("Could not open pipe for \"%s\"", cmd);
    return std::optional<std::string>{};
  }

  std::array<char, 128> buffer{};
  std::string result;
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

}  // namespace orbit_base
