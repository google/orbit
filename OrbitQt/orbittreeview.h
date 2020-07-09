// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

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
  void Initialize(DataView* data_view, SelectionType selection_type,
                  FontType font_type);
  void SetDataModel(DataView* model);
  void OnFilter(const QString& a_Filter);
  void Refresh();
  void Link(OrbitTreeView* a_Link);
  void SetGlWidget(OrbitGLWidget* a_Link);
  void resizeEvent(QResizeEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void selectionChanged(const QItemSelection& selected,
                        const QItemSelection& deselected) override;
  OrbitTableModel* GetModel() { return model_.get(); }
  std::string GetLabel();

 protected:
  void drawRow(QPainter* painter, const QStyleOptionViewItem& options,
               const QModelIndex& index) const override;

 signals:

 public slots:
  void columnResized(int column, int oldSize, int newSize);

 private slots:
  void OnSort(int a_Section, Qt::SortOrder a_Order);
  void OnTimer();
  void ShowContextMenu(const QPoint& pos);
  void OnMenuClicked(const std::string& a_Action, int a_MenuIndex);
  void OnRangeChanged(int a_Min, int a_Max);
  void OnRowSelected(int row);

 private:
  std::unique_ptr<OrbitTableModel> model_;
  std::unique_ptr<QTimer> timer_;
  std::vector<OrbitTreeView*> links_;
  bool auto_resize_;
  bool is_internal_refresh = false;
};
