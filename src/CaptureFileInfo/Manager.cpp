// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/Manager.h"

#include <QDateTime>
#include <QSettings>
#include <algorithm>

#include "CaptureFileInfo/CaptureFileInfo.h"

constexpr const char* kCaptureFileInfoArrayKey = "capture_file_infos";
constexpr const char* kCaptureFileInfoPathKey = "capture_file_info_path";
constexpr const char* kCaptureFileInfoLastUsedKey = "capture_file_info_last_used";

namespace orbit_capture_file_info {

Manager::Manager() {
  LoadCaptureFileInfos();
  PurgeNonExistingFiles();
}

void Manager::LoadCaptureFileInfos() {
  QSettings settings{};
  const int size = settings.beginReadArray(kCaptureFileInfoArrayKey);
  capture_file_infos_.clear();
  capture_file_infos_.reserve(size);

  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    QString path = settings.value(kCaptureFileInfoPathKey).toString();
    QDateTime last_used(settings.value(kCaptureFileInfoLastUsedKey).toDateTime());
    capture_file_infos_.emplace_back(path, std::move(last_used));
  }
  settings.endArray();
}

void Manager::SaveCaptureFileInfos() {
  QSettings settings{};
  settings.beginWriteArray(kCaptureFileInfoArrayKey, static_cast<int>(capture_file_infos_.size()));
  for (size_t i = 0; i < capture_file_infos_.size(); ++i) {
    settings.setArrayIndex(i);
    const CaptureFileInfo& capture_file_info = capture_file_infos_[i];
    settings.setValue(kCaptureFileInfoPathKey, capture_file_info.FilePath());
    settings.setValue(kCaptureFileInfoLastUsedKey, capture_file_info.LastUsed());
  }
  settings.endArray();
}

void Manager::AddOrTouchCaptureFile(const std::filesystem::path& path) {
  auto it = std::find_if(capture_file_infos_.begin(), capture_file_infos_.end(),
                         [&](const CaptureFileInfo& capture_file_info) {
                           return capture_file_info.FilePath().toStdString() == path.string();
                         });

  if (it == capture_file_infos_.end()) {
    capture_file_infos_.emplace_back(QString::fromStdString(path.string()));
  } else {
    it->Touch();
  }

  SaveCaptureFileInfos();
}

void Manager::Clear() {
  capture_file_infos_.clear();
  SaveCaptureFileInfos();
}

void Manager::PurgeNonExistingFiles() {
  capture_file_infos_.erase(std::remove_if(capture_file_infos_.begin(), capture_file_infos_.end(),
                                           [](const CaptureFileInfo& capture_file_info) {
                                             return !capture_file_info.FileExists();
                                           }),
                            capture_file_infos_.end());
  SaveCaptureFileInfos();
}

}  // namespace orbit_capture_file_info