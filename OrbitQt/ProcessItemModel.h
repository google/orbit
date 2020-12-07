// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_PROCESS_ITEM_MODEL_H_
#define ORBIT_QT_PROCESS_ITEM_MODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtCore>
#include <vector>

#include "process.pb.h"

namespace OrbitQt {

class ProcessItemModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  enum class Column { kName, kPid, kCpu, kEnd };

  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& parent) const override;
  int rowCount(const QModelIndex& parent = {}) const override;

  void SetProcesses(std::vector<orbit_grpc_protos::ProcessInfo> processes);
  bool HasProcesses() const { return !processes_.empty(); }
  void Clear() { SetProcesses({}); }

 private:
  std::vector<orbit_grpc_protos::ProcessInfo> processes_;
};

}  // namespace OrbitQt

Q_DECLARE_METATYPE(const orbit_grpc_protos::ProcessInfo*);

#endif  // ORBIT_QT_PROCESS_ITEM_MODEL_H_