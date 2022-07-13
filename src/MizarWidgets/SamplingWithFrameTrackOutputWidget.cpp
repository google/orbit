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

SamplingWithFrameTrackOutputWidget::~SamplingWithFrameTrackOutputWidget() = default;

void SamplingWithFrameTrackOutputWidget::UpdateReport(Report report) {
  auto model = std::make_unique<SamplingWithFrameTrackReportModel>(
      std::move(report), is_multiplicity_correction_enabled_, confidence_level_, this);
  model_ = model.get();

  auto proxy_model = std::make_unique<QSortFilterProxyModel>(this);
  proxy_model->setSourceModel(model.release());
  proxy_model->setSortRole(SamplingWithFrameTrackReportModel::kSortRole);

  ui_->report_->setModel(proxy_model.release());
  ui_->report_->setSortingEnabled(true);
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

void SamplingWithFrameTrackOutputWidget::SetMultiplicityCorrectionEnabled(bool checked) {
  is_multiplicity_correction_enabled_ = checked;
  if (model_ != nullptr) {
    model_->SetMultiplicityCorrectionEnabled(is_multiplicity_correction_enabled_);
  }
}

void SamplingWithFrameTrackOutputWidget::OnSignificanceLevelChanged(double significance_level) {
  confidence_level_ = significance_level;
  if (model_ != nullptr) {
    model_->SetSignificanceLevel(significance_level);
  }
}

}  // namespace orbit_mizar_widgets