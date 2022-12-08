// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ProcessItemModel.h"

#include <QFlags>
#include <algorithm>
#include <functional>
#include <iterator>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Sort.h"
#include "google/protobuf/util/message_differencer.h"

namespace orbit_session_setup {

using orbit_grpc_protos::ProcessInfo;

int ProcessItemModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(Column::kEnd);
}

QVariant ProcessItemModel::data(const QModelIndex& idx, int role) const {
  ORBIT_CHECK(idx.isValid());
  ORBIT_CHECK(idx.model() == static_cast<const QAbstractItemModel*>(this));
  ORBIT_CHECK(idx.row() >= 0 && idx.row() < static_cast<int>(processes_.size()));
  ORBIT_CHECK(idx.column() >= 0 && idx.column() < static_cast<int>(Column::kEnd));

  const auto& process = processes_[idx.row()];

  if (role == Qt::UserRole) {
    return QVariant::fromValue(&process);
  }

  if (role == Qt::DisplayRole) {
    switch (static_cast<Column>(idx.column())) {
      case Column::kPid:
        return process.pid();
      case Column::kName:
        return QString::fromStdString(process.name());
      case Column::kCpu:
        return QString("%1 %").arg(process.cpu_usage(), 0, 'f', 1);
      case Column::kEnd:
        ORBIT_UNREACHABLE();
    }
  }

  // When the EditRole is requested, we return the unformatted raw value, which
  // means the CPU Usage is returned as a double.
  if (role == Qt::EditRole) {
    switch (static_cast<Column>(idx.column())) {
      case Column::kPid:
        return process.pid();
      case Column::kName:
        return QString::fromStdString(process.name());
      case Column::kCpu:
        return process.cpu_usage();
      case Column::kEnd:
        ORBIT_UNREACHABLE();
    }
  }

  if (role == Qt::ToolTipRole) {
    // We don't care about the column when showing tooltip. It's the same for
    // the whole row.
    return QString::fromStdString(process.command_line());
  }

  if (role == Qt::TextAlignmentRole) {
    switch (static_cast<Column>(idx.column())) {
      case Column::kPid:
        return QVariant::fromValue(Qt::AlignVCenter | Qt::AlignRight);
      case Column::kName:
        return {};
      case Column::kCpu:
        return QVariant::fromValue(Qt::AlignVCenter | Qt::AlignRight);
      case Column::kEnd:
        ORBIT_UNREACHABLE();
    }
  }

  return {};
}

QVariant ProcessItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical) {
    return {};
  }

  if (role == Qt::DisplayRole) {
    switch (static_cast<Column>(section)) {
      case Column::kPid:
        return "PID";
      case Column::kName:
        return "Name";
      case Column::kCpu:
        return "CPU %";
      case Column::kEnd:
        ORBIT_UNREACHABLE();
    }
  }
  return {};
}

QModelIndex ProcessItemModel::index(int row, int column, const QModelIndex& parent) const {
  if (parent.isValid()) return {};
  if (row < 0 || row >= static_cast<int>(processes_.size())) return {};
  if (column < 0 || column >= static_cast<int>(Column::kEnd)) return {};

  return createIndex(row, column);
}

QModelIndex ProcessItemModel::parent(const QModelIndex& /*parent*/) const { return {}; }

int ProcessItemModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return processes_.size();
}

void ProcessItemModel::SetProcesses(QVector<ProcessInfo> new_processes) {
  orbit_base::sort(new_processes.begin(), new_processes.end(), &ProcessInfo::pid);

  auto* old_iter = processes_.begin();
  auto* new_iter = new_processes.begin();

  while (old_iter != processes_.end() && new_iter != new_processes.end()) {
    const int current_row = static_cast<int>(std::distance(processes_.begin(), old_iter));

    if (old_iter->pid() == new_iter->pid()) {
      if (!google::protobuf::util::MessageDifferencer::Equivalent(*old_iter, *new_iter)) {
        *old_iter = *new_iter;
        emit dataChanged(index(current_row, 0, {}),
                         index(current_row, static_cast<int>(Column::kEnd) - 1, {}));
      }
      ++old_iter;
      ++new_iter;
    } else if (old_iter->pid() < new_iter->pid()) {
      beginRemoveRows({}, current_row, current_row);
      old_iter = processes_.erase(old_iter);
      endRemoveRows();
    } else {
      beginInsertRows({}, current_row, current_row);
      old_iter = processes_.insert(old_iter, *new_iter);
      ++old_iter;
      ++new_iter;
      endInsertRows();
    }
  }

  if (old_iter == processes_.end() && new_iter != new_processes.end()) {
    beginInsertRows({}, processes_.size(), new_processes.size() - 1);
    std::copy(new_iter, new_processes.end(), std::back_inserter(processes_));
    ORBIT_CHECK(processes_.size() == new_processes.size());
    endInsertRows();
  } else if (old_iter != processes_.end() && new_iter == new_processes.end()) {
    beginRemoveRows({}, new_processes.size(), processes_.size() - 1);
    processes_.erase(old_iter, processes_.end());
    ORBIT_CHECK(processes_.size() == new_processes.size());
    endRemoveRows();
  }
}

}  // namespace orbit_session_setup