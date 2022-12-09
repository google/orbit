// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_TABLE_MODEL_H_
#define ORBIT_QT_ORBIT_TABLE_MODEL_H_

#include <absl/types/span.h>

#include <QAbstractTableModel>
#include <QFlags>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "DataViews/DataView.h"

class OrbitTableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit OrbitTableModel(orbit_data_views::DataView* data_view, QObject* parent = nullptr,
                           QFlags<Qt::AlignmentFlag> text_alignment = Qt::AlignVCenter |
                                                                      Qt::AlignLeft);
  explicit OrbitTableModel(QObject* parent = nullptr);

  [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  int GetUpdatePeriodMs() { return data_view_->GetUpdatePeriodMs(); }
  [[nodiscard]] std::vector<int> GetVisibleSelectedIndices() const {
    return data_view_->GetVisibleSelectedIndices();
  }
  QModelIndex CreateIndex(int row, int column) { return createIndex(row, column); }
  orbit_data_views::DataView* GetDataView() { return data_view_; }
  void SetDataView(orbit_data_views::DataView* model) { data_view_ = model; }
  bool IsSortingAllowed() { return GetDataView()->IsSortingAllowed(); }
  std::pair<int, Qt::SortOrder> GetDefaultSortingColumnAndOrder();

  void OnTimer();
  void OnFilter(const QString& filter);
  void OnRowsSelected(absl::Span<const int> rows);

 protected:
  orbit_data_views::DataView* data_view_;
  QFlags<Qt::AlignmentFlag> text_alignment_;
};

#endif  // ORBIT_QT_ORBIT_TABLE_MODEL_H_
