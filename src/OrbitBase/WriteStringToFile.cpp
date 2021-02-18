// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/WriteStringToFile.h"

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

#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_base {

ErrorMessageOr<void> WriteStringToFileImpl(int fd, std::string_view content) {
  int64_t bytes_left = content.size();
  const char* current_position = content.data();
  while (bytes_left > 0) {
    int64_t bytes_written = TEMP_FAILURE_RETRY(write(fd, current_position, bytes_left));
    if (bytes_written == -1) {
      return ErrorMessage{SafeStrerror(errno)};
    }
    current_position += bytes_written;
    bytes_left -= bytes_written;
  }

  CHECK(bytes_left == 0);
  return outcome::success();
}

ErrorMessageOr<void> WriteStringToFile(const std::filesystem::path& file_name,
                                       std::string_view content) {
#if defined(__linux)
  constexpr int open_flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
  constexpr int open_mode = 0600;
#elif defined(_WIN32)
  constexpr int open_flags = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
  constexpr int open_mode = _S_IREAD | _S_IWRITE;
#endif  // defined(__linux)
  auto fd = unique_resource(
      TEMP_FAILURE_RETRY(open(file_name.string().c_str(), open_flags, open_mode)), [](int fd) {
        if (fd != -1) close(fd);
      });
  if (fd == -1) {
    return ErrorMessage(
        absl::StrFormat("Unable to open file \"%s\": %s", file_name.string(), SafeStrerror(errno)));
  }

  ErrorMessageOr<void> result = WriteStringToFileImpl(fd, content);
  if (!result) {
    remove(file_name.string().c_str());
    return ErrorMessage{absl::StrFormat("Unable to write to \"%s\": %s", file_name.string(),
                                        result.error().message())};
  }

  return outcome::success();
}

}  // namespace orbit_base