// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackOutputWidget.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QObject>
#include <QResizeEvent>
#include <QSize>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>
#include <Qt>
#include <memory>
#include <utility>

#include "ui_SamplingWithFrameTrackOutputWidget.h"

namespace orbit_mizar_widgets {

template <typename T>
using Baseline = ::orbit_mizar_base::Baseline<T>;
template <typename T>
using Comparison = ::orbit_mizar_base::Comparison<T>;

SamplingWithFrameTrackOutputWidget::SamplingWithFrameTrackOutputWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackOutputWidget>()) {
  ui_->setupUi(this);
}

SamplingWithFrameTrackOutputWidget::~SamplingWithFrameTrackOutputWidget() = default;

void SamplingWithFrameTrackOutputWidget::UpdateReport(Report report) {
  model_ = new SamplingWithFrameTrackReportModel(  // NOLINT
      std::move(report), is_multiplicity_correction_enabled_, confidence_level_,
      function_name_to_show_, this);

  auto* proxy_model = new QSortFilterProxyModel(this);  // NOLINT
  proxy_model->setSourceModel(model_);
  proxy_model->setSortRole(Qt::EditRole);
  proxy_model->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxy_model->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxy_model->setFilterKeyColumn(
      static_cast<int>(SamplingWithFrameTrackReportModel::Column::kFunctionName));

  QObject::connect(ui_->filter_line_, &QLineEdit::textChanged, proxy_model,
                   &QSortFilterProxyModel::setFilterFixedString);

  ui_->report_->setModel(proxy_model);
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

void SamplingWithFrameTrackOutputWidget::SetFunctionNameToShow(
    FunctionNameToShow function_name_to_show) {
  function_name_to_show_ = function_name_to_show;
  if (model_ != nullptr) {
    model_->SetFunctionNameToShow(function_name_to_show);
  }
}

}  // namespace orbit_mizar_widgets