// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/CaptureFileInfo.h"

namespace orbit_capture_file_info {

CaptureFileInfo::CaptureFileInfo(const QString& path, std::optional<absl::Duration> capture_length)
    : file_info_(path), last_used_(QDateTime::currentDateTime()), capture_length_(capture_length) {}

CaptureFileInfo::CaptureFileInfo(const QString& path, QDateTime last_used,
                                 std::optional<absl::Duration> capture_length)
    : file_info_(path), last_used_(std::move(last_used)), capture_length_(capture_length) {}

bool CaptureFileInfo::FileExists() const { return file_info_.exists() && file_info_.isFile(); }

uint64_t CaptureFileInfo::FileSize() const { return file_info_.size(); }

}  // namespace orbit_capture_file_info