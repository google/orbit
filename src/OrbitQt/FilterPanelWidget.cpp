// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/FilterPanelWidget.h"

#include <QFontMetrics>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include "ui_FilterPanelWidget.h"

FilterPanelWidget::FilterPanelWidget(QWidget* parent)
    : QFrame(parent), ui(new Ui::FilterPanelWidget) {
  ui->setupUi(this);

  connect(ui->filterTracks, &QLineEdit::textChanged, this,
          &FilterPanelWidget::FilterTracksTextChanged);
  connect(ui->filterFunctions, &QLineEdit::textChanged, this,
          &FilterPanelWidget::FilterFunctionsTextChanged);

  QFontMetrics fm(ui->timerLabel->font());
  int pixel_width = fm.horizontalAdvance("w");
  ui->timerLabel->setMinimumWidth(5 * pixel_width);
}

void FilterPanelWidget::SetFilterFunctionsText(const QString& text) {
  ui->filterFunctions->blockSignals(true);
  ui->filterFunctions->setText(text);
  ui->filterFunctions->blockSignals(false);
}

void FilterPanelWidget::SetTimerLabelText(const QString& text) { ui->timerLabel->setText(text); }

void FilterPanelWidget::ClearEdits() {
  ui->filterFunctions->clear();
  ui->filterTracks->clear();
}
