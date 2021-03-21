// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/File.h"

#include "OrbitBase/Logging.h"

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

static ErrorMessageOr<unique_fd> OpenFile(const std::filesystem::path& path, int flags, int mode) {
  int fd = TEMP_FAILURE_RETRY(open(path.string().c_str(), flags, mode));
  if (fd == kInvalidFd) {
    return ErrorMessage(
        absl::StrFormat("Unable to open file \"%s\": %s", path.string(), SafeStrerror(errno)));
  }

  return unique_fd{fd};
}

ErrorMessageOr<unique_fd> OpenFileForReading(const std::filesystem::path& path) {
#if defined(__linux)
  constexpr int kOpenFlags = O_RDONLY | O_CLOEXEC;
#elif defined(_WIN32)
  constexpr int kOpenFlags = O_RDONLY | O_BINARY;
#endif  // defined(__linux)
  return OpenFile(path, kOpenFlags, 0);
}

ErrorMessageOr<unique_fd> OpenFileForWriting(const std::filesystem::path& path) {
#if defined(__linux)
  constexpr int kOpenFlags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
  constexpr int kOpenMode = 0600;
#elif defined(_WIN32)
  constexpr int kOpenFlags = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
  constexpr int kOpenMode = _S_IREAD | _S_IWRITE;
#endif  // defined(__linux)
  return OpenFile(path, kOpenFlags, kOpenMode);
}

ErrorMessageOr<unique_fd> OpenNewFileForReadWrite(const std::filesystem::path& path) {
#if defined(__linux)
  constexpr int kOpenFlags = O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC;
  constexpr int kOpenMode = 0600;
#elif defined(_WIN32)
  constexpr int kOpenFlags = O_RDWR | O_CREAT | O_EXCL | O_BINARY;
  constexpr int kOpenMode = _S_IREAD | _S_IWRITE;
#endif  // defined(__linux)
  return OpenFile(path, kOpenFlags, kOpenMode);
}

ErrorMessageOr<void> WriteFully(const unique_fd& fd, std::string_view content) {
  int64_t bytes_left = content.size();
  const char* current_position = content.data();
  while (bytes_left > 0) {
    int64_t bytes_written = TEMP_FAILURE_RETRY(write(fd.get(), current_position, bytes_left));
    if (bytes_written == -1) {
      return ErrorMessage{SafeStrerror(errno)};
    }
    current_position += bytes_written;
    bytes_left -= bytes_written;
  }

  CHECK(bytes_left == 0);
  return outcome::success();
}

ErrorMessageOr<size_t> ReadFully(const unique_fd& fd, void* buffer, size_t size) {
  size_t bytes_left = size;
  auto current_position = static_cast<uint8_t*>(buffer);

  int64_t result = 0;
  while (bytes_left != 0 &&
         (result = TEMP_FAILURE_RETRY(read(fd.get(), current_position, bytes_left))) > 0) {
    bytes_left -= result;
    current_position += result;
  }

  if (result == -1) {
    return ErrorMessage{SafeStrerror(errno)};
  }

  return size - bytes_left;
}

}  // namespace orbit_base