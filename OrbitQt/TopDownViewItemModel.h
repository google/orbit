// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TOP_DOWN_VIEW_ITEM_MODEL_H_
#define ORBIT_QT_TOP_DOWN_VIEW_ITEM_MODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <memory>

#include "TopDownView.h"

class TopDownViewItemModel : public QAbstractItemModel {
  Q_OBJECT

 public:
  explicit TopDownViewItemModel(std::unique_ptr<TopDownView> top_down_view,
                                QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;

  enum Columns {
    kThreadOrFunction = 0,
    kInclusive,
    kExclusive,
    kOfParent,
    kFunctionAddress,
    kColumnCount
  };

 private:
  std::unique_ptr<TopDownView> top_down_view_;
};

#endif  // ORBIT_QT_TOP_DOWN_VIEW_ITEM_MODEL_H_
