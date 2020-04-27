// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/GgpInstanceItemModel.h"

namespace {
enum class Columns {
  DisplayName,
  ID,
  IPAddress,
  LastUpdated,
  Owner,
  Pool,
  NumberOfColumns
};
}  // namespace

GgpInstanceItemModel::GgpInstanceItemModel(QVector<GgpInstance> instances,
                                           QObject* parent)
    : QAbstractItemModel(parent), instances_(std::move(instances)) {
  std::sort(instances_.begin(), instances_.end(), &GgpInstance::CmpById);
}

int GgpInstanceItemModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(Columns::NumberOfColumns);
}

QVariant GgpInstanceItemModel::data(const QModelIndex& index, int role) const {
  CHECK(index.isValid());
  CHECK(index.model() == this);
  CHECK(index.row() < instances_.size());  // instances_.size());

  const GgpInstance& current_instance = instances_[index.row()];

  if (role == Qt::UserRole) return QVariant::fromValue(current_instance);

  if (role != Qt::DisplayRole) return {};

  switch (static_cast<Columns>(index.column())) {
    case Columns::DisplayName:
      return current_instance.display_name;
    case Columns::ID:
      return current_instance.id;
    case Columns::IPAddress:
      return current_instance.ip_address;
    case Columns::LastUpdated:
      return current_instance.last_updated.toString(Qt::TextDate);
    case Columns::Owner:
      return current_instance.owner;
    case Columns::Pool:
      return current_instance.pool;
    case Columns::NumberOfColumns:
      CHECK(false);
      return {};
  }

  CHECK(false);  // That means, someone (me?) forgot a column.
  return {};
}

QModelIndex GgpInstanceItemModel::index(int row, int col,
                                        const QModelIndex& parent) const {
  if (parent.isValid()) return {};
  if (row < 0 || row >= instances_.size()) return {};
  if (col < 0 || col >= static_cast<int>(Columns::NumberOfColumns)) return {};

  return createIndex(row, col, nullptr);
}

QVariant GgpInstanceItemModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (role != Qt::DisplayRole) return {};
  if (orientation != Qt::Horizontal) return {};
  if (section < 0 || section >= static_cast<int>(Columns::NumberOfColumns)) {
    return {};
  }

  switch (static_cast<Columns>(section)) {
    case Columns::DisplayName:
      return QLatin1String("Display Name");
    case Columns::ID:
      return QLatin1String("ID");
    case Columns::IPAddress:
      return QLatin1String("IP Address");
    case Columns::LastUpdated:
      return QLatin1String("Last Updated");
    case Columns::Owner:
      return QLatin1String("Owner");
    case Columns::Pool:
      return QLatin1String("Pool");
    case Columns::NumberOfColumns:
      CHECK(false);
      return {};
  }

  CHECK(false);
  return {};
}

QModelIndex GgpInstanceItemModel::parent(const QModelIndex& /*child*/) const {
  return {};
}

int GgpInstanceItemModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : instances_.size();
}

void GgpInstanceItemModel::SetInstances(QVector<GgpInstance> new_instances) {
  std::sort(new_instances.begin(), new_instances.end(), &GgpInstance::CmpById);

  QVector<GgpInstance>& old_instances(instances_);

  QVector<GgpInstance>::iterator old_iter = old_instances.begin();
  QVector<GgpInstance>::iterator new_iter = new_instances.begin();

  while (old_iter != old_instances.end() && new_iter != new_instances.end()) {
    const int current_row =
        static_cast<int>(std::distance(old_instances.begin(), old_iter));

    if (old_iter->id == new_iter->id) {
      if (*old_iter != *new_iter) {
        *old_iter = *new_iter;
        emit dataChanged(
            index(current_row, 0, {}),
            index(current_row, static_cast<int>(Columns::NumberOfColumns) - 1,
                  {}));
      }
      ++old_iter;
      ++new_iter;
    } else if (old_iter->id < new_iter->id) {
      beginRemoveRows({}, current_row, current_row);
      old_iter = old_instances.erase(old_iter);
      endRemoveRows();
    } else {
      beginInsertRows({}, current_row, current_row);
      old_iter = old_instances.insert(old_iter, *new_iter);
      ++old_iter;
      ++new_iter;
      endInsertRows();
    }
  }

  if (old_iter == old_instances.end() && new_iter != new_instances.end()) {
    beginInsertRows({}, old_instances.size(), new_instances.size() - 1);
    std::copy(new_iter, new_instances.end(), std::back_inserter(old_instances));
    CHECK(old_instances.size() == new_instances.size());
    endInsertRows();
  } else if (old_iter != old_instances.end() &&
             new_iter == new_instances.end()) {
    beginRemoveRows({}, new_instances.size(), old_instances.size() - 1);
    old_instances.erase(old_iter, old_instances.end());
    CHECK(old_instances.size() == new_instances.size());
    endRemoveRows();
  }
}