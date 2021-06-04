// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/File.h"

#include <cstdint>

#include "OrbitBase/Logging.h"

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif

#if defined(_WIN32)

// Windows never returns EINTR - so there is no need for TEMP_FAILURE_RETRY implementation
#define TEMP_FAILURE_RETRY(expression) (expression)

namespace {

using ssize_t = int64_t;

// There is no implementation for pread and pwrite in 'io.h' so we implement them on top of lseek,
// read and write here.
ssize_t pread(int fd, void* buffer, size_t size, off_t offset) {
  off_t old_position = lseek(fd, 0, SEEK_CUR);
  if (old_position == -1) {
    return -1;
  }
  if (lseek(fd, offset, SEEK_SET) == -1) {
    return -1;
  }
  ssize_t bytes_read = read(fd, buffer, size);
  if ((lseek(fd, old_position, SEEK_SET)) == -1) {
    return -1;
  }
  return bytes_read;
}

ssize_t pwrite(int fd, const void* buffer, size_t size, off_t offset) {
  off_t cpos, opos;
  off_t old_position = lseek(fd, 0, SEEK_CUR);
  if (old_position == -1) {
    return -1;
  }
  if (lseek(fd, offset, SEEK_SET) == -1) {
    return -1;
  }
  ssize_t bytes_written = write(fd, buffer, size);
  if (lseek(fd, old_position, SEEK_SET) == -1) {
    return -1;
  }
  return bytes_written;
}

}  // namespace

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

  CHECK(bytes_left == 0);
  return outcome::success();
}

ErrorMessageOr<void> WriteFully(const unique_fd& fd, std::string_view content) {
  return WriteFully(fd, content.data(), content.size());
}

ErrorMessageOr<void> WriteFullyAtOffset(const unique_fd& fd, const void* buffer, size_t size,
                                        off_t offset) {
  const char* current_position = static_cast<const char*>(buffer);

  while (size > 0) {
    int64_t bytes_written = TEMP_FAILURE_RETRY(pwrite(fd.get(), current_position, size, offset));
    CHECK(bytes_written != 0);
    if (bytes_written == -1) {
      return ErrorMessage{SafeStrerror(errno)};
    }
    current_position += bytes_written;
    offset += bytes_written;
    size -= bytes_written;
  }

  CHECK(size == 0);
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

ErrorMessageOr<size_t> ReadFullyAtOffset(const unique_fd& fd, void* buffer, size_t size,
                                         off_t offset) {
  uint8_t* current_position = static_cast<uint8_t*>(buffer);
  size_t bytes_read = 0;
  while (bytes_read < size) {
    int64_t pread_result = TEMP_FAILURE_RETRY(pread(fd.get(), current_position, size, offset));
    if (pread_result == -1) {
      return ErrorMessage{SafeStrerror(errno)};
    }
    if (pread_result == 0) {
      break;
    }
    bytes_read += pread_result;
    current_position += pread_result;
    size -= pread_result;
    offset += pread_result;
  }

  return bytes_read;
}

ErrorMessageOr<bool> FileExists(const std::filesystem::path& path) {
  std::error_code error;
  bool result = std::filesystem::exists(path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return result;
}

ErrorMessageOr<void> MoveFile(const std::filesystem::path& from, const std::filesystem::path& to) {
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

ErrorMessageOr<bool> CreateDirectory(const std::filesystem::path& file_path) {
  std::error_code error;
  bool created = std::filesystem::create_directories(file_path, error);
  if (error) {
    return ErrorMessage{error.message()};
  }

  return created;
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

}  // namespace orbit_base