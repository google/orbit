// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/CaptureWindowDebugWidget.h"

#include <QPlainTextEdit>
#include <QWidget>
#include <Qt>
#include <chrono>
#include <memory>

#include "OrbitQt/TimeGraphLayoutWidget.h"
#include "ui_CaptureWindowDebugWidget.h"

namespace orbit_qt {

CaptureWindowDebugWidget::CaptureWindowDebugWidget(QWidget* parent)
    : QWidget{parent}, ui_{std::make_unique<Ui::CaptureWindowDebugWidget>()} {
  ui_->setupUi(this);

  QObject::connect(ui_->layoutPropertiesWidget,
                   &TimeGraphLayoutWidget::AnyRegisteredPropertyChangedValue, this,
                   [this]() { emit AnyLayoutPropertyChanged(); });
}

// This needs to be here to allow Ui::CaptureWindowDebugWidget to be incomplete in the header.
CaptureWindowDebugWidget::~CaptureWindowDebugWidget() = default;

void CaptureWindowDebugWidget::SetCaptureWindowDebugInterface(
    const orbit_gl::CaptureWindowDebugInterface* capture_window_debug_interface) {
  capture_window_debug_interface_ = capture_window_debug_interface;

  if (capture_window_debug_interface_ != nullptr) {
    constexpr std::chrono::milliseconds kDebugDataUpdateInterval{200};
    update_timer.start(kDebugDataUpdateInterval);
    UpdateUiElements();
  } else {
    update_timer.stop();
  }

  QObject::connect(&update_timer, &QTimer::timeout, this,
                   &CaptureWindowDebugWidget::UpdateUiElements, Qt::UniqueConnection);
}

void CaptureWindowDebugWidget::ResetCaptureWindowDebugInterface() {
  update_timer.stop();
  capture_window_debug_interface_ = nullptr;
}

TimeGraphLayout* CaptureWindowDebugWidget::GetTimeGraphLayout() const {
  return ui_->layoutPropertiesWidget;
}

void CaptureWindowDebugWidget::UpdateUiElements() {
  if (capture_window_debug_interface_ == nullptr) return;

  QString capture_info = QString::fromStdString(capture_window_debug_interface_->GetCaptureInfo());

  if (ui_->captureInfoTextEdit->toPlainText() != capture_info) {
    ui_->captureInfoTextEdit->setPlainText(capture_info);
  }

  QString performance_info =
      QString::fromStdString(capture_window_debug_interface_->GetPerformanceInfo());

  if (ui_->performanceTextEdit->toPlainText() != performance_info) {
    ui_->performanceTextEdit->setPlainText(performance_info);
  }

  QString selection_summary =
      QString::fromStdString(capture_window_debug_interface_->GetSelectionSummary());

  if (ui_->selectionSummaryTextEdit->toPlainText() != selection_summary) {
    ui_->selectionSummaryTextEdit->setPlainText(selection_summary);
  }
}
}  // namespace orbit_qt