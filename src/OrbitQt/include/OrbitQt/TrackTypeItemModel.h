// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TRACK_TYPE_ITEM_MODEL_H_
#define ORBIT_QT_TRACK_TYPE_ITEM_MODEL_H_

#include <QAbstractTableModel>
#include <QIcon>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtCore>
#include <array>
#include <map>
#include <vector>

#include "OrbitGl/Track.h"
#include "OrbitGl/TrackManager.h"

namespace orbit_qt {

class TrackTypeItemModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  static const int kTrackTypeRole = Qt::UserRole;

  enum class Column { kVisibility, kName, kEnd };

  explicit TrackTypeItemModel(QObject* parent = nullptr);

  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  [[nodiscard]] bool setData(const QModelIndex& idx, const QVariant& value,
                             int role = Qt::EditRole) override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

  void SetTrackManager(orbit_gl::TrackManager* track_manager);

 private:
  orbit_gl::TrackManager* track_manager_ = nullptr;
  std::vector<Track::Type> known_track_types_;

  static QString GetTrackTypeDisplayName(Track::Type track_type);
};

}  // namespace orbit_qt

Q_DECLARE_METATYPE(Track::Type);

#endif