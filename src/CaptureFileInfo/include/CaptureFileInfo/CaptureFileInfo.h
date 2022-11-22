// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_INFO_CAPTURE_FILE_INFO_H_
#define CAPTURE_FILE_INFO_CAPTURE_FILE_INFO_H_

#include <absl/time/time.h>
#include <stdint.h>

#include <QDateTime>
#include <QFileInfo>
#include <QString>
#include <filesystem>
#include <optional>
#include <utility>

namespace orbit_capture_file_info {

class CaptureFileInfo {
 public:
  explicit CaptureFileInfo(const QString& path, std::optional<absl::Duration> capture_length);
  explicit CaptureFileInfo(const QString& path, QDateTime last_used,
                           std::optional<absl::Duration> capture_length);
  explicit CaptureFileInfo(const QString& path, QDateTime last_used, QDateTime last_modified,
                           uint64_t file_size, std::optional<absl::Duration> capture_length);

  [[nodiscard]] QString FilePath() const { return file_info_.filePath(); }
  [[nodiscard]] QString FileName() const { return file_info_.fileName(); }
  [[nodiscard]] QDateTime Created() const { return file_info_.birthTime(); }

  [[nodiscard]] QDateTime LastUsed() const { return last_used_; }
  [[nodiscard]] QDateTime LastModified() const { return last_modified_; }
  [[nodiscard]] uint64_t FileSize() const { return file_size_; }
  [[nodiscard]] std::optional<absl::Duration> CaptureLength() const { return capture_length_; }

  [[nodiscard]] bool FileExists() const;
  [[nodiscard]] bool IsOutOfSync() const;

  void Touch();
  void SetCaptureLength(std::optional<absl::Duration> capture_length) {
    capture_length_ = capture_length;
  }

 private:
  QFileInfo file_info_;
  // Last used time inside Orbit
  QDateTime last_used_;
  // Last modified time inside Orbit
  QDateTime last_modified_;
  uint64_t file_size_;
  std::optional<absl::Duration> capture_length_ = std::nullopt;
};

}  // namespace orbit_capture_file_info

#endif  // CAPTURE_FILE_INFO_CAPTURE_FILE_INFO_H_
