// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/MizarMainWindow.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <memory>
#include <string>

#include "ui_MizarMainWindow.h"

namespace orbit_mizar_widgets {

MizarMainWindow::MizarMainWindow(
    const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison, QWidget* parent)
    : QMainWindow(parent), ui_(std::make_unique<Ui::MainWindow>()) {
  ui_->setupUi(this);
  ui_->sampling_with_frame_track_widget_->Init(baseline_and_comparison);

  QObject::connect(ui_->sampling_with_frame_track_widget_,
                   &SamplingWithFrameTrackWidget::ReportError, this,
                   [this](const std::string& message) {
                     QMessageBox::critical(this, "Invalid input", QString::fromStdString(message));
                   });
}

MizarMainWindow::~MizarMainWindow() = default;

}  // namespace orbit_mizar_widgets