// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/TemporaryFile.h"

#include <stdlib.h>

namespace orbit_base {

ErrorMessageOr<TemporaryFile> TemporaryFile::Create() {
  TemporaryFile temporary_file;
  auto init_result = temporary_file.Init();
  if (init_result.has_error()) {
    return init_result.error();
  }

  return temporary_file;
}

ErrorMessageOr<void> TemporaryFile::Init() {
  std::error_code error;
  std::filesystem::path temporary_dir = std::filesystem::temp_directory_path(error);
  if (error) {
    return ErrorMessage{absl::StrFormat("Unable to get temporary dir: %s", error.message())};
  }
  std::string file_path = (temporary_dir / "orbit_XXXXXX").string();

#if defined(__linux)
  int fd = mkostemp(file_path.data(), O_CLOEXEC);

  if (fd == -1) {
    return ErrorMessage{
        absl::StrFormat("Unable to create a temporary file: %s", SafeStrerror(errno))};
  }

  fd_ = unique_fd(fd);
#elif defined(_WIN32)
  // _mktemp_s requires string to be null-terminated.
  file_path += '\0';
  errno_t errnum = _mktemp_s(file_path.data(), file_path.size());
  if (errnum != 0) {
    return ErrorMessage{
        absl::StrFormat("Unable to create a temporary file: %s", SafeStrerror(errnum))};
  }

  auto fd_or_error = OpenNewFileForReadWrite(file_path);
  if (fd_or_error.has_error()) {
    return ErrorMessage{
        absl::StrFormat("Unable to create a temporary file: %s", fd_or_error.error().message())};
  }

  fd_ = std::move(fd_or_error.value());
#endif  // defined(__linux)

  file_path_ = file_path;

  return outcome::success();
}

void TemporaryFile::CloseAndRemove() noexcept {
  fd_.release();
  if (!file_path_.empty()) {
    unlink(file_path_.string().c_str());
  }
}

}  // namespace orbit_base