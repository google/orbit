// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "processlauncherwidget.h"

#include "orbitdataviewpanel.h"
#include "types.h"
#include "ui_processlauncherwidget.h"

ProcessLauncherWidget::ProcessLauncherWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::ProcessLauncherWidget) {
  ui->setupUi(this);
}

ProcessLauncherWidget::~ProcessLauncherWidget() { delete ui; }

void ProcessLauncherWidget::SetDataView(DataView* data_view) {
  ui->LiveProcessList->Initialize(data_view, SelectionType::kDefault, FontType::kDefault);
}

void ProcessLauncherWidget::ClearDataView() { ui->LiveProcessList->Deinitialize(); }

void ProcessLauncherWidget::Refresh() { ui->LiveProcessList->Refresh(); }
