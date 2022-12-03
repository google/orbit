// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/MizarMainWindow.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <memory>
#include <string_view>

#include "MizarWidgets/SamplingWithFrameTrackWidget.h"
#include "ui_MizarMainWindow.h"

namespace orbit_mizar_widgets {

MizarMainWindow::MizarMainWindow(
    const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison,
    const Baseline<QString>& baseline_file_name, const Comparison<QString>& comparison_file_name,
    QWidget* parent)
    : QMainWindow(parent), ui_(std::make_unique<Ui::MainWindow>()) {
  ui_->setupUi(this);
  QObject::connect(ui_->sampling_with_frame_track_widget_,
                   &SamplingWithFrameTrackWidget::ReportError, this,
                   [this](std::string_view message) {
                     QMessageBox::critical(this, "Invalid input",
                                           QString::fromUtf8(message.data(), message.size()));
                   });

  ui_->sampling_with_frame_track_widget_->Init(baseline_and_comparison, baseline_file_name,
                                               comparison_file_name);
}

MizarMainWindow::~MizarMainWindow() = default;

}  // namespace orbit_mizar_widgets