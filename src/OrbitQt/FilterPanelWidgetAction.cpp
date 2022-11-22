// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/FilterPanelWidgetAction.h"

#include <QWidget>

#include "OrbitQt/FilterPanelWidget.h"

FilterPanelWidgetAction::FilterPanelWidgetAction(QWidget* parent) : QWidgetAction(parent) {}

QWidget* FilterPanelWidgetAction::createWidget(QWidget* parent) {
  filter_panel_ = new FilterPanelWidget(parent);

  // Directly calling methods on FilterPanelWidget (e.g. to set timer text or clear the text),
  // can cause access violations when the toolbar layout changes and the filter panel appears on the
  // capture toolbar again. We therefore use the signal/slot system instead of calling methods
  // directly on QWidgetAction.
  connect(filter_panel_, &FilterPanelWidget::FilterTracksTextChanged, this,
          &FilterPanelWidgetAction::FilterTracksTextChanged);
  connect(filter_panel_, &FilterPanelWidget::FilterFunctionsTextChanged, this,
          &FilterPanelWidgetAction::FilterFunctionsTextChanged);
  connect(this, &FilterPanelWidgetAction::SetTimerLabelText, filter_panel_,
          &FilterPanelWidget::SetTimerLabelText);
  connect(this, &FilterPanelWidgetAction::SetFilterFunctionsText, filter_panel_,
          &FilterPanelWidget::SetFilterFunctionsText);
  connect(this, &FilterPanelWidgetAction::ClearEdits, filter_panel_,
          &FilterPanelWidget::ClearEdits);
  return filter_panel_;
}