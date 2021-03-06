// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_FILE_H_
#define ORBIT_BASE_FILE_H_

#include <absl/strings/str_format.h>
#include <fcntl.h>

#include <filesystem>
#include <string_view>

#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif

namespace orbit_base {

constexpr int kInvalidFd = -1;

class unique_fd {
 public:
  constexpr unique_fd() noexcept = default;
  constexpr explicit unique_fd(int fd) noexcept : fd_{fd} {}
  ~unique_fd() noexcept { release(); }

  unique_fd(const unique_fd&) = delete;
  unique_fd& operator=(const unique_fd&) = delete;

  constexpr unique_fd(unique_fd&& other) noexcept : fd_{other.fd_} { other.fd_ = kInvalidFd; }

  unique_fd& operator=(unique_fd&& other) noexcept {
    if (&other == this) return *this;

    reset(other.fd_);
    other.fd_ = kInvalidFd;

    return *this;
  }

  void release() noexcept {
    if (fd_ != kInvalidFd) close(fd_);
    fd_ = kInvalidFd;
  }

  [[nodiscard]] constexpr bool valid() const noexcept { return fd_ != kInvalidFd; }

  [[nodiscard]] constexpr int get() const noexcept { return fd_; }

 private:
  void reset(int fd) noexcept {
    release();
    fd_ = fd;
  }

  int fd_{kInvalidFd};
};

// The following functions provide convenience wrappers for working with file
// descriptors without having having to do TEMP_FAILURE_RETRY and
// different subroutines for Windows/Linux. They also translate errors to
// ErrorMessageOr<T>.

ErrorMessageOr<unique_fd> OpenFileForReading(const std::filesystem::path& path);

ErrorMessageOr<unique_fd> OpenFileForWriting(const std::filesystem::path& path);

ErrorMessageOr<void> WriteFully(const unique_fd& fd, std::string_view content);

// Tries to read 'size' bytes from the file to the buffer, returns actual
// number of bytes read. Note that the return value is less then size in
// the case when end of file was encountered.
//
// Use this function only for reading from files. This function is not supposed to be
// used for non-blocking reads from sockets/pipes - it does not handle EAGAIN.
ErrorMessageOr<size_t> ReadFully(const unique_fd& fd, void* buffer, size_t size);

}  // namespace orbit_base

#endif  // ORBIT_BASE_FILE_H_
