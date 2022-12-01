// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SourcePathsMapping/MappingItemModel.h"

#include <QFlags>
#include <QModelIndex>
#include <QVariant>
#include <algorithm>
#include <filesystem>
#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_source_paths_mapping {

void MappingItemModel::SetMappings(std::vector<Mapping> new_mappings) {
  if (!mappings_.empty()) {
    beginRemoveRows(QModelIndex{}, 0, rowCount() - 1);
    mappings_.clear();
    endRemoveRows();
  }

  if (!new_mappings.empty()) {
    beginInsertRows(QModelIndex{}, 0, static_cast<int>(new_mappings.size()) - 1);
    mappings_ = std::move(new_mappings);
    endInsertRows();
  }
}

Qt::ItemFlags MappingItemModel::flags(const QModelIndex& index) const {
  if (index.isValid()) return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled;

  return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
}

QVariant MappingItemModel::data(const QModelIndex& index, int role) const {
  ORBIT_CHECK(index.model() == this);
  ORBIT_CHECK(index.row() < static_cast<int>(mappings_.size()));

  const auto& mapping = mappings_.at(index.row());

  if (role == Qt::DisplayRole) {
    auto source = QString::fromStdString(mapping.source_path.string());
    if (source.isEmpty()) source = "{new source path}";

    auto target = QString::fromStdString(mapping.target_path.string());
    if (target.isEmpty()) target = "{new target path}";

    return QString{"%1 -> %2"}.arg(source, target);
  }

  if (role == Qt::UserRole) return QVariant::fromValue(&mapping);

  return {};
}

QVariant MappingItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && section == 0 && role == Qt::DisplayRole) {
    return "Source Path Mapping";
  }

  return {};
}

bool MappingItemModel::moveRows(const QModelIndex& source_parent, int source_row, int count,
                                const QModelIndex& destination_parent, int destination_child) {
  ORBIT_CHECK(!source_parent.isValid());
  ORBIT_CHECK(!destination_parent.isValid());
  // We don't have to support moving more than a single row.
  if (count != 1) return false;

  // When the destination is part of the source selection, the move is a no-op. We have to return
  // true in that case.
  if (source_row <= destination_child && destination_child <= source_row + count) return true;

  beginMoveRows(source_parent, source_row, source_row + count - 1, destination_parent,
                destination_child);
  if (destination_child >= source_row + count) {
    std::rotate(mappings_.begin() + source_row, mappings_.begin() + source_row + count,
                mappings_.begin() + destination_child);
  } else {
    std::rotate(mappings_.begin() + destination_child, mappings_.begin() + source_row,
                mappings_.begin() + source_row + count);
  }
  endMoveRows();
  return true;
}

bool MappingItemModel::RemoveRows(int row, int count, const QModelIndex& parent) {
  // We don't have a tree structure, so the parent can't be valid.
  if (parent.isValid()) return false;
  if (row >= rowCount()) return false;
  if (row + count > rowCount()) return false;

  beginRemoveRows(parent, row, row + count - 1);
  mappings_.erase(mappings_.begin() + row, mappings_.begin() + row + count);
  endRemoveRows();
  return true;
}

bool MappingItemModel::setData(const QModelIndex& idx, const QVariant& value, int role) {
  ORBIT_CHECK(idx.isValid());
  ORBIT_CHECK(idx.model() == this);

  if (role != Qt::EditRole) return false;
  if (!value.canConvert<Mapping>()) return false;

  mappings_[idx.row()] = value.value<Mapping>();
  emit dataChanged(index(idx.row(), 0, {}), index(idx.row(), 0, {}));
  return true;
}

void MappingItemModel::AppendNewEmptyMapping() {
  beginInsertRows({}, rowCount(), rowCount());
  mappings_.emplace_back(Mapping{});
  endInsertRows();
}

}  // namespace orbit_source_paths_mapping
