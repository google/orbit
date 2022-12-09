// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_TREE_VIEW_H_
#define ORBIT_QT_ORBIT_TREE_VIEW_H_

#include <QFlags>
#include <QItemSelection>
#include <QKeyEvent>
#include <QModelIndex>
#include <QMouseEvent>
#include <QObject>
#include <QPoint>
#include <QResizeEvent>
#include <QString>
#include <QTimer>
#include <QTreeView>
#include <QWidget>
#include <Qt>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "DataViews/DataView.h"
#include "OrbitQt/orbittablemodel.h"
#include "OrbitQt/types.h"

class OrbitTreeView : public QTreeView {
  Q_OBJECT
 public:
  explicit OrbitTreeView(QWidget* parent = nullptr);
  void Initialize(orbit_data_views::DataView* data_view, SelectionType selection_type,
                  FontType font_type, bool uniform_row_height = true,
                  QFlags<Qt::AlignmentFlag> text_alignment = Qt::AlignVCenter | Qt::AlignLeft);
  void Deinitialize();
  void SetDataModel(orbit_data_views::DataView* data_view);
  void ClearDataModel();
  void OnFilter(const QString& filter);
  void Refresh(RefreshMode refresh_mode = RefreshMode::kOther);
  void Link(OrbitTreeView* link);
  void resizeEvent(QResizeEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
  OrbitTableModel* GetModel() { return model_.get(); }
  std::string GetLabel();
  [[nodiscard]] bool HasRefreshButton() const;
  void OnRefreshButtonClicked();
  void SetIsInternalRefresh(bool status) { is_internal_refresh_ = status; }
  void SetIsMultiSelection(bool status) { is_multi_selection_ = status; }

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 public slots:  // NOLINT(readability-redundant-access-specifiers)
  void columnResized(int column, int oldSize, int newSize);

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 private slots:
  void OnSort(int section, Qt::SortOrder order);
  void OnTimer();
  void ShowContextMenu(const QPoint& pos);
  void OnMenuClicked(std::string_view action, int menu_index);
  void OnRangeChanged(int min, int max);
  void OnDoubleClicked(QModelIndex index);
  void OnRowsSelected(std::vector<int>& rows);

  // TODO(https://github.com/google/orbit/issues/4589): Remove redundant "private" once slots is not
  // needed anymore above.
 private:  // NOLINT(readability-redundant-access-specifiers)
  std::unique_ptr<OrbitTableModel> model_;
  std::unique_ptr<QTimer> timer_;
  std::vector<OrbitTreeView*> links_;
  std::vector<float> column_ratios_;
  bool maintain_user_column_ratios_ = false;
  bool is_internal_refresh_ = false;
  bool is_multi_selection_ = false;
};

#endif  // ORBIT_QT_ORBIT_TREE_VIEW_H_
