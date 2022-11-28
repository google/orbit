// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>

#include <filesystem>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

std::filesystem::path GetExecutablePath() {
  char buffer[PATH_MAX];
  ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (length == -1) {
    ORBIT_FATAL("Unable to readlink /proc/self/exe: %s", SafeStrerror(errno));
  }

  return std::filesystem::path(std::string(buffer, length));
}

ErrorMessageOr<std::filesystem::path> GetExecutablePath(uint32_t process_id) {
  char buffer[PATH_MAX];
  pid_t pid = ToNativeProcessId(process_id);

  ssize_t length = readlink(absl::StrFormat("/proc/%i/exe", pid).c_str(), buffer, sizeof(buffer));
  if (length == -1) {
    return ErrorMessage(absl::StrFormat("Unable to get executable path of process with pid %d: %s",
                                        pid, SafeStrerror(errno)));
  }

  return std::filesystem::path{std::string(buffer, length)};
}

}  // namespace orbit_base
