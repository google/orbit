// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/File.h"

namespace orbit_base {

ErrorMessageOr<std::filesystem::path> GetFilePathFromFd(const unique_fd& fd) {
  std::array<char, PATH_MAX> buffer{};
  std::string path_to_fd = absl::StrFormat("/proc/self/fd/%d", fd.get());
  ssize_t length = readlink(path_to_fd.c_str(), buffer.data(), buffer.size());
  if (length == -1) {
    return ErrorMessage{
        absl::StrFormat("Unable to readlink \"%s\": %s", path_to_fd, SafeStrerror(errno))};
  }

  CHECK(static_cast<size_t>(length) <= buffer.size());
  return std::filesystem::path(std::string(buffer.data(), length));
}

}  // namespace orbit_base