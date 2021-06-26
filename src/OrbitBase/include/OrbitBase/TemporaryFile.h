// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TEMPORARY_FILE_H_
#define ORBIT_BASE_TEMPORARY_FILE_H_

#include "OrbitBase/File.h"

namespace orbit_base {
class TemporaryFile final {
 public:
  ~TemporaryFile() { CloseAndRemove(); }

  TemporaryFile(const TemporaryFile&) = delete;
  TemporaryFile& operator=(const TemporaryFile&) = delete;

  TemporaryFile(TemporaryFile&& that)
      : fd_{std::move(that.fd_)}, file_path_{std::move(that.file_path_)} {
    that.file_path_.clear();
  }

  TemporaryFile& operator=(TemporaryFile&& that) noexcept {
    if (&that == this) return *this;

    CloseAndRemove();
    fd_ = std::move(that.fd_);
    file_path_ = std::move(that.file_path_);
    that.file_path_.clear();

    return *this;
  }

  void CloseAndRemove() noexcept;

  [[nodiscard]] const unique_fd& fd() const { return fd_; }
  [[nodiscard]] const std::filesystem::path& file_path() const { return file_path_; }

  static ErrorMessageOr<TemporaryFile> Create();

 private:
  TemporaryFile() = default;
  ErrorMessageOr<void> Init();

  unique_fd fd_;
  std::filesystem::path file_path_;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_TEMPORARY_FILE_H_
