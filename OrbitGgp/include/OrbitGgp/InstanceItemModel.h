// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_INSTANCE_ITEM_MODEL_H_
#define ORBIT_GGP_INSTANCE_ITEM_MODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QVector>

#include "Instance.h"
#include "OrbitBase/Logging.h"

namespace OrbitGgp {

class InstanceItemModel : public QAbstractItemModel {
 public:
  explicit InstanceItemModel(QVector<Instance> instances = {}, QObject* parent = nullptr);

  void SetInstances(QVector<Instance> instances);

  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int col, const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& parent) const override;
  int rowCount(const QModelIndex& parent = {}) const override;

  int GetRowOfInstanceById(const QString& instance_id);

 private:
  QVector<Instance> instances_;
};

}  // namespace OrbitGgp

#endif  // ORBIT_GGP_INSTANCE_ITEM_MODEL_H_
