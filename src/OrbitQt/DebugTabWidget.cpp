// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DebugTabWidget.h"

#include <QObject>
#include <QWidget>
#include <memory>

#include "CaptureWindowDebugWidget.h"
#include "TimeGraphLayout.h"
#include "ui_DebugTabWidget.h"

namespace orbit_qt {

constexpr int kCaptureWindowTabIndex = 0;
constexpr int kIntrospectionTabIndex = 1;

DebugTabWidget::DebugTabWidget(QWidget* parent)
    : QWidget{parent}, ui_{std::make_unique<Ui::DebugTabWidget>()} {
  ui_->setupUi(this);
  ui_->tabWidget->setTabEnabled(kCaptureWindowTabIndex, false);
  ui_->tabWidget->setTabEnabled(kIntrospectionTabIndex, false);

  QObject::connect(ui_->captureWindowDebugWidget,
                   &CaptureWindowDebugWidget::AnyLayoutPropertyChanged, this,
                   [this]() { emit AnyCaptureWindowPropertyChanged(); });
  QObject::connect(ui_->introspectionWindowDebugWidget,
                   &CaptureWindowDebugWidget::AnyLayoutPropertyChanged, this,
                   [this]() { emit AnyIntrospectionWindowPropertyChanged(); });
}

DebugTabWidget::~DebugTabWidget() = default;

void DebugTabWidget::SetCaptureWindowDebugInterface(
    const orbit_gl::CaptureWindowDebugInterface* interface) {
  ui_->captureWindowDebugWidget->SetCaptureWindowDebugInterface(interface);
  ui_->tabWidget->setTabEnabled(kCaptureWindowTabIndex, interface != nullptr);
}

void DebugTabWidget::ResetCaptureWindowDebugInterface() {
  ui_->captureWindowDebugWidget->ResetCaptureWindowDebugInterface();
  ui_->tabWidget->setTabEnabled(kCaptureWindowTabIndex, false);
}

void DebugTabWidget::SetIntrospectionWindowDebugInterface(
    const orbit_gl::CaptureWindowDebugInterface* interface) {
  ui_->introspectionWindowDebugWidget->SetCaptureWindowDebugInterface(interface);
  ui_->tabWidget->setTabEnabled(kIntrospectionTabIndex, interface != nullptr);
}

void DebugTabWidget::ResetIntrospectionWindowDebugInterface() {
  ui_->introspectionWindowDebugWidget->ResetCaptureWindowDebugInterface();
  ui_->tabWidget->setTabEnabled(kIntrospectionTabIndex, false);
}

TimeGraphLayout* DebugTabWidget::GetTimeGraphLayoutForTheCaptureWindow() const {
  return ui_->captureWindowDebugWidget->GetTimeGraphLayout();
}

TimeGraphLayout* DebugTabWidget::GetTimeGraphLayoutForTheIntrospectionWindow() const {
  return ui_->introspectionWindowDebugWidget->GetTimeGraphLayout();
}

}  // namespace orbit_qt
