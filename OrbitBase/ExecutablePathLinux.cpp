// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "absl/strings/str_format.h"

namespace orbit_base {

std::filesystem::path GetExecutablePath() {
  char buffer[PATH_MAX];
  ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (length == -1) {
    FATAL("Unable to readlink /proc/self/exe: %s", SafeStrerror(errno));
  }

  return std::filesystem::path(std::string(buffer, length));
}

}  // namespace orbit_base
