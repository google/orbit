// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_TREE_VIEW_H_
#define ORBIT_QT_ORBIT_TREE_VIEW_H_

#include <QItemSelection>
#include <QTimer>
#include <QTreeView>
#include <memory>

#include "orbitglwidget.h"
#include "orbittablemodel.h"
#include "types.h"

class OrbitTreeView : public QTreeView {
  Q_OBJECT
 public:
  explicit OrbitTreeView(QWidget* parent = nullptr);
  void Initialize(DataView* data_view, SelectionType selection_type, FontType font_type,
                  bool uniform_row_height = true,
                  QFlags<Qt::AlignmentFlag> text_alignment = Qt::AlignVCenter | Qt::AlignLeft);
  void Deinitialize();
  void SetDataModel(DataView* model);
  void ClearDataModel();
  void OnFilter(const QString& filter);
  void Refresh();
  void Link(OrbitTreeView* link);
  void SetGlWidget(OrbitGLWidget* gl_widget);
  void resizeEvent(QResizeEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
  OrbitTableModel* GetModel() { return model_.get(); }
  std::string GetLabel();
  bool HasRefreshButton() const;
  void OnRefreshButtonClicked();
  void SetIsInternalRefresh(bool status) { is_internal_refresh_ = status; }

 public slots:
  void columnResized(int column, int oldSize, int newSize);

 private slots:
  void OnSort(int section, Qt::SortOrder order);
  void OnTimer();
  void ShowContextMenu(const QPoint& pos);
  void OnMenuClicked(const std::string& action, int menu_index);
  void OnRangeChanged(int min, int max);
  void OnRowSelected(std::optional<int> row);

 private:
  std::unique_ptr<OrbitTableModel> model_;
  std::unique_ptr<QTimer> timer_;
  std::vector<OrbitTreeView*> links_;
  bool auto_resize_;
  bool is_internal_refresh_ = false;
};

#endif  // ORBIT_QT_ORBIT_TREE_VIEW_H_
