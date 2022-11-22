// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/ReadFileToString.h"

#include <absl/strings/str_format.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#include <array>

#include "OrbitBase/File.h"
#include "OrbitBase/SafeStrerror.h"

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif

#if defined(_WIN32)
// Windows never returns EINTR - so there is no need for TEMP_FAILURE_RETRY implementation
#define TEMP_FAILURE_RETRY(expression) (expression)
#endif

namespace orbit_base {

ErrorMessageOr<std::string> ReadFileToString(const std::filesystem::path& file_name) {
  ErrorMessageOr<unique_fd> fd_or_error = OpenFileForReading(file_name);
  if (fd_or_error.has_error()) {
    return fd_or_error.error();
  }

  const unique_fd& fd = fd_or_error.value();

  std::string result;

  // We want to avoid memory reallocations in case the file is relatively large.
  struct stat st {};
  if (fstat(fd.get(), &st) != -1 && st.st_size > 0) {
    result.reserve(st.st_size);
  }

  std::array<char, BUFSIZ> buf{};
  int64_t number_of_bytes;
  while ((number_of_bytes = TEMP_FAILURE_RETRY(read(fd.get(), buf.data(), buf.size()))) > 0) {
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