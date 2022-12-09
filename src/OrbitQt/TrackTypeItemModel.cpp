// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/TrackTypeItemModel.h"

#include <QAbstractItemModel>
#include <algorithm>

#include "OrbitBase/Logging.h"

namespace orbit_qt {

TrackTypeItemModel::TrackTypeItemModel(QObject* parent)
    : QAbstractTableModel(parent),
      known_track_types_{Track::Type::kSchedulerTrack,  Track::Type::kGpuTrack,
                         Track::Type::kFrameTrack,      Track::Type::kMemoryTrack,
                         Track::Type::kPageFaultsTrack, Track::Type::kThreadTrack,
                         Track::Type::kAsyncTrack,      Track::Type::kVariableTrack} {}

int TrackTypeItemModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(Column::kEnd);
}

QVariant TrackTypeItemModel::data(const QModelIndex& idx, int role) const {
  ORBIT_CHECK(idx.isValid());
  ORBIT_CHECK(idx.model() == static_cast<const QAbstractItemModel*>(this));
  ORBIT_CHECK(idx.row() >= 0 && idx.row() < static_cast<int>(known_track_types_.size()));
  ORBIT_CHECK(idx.column() >= 0 && idx.column() < static_cast<int>(Column::kEnd));

  Track::Type track_type = known_track_types_.at(idx.row());
  auto col = static_cast<Column>(idx.column());

  switch (role) {
    case kTrackTypeRole:
      return QVariant::fromValue(track_type);
    case Qt::DisplayRole:
      if (col == Column::kName) {
        return GetTrackTypeDisplayName(track_type);
      }
      break;
    case Qt::CheckStateRole:
      if (col == Column::kVisibility) {
        return track_manager_->GetTrackTypeVisibility(track_type) ? Qt::Checked : Qt::Unchecked;
      }
      break;
  }

  return {};
}

bool TrackTypeItemModel::setData(const QModelIndex& idx, const QVariant& value, int role) {
  ORBIT_CHECK(idx.isValid());
  ORBIT_CHECK(idx.model() == static_cast<const QAbstractItemModel*>(this));
  ORBIT_CHECK(idx.row() >= 0 && idx.row() < static_cast<int>(known_track_types_.size()));
  ORBIT_CHECK(idx.column() >= 0 && idx.column() < static_cast<int>(Column::kEnd));

  Track::Type track_type = known_track_types_.at(idx.row());
  auto col = static_cast<Column>(idx.column());

  switch (role) {
    case Qt::CheckStateRole:
      if (col == Column::kVisibility) {
        track_manager_->SetTrackTypeVisibility(track_type,
                                               value.value<Qt::CheckState>() == Qt::Checked);
        return true;
      }
      break;
  }

  return false;
}

QVariant TrackTypeItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical) {
    return {};
  }

  if (role == Qt::DisplayRole) {
    switch (static_cast<Column>(section)) {
      case Column::kVisibility:
        return "Visibility";
      case Column::kName:
        return "Track Type";
      case Column::kEnd:
        ORBIT_UNREACHABLE();
    }
  }
  return {};
}

int TrackTypeItemModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }

  if (track_manager_ == nullptr) {
    return 0;
  }

  return known_track_types_.size();
}

Qt::ItemFlags TrackTypeItemModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags flags = QAbstractTableModel::flags(index);
  if (index.column() == static_cast<int>(Column::kVisibility)) {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}

void TrackTypeItemModel::SetTrackManager(orbit_gl::TrackManager* track_manager) {
  int row_count = rowCount();
  if (row_count > 0) {
    beginRemoveRows({}, 0, rowCount() - 1);
    track_manager_ = nullptr;
    endRemoveRows();
  }
  beginInsertRows({}, 0, known_track_types_.size() - 1);
  track_manager_ = track_manager;
  endInsertRows();
}

QString TrackTypeItemModel::GetTrackTypeDisplayName(Track::Type track_type) {
  switch (track_type) {
    case Track::Type::kSchedulerTrack:
      return "Scheduler";
    case Track::Type::kGpuTrack:
      return "GPU Information";
    case Track::Type::kFrameTrack:
      return "Frame Tracks";
    case Track::Type::kMemoryTrack:
      return "Memory Usage";
    case Track::Type::kPageFaultsTrack:
      return "Page Faults";
    case Track::Type::kThreadTrack:
      return "Threads";
    case Track::Type::kAsyncTrack:
      return "Async Events (Manual Instrumentation)";
    case Track::Type::kVariableTrack:
      return "Variables (Manual Instrumentation)";
    case Track::Type::kGraphTrack:
      [[fallthrough]];
    case Track::Type::kTimerTrack:
      [[fallthrough]];
    case Track::Type::kUnknown:
      ORBIT_UNREACHABLE();
  };

  ORBIT_UNREACHABLE();
}

}  // namespace orbit_qt