// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/CaptureFileInfo.h"

namespace orbit_capture_file_info {

CaptureFileInfo::CaptureFileInfo(const QString& path)
    : file_info_(path), last_used_(QDateTime::currentDateTime()) {}

CaptureFileInfo::CaptureFileInfo(const QString& path, QDateTime last_used)
    : file_info_(path), last_used_(std::move(last_used)) {}

bool CaptureFileInfo::FileExists() const { return file_info_.exists() && file_info_.isFile(); }

uint64_t CaptureFileInfo::FileSize() const { return file_info_.size(); }

}  // namespace orbit_capture_file_info