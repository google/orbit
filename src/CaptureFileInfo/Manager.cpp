// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/Manager.h"

#include <absl/time/time.h>
#include <stddef.h>
#include <stdint.h>

#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <algorithm>
#include <utility>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "OrbitBase/File.h"

constexpr const char* kCaptureFileInfoArrayKey = "capture_file_infos";
constexpr const char* kCaptureFileInfoPathKey = "capture_file_info_path";
constexpr const char* kCaptureFileInfoLastUsedKey = "capture_file_info_last_used";
constexpr const char* kCaptureFileInfoLastModifiedKey = "capture_file_info_last_modified";
constexpr const char* kCaptureFileInfoFileSizeKey = "capture_file_info_file_size";
constexpr const char* kCaptureFileInfoCaptureLengthKey = "capture_file_info_capture_length";

namespace orbit_capture_file_info {

Manager::Manager() {
  LoadCaptureFileInfos();
  PurgeNonExistingFiles();
  ProcessOutOfSyncFiles();
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
    QDateTime last_modified(settings.value(kCaptureFileInfoLastModifiedKey).toDateTime());
    uint64_t file_size =
        static_cast<uint64_t>(settings.value(kCaptureFileInfoFileSizeKey).toULongLong());

    std::optional<absl::Duration> capture_length = std::nullopt;
    if (settings.contains(kCaptureFileInfoCaptureLengthKey)) {
      capture_length = absl::Nanoseconds(
          static_cast<int64_t>(settings.value(kCaptureFileInfoCaptureLengthKey).toLongLong()));
    }

    capture_file_infos_.emplace_back(path, std::move(last_used), std::move(last_modified),
                                     file_size, capture_length);
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
    settings.setValue(kCaptureFileInfoLastModifiedKey, capture_file_info.LastModified());
    settings.setValue(kCaptureFileInfoFileSizeKey,
                      QVariant::fromValue(capture_file_info.FileSize()));

    if (!capture_file_info.CaptureLength().has_value()) continue;

    int64_t capture_length_ns = absl::ToInt64Nanoseconds(capture_file_info.CaptureLength().value());
    settings.setValue(kCaptureFileInfoCaptureLengthKey, QVariant::fromValue(capture_length_ns));
  }
  settings.endArray();
}

void Manager::AddOrTouchCaptureFile(const std::filesystem::path& path,
                                    std::optional<absl::Duration> capture_length) {
  auto it = std::find_if(capture_file_infos_.begin(), capture_file_infos_.end(),
                         [&](const CaptureFileInfo& capture_file_info) {
                           std::filesystem::path path_from_capture_file_info{
                               capture_file_info.FilePath().toStdString()};
                           return path_from_capture_file_info == path;
                         });

  if (it == capture_file_infos_.end()) {
    capture_file_infos_.emplace_back(QString::fromStdString(path.string()), capture_length);
  } else {
    it->Touch();
    it->SetCaptureLength(capture_length);
  }

  SaveCaptureFileInfos();
}

std::optional<absl::Duration> Manager::GetCaptureLengthByPath(
    const std::filesystem::path& path) const {
  auto it = std::find_if(capture_file_infos_.begin(), capture_file_infos_.end(),
                         [&](const CaptureFileInfo& capture_file_info) {
                           std::filesystem::path path_from_capture_file_info{
                               capture_file_info.FilePath().toStdString()};
                           return path_from_capture_file_info == path;
                         });
  if (it == capture_file_infos_.end()) return std::nullopt;
  return it->CaptureLength();
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

void Manager::ProcessOutOfSyncFiles() {
  std::for_each(capture_file_infos_.begin(), capture_file_infos_.end(),
                [](CaptureFileInfo& capture_file_info) {
                  if (!capture_file_info.IsOutOfSync()) return;
                  capture_file_info.SetCaptureLength(std::nullopt);
                });
  SaveCaptureFileInfos();
}

ErrorMessageOr<void> Manager::FillFromDirectory(const std::filesystem::path& directory) {
  Clear();

  OUTCOME_TRY(auto&& files, orbit_base::ListFilesInDirectory(directory));

  for (const auto& file : files) {
    if (file.extension() != ".orbit") continue;

    QFileInfo tmp_file_info{QString::fromStdString(file.string())};
    capture_file_infos_.emplace_back(tmp_file_info.filePath(), tmp_file_info.birthTime(),
                                     std::nullopt);
  }

  SaveCaptureFileInfos();

  return outcome::success();
}

}  // namespace orbit_capture_file_info