// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/CaptureFileInfo.h"

#include <utility>

namespace orbit_capture_file_info {

CaptureFileInfo::CaptureFileInfo(const QString& path, std::optional<absl::Duration> capture_length)
    : file_info_(path),
      last_used_(QDateTime::currentDateTime()),
      last_modified_(file_info_.lastModified()),
      file_size_(file_info_.size()),
      capture_length_(capture_length) {}

CaptureFileInfo::CaptureFileInfo(const QString& path, QDateTime last_used,
                                 std::optional<absl::Duration> capture_length)
    : file_info_(path),
      last_used_(std::move(last_used)),
      last_modified_(file_info_.lastModified()),
      file_size_(file_info_.size()),
      capture_length_(capture_length) {}

CaptureFileInfo::CaptureFileInfo(const QString& path, QDateTime last_used, QDateTime last_modified,
                                 uint64_t file_size, std::optional<absl::Duration> capture_length)
    : file_info_(path),
      last_used_(std::move(last_used)),
      last_modified_(std::move(last_modified)),
      file_size_(file_size),
      capture_length_(capture_length) {}

bool CaptureFileInfo::FileExists() const { return file_info_.exists() && file_info_.isFile(); }

bool CaptureFileInfo::IsOutOfSync() const {
  return file_size_ != static_cast<uint64_t>(file_info_.size()) ||
         last_modified_ != file_info_.lastModified();
}

void CaptureFileInfo::Touch() {
  last_used_ = QDateTime::currentDateTime();

  // Note that QFileInfo is not synchronized, it reads the information from the file system when it
  // is created when caching is enabled (by default). We need to call refresh() to refresh the file
  // information from the file system. See QFileInfo::refresh() and QFileInfo::setCaching() for more
  // details.
  file_info_.refresh();
  last_modified_ = file_info_.lastModified();
  file_size_ = static_cast<uint64_t>(file_info_.size());
}

}  // namespace orbit_capture_file_info