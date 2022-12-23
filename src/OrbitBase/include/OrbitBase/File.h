// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_FILE_H_
#define ORBIT_BASE_FILE_H_

#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <fcntl.h>
#include <stdint.h>

#include <filesystem>
#include <string_view>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif

namespace orbit_base {

constexpr int kInvalidFd = -1;

class UniqueFd {
 public:
  constexpr UniqueFd() = default;
  constexpr explicit UniqueFd(int fd) : fd_{fd} {}
  ~UniqueFd() { release(); }

  UniqueFd(const UniqueFd&) = delete;
  UniqueFd& operator=(const UniqueFd&) = delete;

  constexpr UniqueFd(UniqueFd&& other) : fd_{other.fd_} { other.fd_ = kInvalidFd; }

  UniqueFd& operator=(UniqueFd&& other) {
    if (&other == this) return *this;

    reset(other.fd_);
    other.fd_ = kInvalidFd;

    return *this;
  }

  void release() {
    if (fd_ != kInvalidFd) close(fd_);
    fd_ = kInvalidFd;
  }

  [[nodiscard]] constexpr bool valid() const { return fd_ != kInvalidFd; }

  [[nodiscard]] int get() const {
    ORBIT_CHECK(valid());
    return fd_;
  }

 private:
  void reset(int fd) {
    release();
    fd_ = fd;
  }

  int fd_{kInvalidFd};
};

// The following functions provide convenience wrappers for working with file
// descriptors without having having to do TEMP_FAILURE_RETRY and
// different subroutines for Windows/Linux. They also translate errors to
// ErrorMessageOr<T>.

ErrorMessageOr<UniqueFd> OpenFileForReading(const std::filesystem::path& path);

ErrorMessageOr<UniqueFd> OpenFileForWriting(const std::filesystem::path& path);

ErrorMessageOr<UniqueFd> OpenNewFileForWriting(const std::filesystem::path& path);

ErrorMessageOr<UniqueFd> OpenNewFileForReadWrite(const std::filesystem::path& path);

ErrorMessageOr<UniqueFd> OpenExistingFileForReadWrite(const std::filesystem::path& path);

ErrorMessageOr<void> WriteFully(const UniqueFd& fd, const void* data, size_t size);

ErrorMessageOr<void> WriteFully(const UniqueFd& fd, std::string_view content);

ErrorMessageOr<void> WriteFullyAtOffset(const UniqueFd& fd, const void* buffer, size_t size,
                                        int64_t offset);

// Tries to read 'size' bytes from the file to the buffer, returns actual
// number of bytes read. Note that the return value is less then size in
// the case when end of file was encountered.
//
// Use this function only for reading from files. This function is not supposed to be
// used for non-blocking reads from sockets/pipes - it does not handle EAGAIN.
ErrorMessageOr<size_t> ReadFully(const UniqueFd& fd, void* buffer, size_t size);

// Same as above but tries to read from an offset. The file referenced by fd must be capable of
// seeking. The same limitation for non-blocking reads as above applies here.
ErrorMessageOr<size_t> ReadFullyAtOffset(const UniqueFd& fd, void* buffer, size_t size,
                                         int64_t offset);

template <typename T>
ErrorMessageOr<T> ReadFullyAtOffset(const UniqueFd& fd, int64_t offset) {
  T value;
  auto size_or_error = ReadFullyAtOffset(fd, &value, sizeof(value), offset);
  if (size_or_error.has_error()) {
    return size_or_error.error();
  }

  if (size_or_error.value() < sizeof(value)) {
    return ErrorMessage{absl::StrFormat("Not enough bytes left in the file: %d < %d",
                                        size_or_error.value(), sizeof(value))};
  }

  return value;
}

// Following functions make sure we call stl in exception-free manner
ErrorMessageOr<bool> FileOrDirectoryExists(const std::filesystem::path& path);
ErrorMessageOr<void> MoveOrRenameFile(const std::filesystem::path& from,
                                      const std::filesystem::path& to);
ErrorMessageOr<bool> RemoveFile(const std::filesystem::path& file_path);
ErrorMessageOr<bool> CreateDirectories(const std::filesystem::path& file_path);
ErrorMessageOr<void> ResizeFile(const std::filesystem::path& file_path, uint64_t new_size);
ErrorMessageOr<uint64_t> FileSize(const std::filesystem::path& file_path);
// Returns all files in directory; non recursively.
ErrorMessageOr<std::vector<std::filesystem::path>> ListFilesInDirectory(
    const std::filesystem::path& directory);
ErrorMessageOr<absl::Time> GetFileDateModified(const std::filesystem::path& path);
ErrorMessageOr<bool> IsDirectory(const std::filesystem::path& path);
ErrorMessageOr<bool> IsRegularFile(const std::filesystem::path& path);

}  // namespace orbit_base

#endif  // ORBIT_BASE_FILE_H_
