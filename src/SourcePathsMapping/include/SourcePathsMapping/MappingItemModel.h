// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_PATHS_MAPPING_MAPPING_ITEM_MODEL_H_
#define SOURCE_PATHS_MAPPING_MAPPING_ITEM_MODEL_H_

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtCore>
#include <vector>

#include "SourcePathsMapping/Mapping.h"

namespace orbit_source_paths_mapping {

class MappingItemModel : public QAbstractListModel {
  Q_OBJECT
 public:
  explicit MappingItemModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

  void SetMappings(std::vector<Mapping> new_mappings);
  [[nodiscard]] const std::vector<Mapping>& GetMappings() const { return mappings_; }

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex{}) const override {
    return parent.isValid() ? 0 : static_cast<int>(mappings_.size());
  }

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
  bool moveRows(const QModelIndex& source_parent, int source_row, int count,
                const QModelIndex& destination_parent, int destination_child) override;

  // TODO(b/181733946): This is not a Qt-overload. Calling it `removeRows` triggers an inconsistency
  // in Qt.
  bool RemoveRows(int row, int count, const QModelIndex& parent = QModelIndex{});

  bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override;

  [[nodiscard]] Qt::DropActions supportedDropActions() const override { return Qt::MoveAction; }

  void AppendNewEmptyMapping();

 private:
  std::vector<Mapping> mappings_;
};

}  // namespace orbit_source_paths_mapping

Q_DECLARE_METATYPE(const orbit_source_paths_mapping::Mapping*);
Q_DECLARE_METATYPE(orbit_source_paths_mapping::Mapping);

#endif  // SOURCE_PATHS_MAPPING_MAPPING_ITEM_MODEL_H_
