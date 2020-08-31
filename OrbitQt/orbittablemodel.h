// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_TABLE_MODEL_H_
#define ORBIT_QT_ORBIT_TABLE_MODEL_H_

#include <QAbstractTableModel>
#include <memory>
#include <utility>

#include "DataView.h"

class OrbitTableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit OrbitTableModel(DataView* data_view, QObject* parent = nullptr,
                           QFlags<Qt::AlignmentFlag> text_alignment = Qt::AlignVCenter |
                                                                      Qt::AlignLeft);
  explicit OrbitTableModel(QObject* parent = nullptr);

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  int GetUpdatePeriodMs() { return data_view_->GetUpdatePeriodMs(); }
  int GetSelectedIndex() { return data_view_->GetSelectedIndex(); }
  QModelIndex CreateIndex(int row, int column) { return createIndex(row, column); }
  DataView* GetDataView() { return data_view_; }
  void SetDataView(DataView* model) { data_view_ = model; }
  bool IsSortingAllowed() { return GetDataView()->IsSortingAllowed(); }
  std::pair<int, Qt::SortOrder> GetDefaultSortingColumnAndOrder();

  void OnTimer();
  void OnFilter(const QString& filter);
  void OnRowSelected(int row);

 protected:
  DataView* data_view_;
  QFlags<Qt::AlignmentFlag> text_alignment_;
};

#endif  // ORBIT_QT_ORBIT_TABLE_MODEL_H_
