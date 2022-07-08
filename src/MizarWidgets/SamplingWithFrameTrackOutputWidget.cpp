// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackOutputWidget.h"

#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <limits>
#include <memory>

#include "MizarModels/SamplingWithFrameTrackReportModel.h"
#include "ui_SamplingWithFrameTrackOutputWidget.h"

namespace orbit_mizar_widgets {

SamplingWithFrameTrackOutputWidget::SamplingWithFrameTrackOutputWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackOutputWidget>()) {
  ui_->setupUi(this);
}

void SamplingWithFrameTrackOutputWidget::UpdateReport(Report report) {
  auto model = std::make_unique<SamplingWithFrameTrackReportModel>(std::move(report), this);

  ui_->report_->setModel(model.release());
  ResizeReportColumns(width());
}

void SamplingWithFrameTrackOutputWidget::resizeEvent(QResizeEvent* event) {
  ResizeReportColumns(event->size().width());
}

void SamplingWithFrameTrackOutputWidget::ResizeReportColumns(int width) const {
  ui_->report_->horizontalHeader()->setMaximumSectionSize(width / 3);
  ui_->report_->resizeColumnsToContents();
  ui_->report_->horizontalHeader()->setMaximumSectionSize(width);
}

}  // namespace orbit_mizar_widgets