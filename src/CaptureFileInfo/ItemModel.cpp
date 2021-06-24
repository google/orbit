// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/ItemModel.h"

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"

namespace orbit_capture_file_info {

void ItemModel::SetCaptureFileInfos(std::vector<CaptureFileInfo> capture_file_infos) {
  if (!capture_files_.empty()) {
    beginRemoveRows(QModelIndex{}, 0, rowCount() - 1);
    capture_files_.clear();
    endRemoveRows();
  }

  if (!capture_file_infos.empty()) {
    beginInsertRows(QModelIndex{}, 0, static_cast<int>(capture_file_infos.size()) - 1);
    capture_files_ = std::move(capture_file_infos);
    endInsertRows();
  }
}

int ItemModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(Column::kEnd);
}

int ItemModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(capture_files_.size());
}

QVariant ItemModel::data(const QModelIndex& idx, int role) const {
  CHECK(idx.isValid());
  CHECK(idx.model() == this);
  CHECK(idx.row() >= 0 && idx.row() < static_cast<int>(capture_files_.size()));
  CHECK(idx.column() >= 0 && idx.column() < static_cast<int>(Column::kEnd));

  const CaptureFileInfo& capture_file_info = capture_files_.at(idx.row());

  if (role == Qt::UserRole) {
    return capture_file_info.FilePath();
  }

  if (role == Qt::DisplayRole) {
    switch (static_cast<Column>(idx.column())) {
      case Column::kFilename:
        return capture_file_info.FileName();
      case Column::kLastUsed:
        return capture_file_info.LastUsed();
      case Column::kCreated:
        return capture_file_info.Created();
      case Column::kEnd:
        UNREACHABLE();
    }
  }

  if (role == Qt::ToolTipRole) {
    return QString::fromStdString("%1 - %2")
        .arg(QString::fromStdString(
            orbit_display_formats::GetDisplaySize(capture_file_info.FileSize())))
        .arg(capture_file_info.FilePath());
  }

  return {};
}

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
    return {};
  }

  switch (static_cast<Column>(section)) {
    case Column::kFilename:
      return "Filename";
    case Column::kLastUsed:
      return "Last used";
    case Column::kCreated:
      return "Created";
    case Column::kEnd:
      UNREACHABLE();
  }
  UNREACHABLE();
}

}  // namespace orbit_capture_file_info