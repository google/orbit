// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_INFO_CAPTURE_FILE_INFO_H_
#define CAPTURE_FILE_INFO_CAPTURE_FILE_INFO_H_

#include <QDateTime>
#include <QFileInfo>
#include <filesystem>
#include <utility>

namespace orbit_capture_file_info {

class CaptureFileInfo {
 public:
  explicit CaptureFileInfo(const QString& path);
  explicit CaptureFileInfo(const QString& path, QDateTime last_used);

  [[nodiscard]] QString FilePath() const { return file_info_.filePath(); }
  [[nodiscard]] QString FileName() const { return file_info_.fileName(); }

  [[nodiscard]] QDateTime LastUsed() const { return last_used_; }
  [[nodiscard]] QDateTime Created() const { return file_info_.birthTime(); }

  [[nodiscard]] bool FileExists() const;

  [[nodiscard]] uint64_t FileSize() const;

  void Touch() { last_used_ = QDateTime::currentDateTime(); }

 private:
  QFileInfo file_info_;
  QDateTime last_used_;
};

}  // namespace orbit_capture_file_info

#endif  // CAPTURE_FILE_INFO_CAPTURE_FILE_INFO_H_
