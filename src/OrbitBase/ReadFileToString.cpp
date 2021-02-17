// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
// Windows does not have TEMP_FAILURE_RETRY - define a shortcut
#define TEMP_FAILURE_RETRY(expression) (expression)
#endif

#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_base {

ErrorMessageOr<std::string> ReadFileToString(const std::filesystem::path& file_name) noexcept {
#if defined(__linux)
  constexpr int open_flags = O_RDONLY | O_CLOEXEC;
#elif defined(_WIN32)
  constexpr int open_flags = O_RDONLY | O_BINARY;
#endif  // defined(__linux)
  auto fd =
      unique_resource(TEMP_FAILURE_RETRY(open(file_name.string().c_str(), open_flags)), [](int fd) {
        if (fd != -1) close(fd);
      });
  if (fd == -1) {
    return ErrorMessage(
        absl::StrFormat("Unable to read file \"%s\": %s", file_name.string(), SafeStrerror(errno)));
  }

  std::string result;

  // We want to avoid memory reallocations in case the file is relatively large.
  struct stat st {};
  if (fstat(fd, &st) != -1 && st.st_size > 0) {
    result.reserve(st.st_size);
  }

  std::array<char, BUFSIZ> buf{};
  int64_t number_of_bytes;
  while ((number_of_bytes = TEMP_FAILURE_RETRY(read(fd, buf.data(), buf.size()))) > 0) {
    result.append(buf.data(), number_of_bytes);
  }

  // It is 0 if we reached end of file, -1 means an error has occurred
  if (number_of_bytes == -1) {
    return ErrorMessage(
        absl::StrFormat("Unable to read file \"%s\": %s", file_name.string(), SafeStrerror(errno)));
  }

  return result;
}

}  // namespace orbit_base