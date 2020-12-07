// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_PROCESS_LAUNCHER_WIDGET_H
#define ORBIT_QT_PROCESS_LAUNCHER_WIDGET_H

#include <QObject>
#include <QString>
#include <QWidget>

#include "DataView.h"
#include "ui_processlauncherwidget.h"

namespace Ui {
class ProcessLauncherWidget;
}

class ProcessLauncherWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ProcessLauncherWidget(QWidget* parent = nullptr);
  ~ProcessLauncherWidget() override;

  void Refresh();
  void SetDataView(DataView* data_view);

 private:
  Ui::ProcessLauncherWidget* ui;
};

#endif  // ORBIT_QT_PROCESS_LAUNCHER_WIDGET_H
