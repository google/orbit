// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_INFO_ITEM_MODEL_H_
#define CAPTURE_FILE_INFO_ITEM_MODEL_H_

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <vector>

#include "CaptureFileInfo/CaptureFileInfo.h"

namespace orbit_capture_file_info {

class ItemModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  enum class Column { kFilename, kLastUsed, kCreated, kCaptureLength, kEnd };

  explicit ItemModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

  void SetCaptureFileInfos(std::vector<CaptureFileInfo> capture_file_infos);

  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;

 private:
  std::vector<CaptureFileInfo> capture_files_;
};

}  // namespace orbit_capture_file_info

#endif  // CAPTURE_FILE_INFO_ITEM_MODEL_H_