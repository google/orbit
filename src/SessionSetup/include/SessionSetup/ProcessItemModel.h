// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_PROCESS_ITEM_MODEL_H_
#define SESSION_SETUP_PROCESS_ITEM_MODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtCore>

#include "GrpcProtos/process.pb.h"

namespace orbit_session_setup {

class ProcessItemModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  enum class Column { kName, kPid, kCpu, kEnd };

  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
  [[nodiscard]] QModelIndex index(int row, int column,
                                  const QModelIndex& parent = {}) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex& parent) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;

  void SetProcesses(QVector<orbit_grpc_protos::ProcessInfo> processes);
  [[nodiscard]] bool HasProcesses() const { return !processes_.empty(); }
  void Clear() { SetProcesses({}); }

 private:
  QVector<orbit_grpc_protos::ProcessInfo> processes_;
};

}  // namespace orbit_session_setup

Q_DECLARE_METATYPE(const orbit_grpc_protos::ProcessInfo*);

#endif  // SESSION_SETUP_PROCESS_ITEM_MODEL_H_