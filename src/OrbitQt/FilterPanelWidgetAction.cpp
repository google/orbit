// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FilterPanelWidgetAction.h"

#include <QWidget>

#include "FilterPanelWidget.h"

FilterPanelWidgetAction::FilterPanelWidgetAction(QWidget* parent) : QWidgetAction(parent) {}

QWidget* FilterPanelWidgetAction::createWidget(QWidget* parent) {
  filter_panel_ = new FilterPanelWidget(parent);
  connect(filter_panel_, &FilterPanelWidget::FilterTracksTextChanged, this,
          &FilterPanelWidgetAction::FilterTracksTextChanged);
  connect(filter_panel_, &FilterPanelWidget::FilterFunctionsTextChanged, this,
          &FilterPanelWidgetAction::FilterFunctionsTextChanged);

  // Directly call `FilterPanelWidget::SetTimerLabelText` from `OrbitMainWindow::OnTimer()` causes
  // the access violation when the toolbar layout changes and the filter panel appears on the
  // capture toolbar again. For the thread-safety consideration, using signal/slot system instead of
  // calling functions for the QWidgetAction.
  connect(this, &FilterPanelWidgetAction::SetTimerLabelText, filter_panel_,
          &FilterPanelWidget::SetTimerLabelText);
  connect(this, &FilterPanelWidgetAction::SetFilterFunctionsText, filter_panel_,
          &FilterPanelWidget::SetFilterFunctionsText);
  return filter_panel_;
}

void FilterPanelWidgetAction::ClearEdits() { filter_panel_->ClearEdits(); }
