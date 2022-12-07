// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TEST_UTILS_TEMPORARY_FILE_H_
#define ORBIT_TEST_UTILS_TEMPORARY_FILE_H_

#include <filesystem>
#include <string_view>
#include <utility>

#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"

namespace orbit_test_utils {
// Creates and opens a temporary file. It will be automatically deleted when this object gets out of
// scope.
class TemporaryFile final {
 public:
  ~TemporaryFile() { CloseAndRemove(); }

  TemporaryFile(const TemporaryFile&) = delete;
  TemporaryFile& operator=(const TemporaryFile&) = delete;

  TemporaryFile(TemporaryFile&& that)
      : fd_{std::move(that.fd_)}, file_path_{std::move(that.file_path_)} {
    that.file_path_.clear();
  }

  TemporaryFile& operator=(TemporaryFile&& that) {
    if (&that == this) return *this;

    CloseAndRemove();
    fd_ = std::move(that.fd_);
    file_path_ = std::move(that.file_path_);
    that.file_path_.clear();

    return *this;
  }

  void CloseAndRemove();

  [[nodiscard]] const orbit_base::unique_fd& fd() const { return fd_; }
  [[nodiscard]] const std::filesystem::path& file_path() const { return file_path_; }

  // Call this function to create a new temporary file. The `prefix` is a component that is
  // guaranteed to be incorporated into the filename. Leave it empty if you have no requirements to
  // the filename.
  static ErrorMessageOr<TemporaryFile> Create(std::string_view prefix = "");

 private:
  TemporaryFile() = default;
  ErrorMessageOr<void> Init(std::string_view prefix);

  orbit_base::unique_fd fd_;
  std::filesystem::path file_path_;
};

}  // namespace orbit_test_utils

#endif  // ORBIT_TEST_UTILS_TEMPORARY_FILE_H_
