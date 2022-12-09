// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_DATA_VIEW_PANEL_H_
#define ORBIT_QT_ORBIT_DATA_VIEW_PANEL_H_

#include <QFlags>
#include <QLineEdit>
#include <QObject>
#include <QString>
#include <QWidget>
#include <Qt>

#include "DataViews/DataView.h"
#include "OrbitQt/orbittreeview.h"
#include "OrbitQt/types.h"

namespace Ui {
class OrbitDataViewPanel;
}

class OrbitDataViewPanel : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitDataViewPanel(QWidget* parent = nullptr);
  ~OrbitDataViewPanel() override;

  void Initialize(orbit_data_views::DataView* data_view, SelectionType selection_type,
                  FontType font_type, bool uniform_row_height = true,
                  QFlags<Qt::AlignmentFlag> text_alignment = Qt::AlignVCenter | Qt::AlignLeft);
  void Deinitialize();
  void Link(OrbitDataViewPanel* panel);
  void Refresh();
  void SetDataModel(orbit_data_views::DataView* model);
  void SetFilter(const QString& filter);
  OrbitTreeView* GetTreeView();
  QLineEdit* GetFilterLineEdit();

 private:
  void OnFilterLineEditTextChanged(const QString& text);
  void OnRefreshButtonClicked();

  Ui::OrbitDataViewPanel* ui_;
};

#endif  // ORBIT_QT_ORBIT_DATA_VIEW_PANEL_H_
