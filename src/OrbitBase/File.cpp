// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/File.h"

#include <errno.h>
#include <fcntl.h>

#include <chrono>
#include <ctime>
#include <string>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif

#if defined(_WIN32)

// Windows never returns EINTR - so there is no need for TEMP_FAILURE_RETRY implementation
#define TEMP_FAILURE_RETRY(expression) (expression)

namespace {

int64_t lseek64(int fd, int64_t offset, int origin) { return _lseeki64(fd, offset, origin); }

}  // namespace

#endif

namespace {

template <typename TimePoint>
std::time_t to_time_t(TimePoint time_point) {
  // TODO(vickyliu): Switch this to `file_clock::to_sys()` once we migregate to C++20.
  auto sc_time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      time_point - TimePoint::clock::now() + std::chrono::system_clock::now());
  return std::chrono::system_clock::to_time_t(sc_time_point);
}

}  // namespace

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

ErrorMessageOr<unique_fd> OpenNewFileForWriting(const std::filesystem::path& path) {
#if defined(__linux)
  constexpr int kOpenFlags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC;
  constexpr int kOpenMode = 0600;
#elif defined(_WIN32)
  constexpr int kOpenFlags = O_WRONLY | O_CREAT | O_EXCL | O_BINARY;
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

ErrorMessageOr<unique_fd> OpenExistingFileForReadWrite(const std::filesystem::path& path) {
#if defined(__linux)
  constexpr int kOpenFlags = O_RDWR | O_CLOEXEC;
#elif defined(_WIN32)
  constexpr int kOpenFlags = O_RDWR | O_BINARY;
#endif  // defined(__linux)
  return OpenFile(path, kOpenFlags, 0);
}

ErrorMessageOr<void> WriteFully(const unique_fd& fd, const void* data, size_t size) {
  int64_t bytes_left = size;
  const char* current_position = static_cast<const char*>(data);
  while (bytes_left > 0) {
    int64_t bytes_written = TEMP_FAILURE_RETRY(write(fd.get(), current_position, bytes_left));
    if (bytes_written == -1) {
      return ErrorMessage{SafeStrerror(errno)};
    }
    current_position += bytes_written;
    bytes_left -= bytes_written;
  }

  ORBIT_CHECK(bytes_left == 0);
  return outcome::success();
}

ErrorMessageOr<void> WriteFully(const unique_fd& fd, std::string_view content) {
  return WriteFully(fd, content.data(), content.size());
}

ErrorMessageOr<void> WriteFullyAtOffset(const unique_fd& fd, const void* buffer, size_t size,
                                        int64_t offset) {
  int64_t seek_result = lseek64(fd.get(), offset, SEEK_SET);

  if (seek_result == -1) {
    return ErrorMessage{SafeStrerror(errno)};
  }

  return WriteFully(fd, buffer, size);
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

ErrorMessageOr<size_t> ReadFullyAtOffset(const unique_fd& fd, void* buffer, size_t size,
                                         int64_t offset) {
  int64_t seek_result = lseek64(fd.get(), offset, SEEK_SET);

  if (seek_result == -1) {
    return ErrorMessage{SafeStrerror(errno)};
  }

  return ReadFully(fd, buffer, size);
}

ErrorMessageOr<bool> FileOrDirectoryExists(const std::filesystem::path& path) {
  std::error_code error;
  bool result = std::filesystem::exists(path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return result;
}

ErrorMessageOr<void> MoveOrRenameFile(const std::filesystem::path& from,
                                      const std::filesystem::path& to) {
  std::error_code error;
  std::filesystem::rename(from, to, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return outcome::success();
}

ErrorMessageOr<bool> RemoveFile(const std::filesystem::path& file_path) {
  std::error_code error;
  bool removed = std::filesystem::remove(file_path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return removed;
}

ErrorMessageOr<bool> CreateDirectories(const std::filesystem::path& file_path) {
  std::error_code error;
  bool created = std::filesystem::create_directories(file_path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return created;
}

ErrorMessageOr<uint64_t> FileSize(const std::filesystem::path& file_path) {
  std::error_code error;
  uint64_t file_size = std::filesystem::file_size(file_path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return file_size;
}

ErrorMessageOr<void> ResizeFile(const std::filesystem::path& file_path, uint64_t new_size) {
  std::error_code error;

  std::filesystem::resize_file(file_path, new_size, error);

  if (error) {
    return ErrorMessage{
        absl::StrFormat("Unable to resize file \"%s\": %s", file_path.string(), error.message())};
  }

  return outcome::success();
}

ErrorMessageOr<std::vector<std::filesystem::path>> ListFilesInDirectory(
    const std::filesystem::path& directory) {
  std::vector<std::filesystem::path> files;
  std::error_code error;

  auto directory_iterator = std::filesystem::directory_iterator(directory, error);
  if (error) {
    return ErrorMessage{absl::StrFormat("Unable to list files in directory \"%s\": %s",
                                        directory.string(), error.message())};
  }

  for (auto it = std::filesystem::begin(directory_iterator),
            end = std::filesystem::end(directory_iterator);
       it != end; it.increment(error)) {
    if (error) {
      return ErrorMessage{
          absl::StrFormat("Iterating directory \"%s\": %s (increment failed, stopping)",
                          directory.string(), error.message())};
    }

    const auto& path = it->path();
    files.push_back(path);
  }

  return files;
}

ErrorMessageOr<absl::Time> GetFileDateModified(const std::filesystem::path& path) {
  std::error_code error;
  auto ftime = std::filesystem::last_write_time(path, error);
  if (error) {
    return ErrorMessage{absl::StrFormat("Fail to get the last write time of file %s: %s",
                                        path.string(), error.message())};
  }

  return absl::FromTimeT(to_time_t(ftime));
}

ErrorMessageOr<bool> IsDirectory(const std::filesystem::path& path) {
  std::error_code error;
  bool result = std::filesystem::is_directory(path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return result;
}

}  // namespace orbit_base