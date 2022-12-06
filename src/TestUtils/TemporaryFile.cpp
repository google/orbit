// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestUtils/TemporaryFile.h"

#include <absl/strings/str_format.h>
#include <errno.h>
#include <stdlib.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

#include "OrbitBase/SafeStrerror.h"

#ifdef __linux
#include <fcntl.h>
#endif

namespace orbit_test_utils {

ErrorMessageOr<TemporaryFile> TemporaryFile::Create(std::string_view prefix) {
  TemporaryFile temporary_file;
  auto init_result = temporary_file.Init(prefix);
  if (init_result.has_error()) {
    return init_result.error();
  }

  return temporary_file;
}

ErrorMessageOr<void> TemporaryFile::Init(std::string_view prefix) {
  std::error_code error;
  std::filesystem::path temporary_dir = std::filesystem::temp_directory_path(error);
  if (error) {
    return ErrorMessage{absl::StrFormat("Unable to get temporary dir: %s", error.message())};
  }
  if (prefix.empty()) prefix = "orbit";
  std::string file_path = (temporary_dir / absl::StrFormat("%s_XXXXXX", prefix)).string();

#if defined(__linux)
  int fd = mkostemp(file_path.data(), O_CLOEXEC);

  if (fd == -1) {
    return ErrorMessage{
        absl::StrFormat("Unable to create a temporary file: %s", SafeStrerror(errno))};
  }

  fd_ = orbit_base::unique_fd(fd);
#elif defined(_WIN32)
  // _mktemp_s expects the `size` to include the NULL character at the end.
  errno_t errnum = _mktemp_s(file_path.data(), file_path.size() + 1);
  if (errnum != 0) {
    return ErrorMessage{
        absl::StrFormat("Unable to create a temporary file: %s", SafeStrerror(errnum))};
  }

  auto fd_or_error = orbit_base::OpenNewFileForReadWrite(file_path);
  if (fd_or_error.has_error()) {
    return ErrorMessage{
        absl::StrFormat("Unable to create a temporary file: %s", fd_or_error.error().message())};
  }

  fd_ = std::move(fd_or_error.value());
#endif  // defined(__linux)

  file_path_ = file_path;

  return outcome::success();
}

void TemporaryFile::CloseAndRemove() {
  fd_.release();
  if (!file_path_.empty()) {
    std::error_code err{};
    std::filesystem::remove(file_path_, err);
  }
}

}  // namespace orbit_test_utils