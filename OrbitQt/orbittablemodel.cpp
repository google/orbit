// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbittablemodel.h"

#include <QColor>
#include <memory>

OrbitTableModel::OrbitTableModel(DataView* data_view, QObject* parent,
                                 QFlags<Qt::AlignmentFlag> text_alignment)
    : QAbstractTableModel(parent), data_view_(data_view), text_alignment_(text_alignment) {}

OrbitTableModel::OrbitTableModel(QObject* parent)
    : QAbstractTableModel(parent),
      data_view_(nullptr),
      text_alignment_(Qt::AlignVCenter | Qt::AlignLeft) {}

int OrbitTableModel::columnCount(const QModelIndex& /*parent*/) const {
  return static_cast<int>(data_view_->GetColumns().size());
}

int OrbitTableModel::rowCount(const QModelIndex& /*parent*/) const {
  return static_cast<int>(data_view_->GetNumElements());
}

QVariant OrbitTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  switch (role) {
    case Qt::DisplayRole:
      if (orientation == Qt::Horizontal &&
          section < static_cast<int>(data_view_->GetColumns().size())) {
        std::string header = data_view_->GetColumns()[section].header;
        return QString::fromStdString(header);
      } else if (orientation == Qt::Vertical) {
        return section;
      } else {
        return QVariant();
      }

    case Qt::InitialSortOrderRole:
      return data_view_->GetColumns()[section].initial_order == DataView::SortingOrder::kAscending
                 ? Qt::AscendingOrder
                 : Qt::DescendingOrder;

    default:
      break;
  }

  return QVariant();
}

QVariant OrbitTableModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    std::string value = data_view_->GetValue(index.row(), index.column());
    return QVariant(QString::fromStdString(value));
  } else if (role == Qt::ForegroundRole) {
    if (data_view_->WantsDisplayColor()) {
      unsigned char r, g, b;
      if (data_view_->GetDisplayColor(index.row(), index.column(), r, g, b)) {
        return QColor(r, g, b);
      }
    }
  } else if (role == Qt::ToolTipRole) {
    std::string tooltip = data_view_->GetToolTip(index.row(), index.column());
    return QString::fromStdString(tooltip);
  } else if (role == Qt::TextAlignmentRole) {
    return static_cast<Qt::Alignment::Int>(text_alignment_);
  }

  return QVariant();
}

void OrbitTableModel::sort(int column, Qt::SortOrder order) {
  // On Linux, the arrows for ascending/descending are reversed, e.g. ascending
  // has an arrow pointing down, which is unexpected. This is because of some
  // Linux UI guideline, it is not the case on Windows.
  data_view_->OnSort(column, order == Qt::AscendingOrder ? DataView::SortingOrder::kAscending
                                                         : DataView::SortingOrder::kDescending);
}

std::pair<int, Qt::SortOrder> OrbitTableModel::GetDefaultSortingColumnAndOrder() {
  if (!IsSortingAllowed()) {
    return std::make_pair(-1, Qt::AscendingOrder);
  }

  int column = data_view_->GetDefaultSortingColumn();
  Qt::SortOrder order =
      data_view_->GetColumns()[column].initial_order == DataView::SortingOrder::kAscending
          ? Qt::AscendingOrder
          : Qt::DescendingOrder;
  return std::make_pair(column, order);
}

void OrbitTableModel::OnTimer() { data_view_->OnTimer(); }

void OrbitTableModel::OnFilter(const QString& filter) {
  data_view_->OnFilter(filter.toStdString());
}

void OrbitTableModel::OnRowSelected(int row) {
  if (static_cast<int>(data_view_->GetNumElements()) > row) {
    data_view_->OnSelect(row);
  }
}
