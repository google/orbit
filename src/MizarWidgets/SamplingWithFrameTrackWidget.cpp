// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackWidget.h"

#include <QObject>
#include <QWidget>
#include <memory>

#include "MizarData/BaselineAndComparison.h"
#include "OrbitBase/Typedef.h"
#include "ui_SamplingWithFrameTrackWidget.h"

namespace orbit_mizar_widgets {

using orbit_base::LiftAndApply;
using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;

SamplingWithFrameTrackWidget::SamplingWithFrameTrackWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackWidget>()) {
  ui_->setupUi(this);

  OnMultiplicityCorrectionCheckBoxClicked(Qt::CheckState::Checked);

  QObject::connect(ui_->multiplicity_correction_, &QCheckBox::clicked, this,
                   &SamplingWithFrameTrackWidget::OnMultiplicityCorrectionCheckBoxClicked);
  QObject::connect(ui_->significance_level_, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &SamplingWithFrameTrackWidget::OnSignificanceLevelSelected);
}

void SamplingWithFrameTrackWidget::Init(
    const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison) {
  LiftAndApply(&SamplingWithFrameTrackInputWidget::Init, GetBaselineInput(),
               baseline_and_comparison->GetBaselineData(), kBaselineTitle);
  LiftAndApply(&SamplingWithFrameTrackInputWidget::Init, GetComparisonInput(),
               baseline_and_comparison->GetComparisonData(), kComparisonTitle);
}

SamplingWithFrameTrackWidget::~SamplingWithFrameTrackWidget() = default;

Baseline<SamplingWithFrameTrackInputWidget*> SamplingWithFrameTrackWidget::GetBaselineInput()
    const {
  return Baseline<SamplingWithFrameTrackInputWidget*>(ui_->baseline_input_);
}

Comparison<SamplingWithFrameTrackInputWidget*> SamplingWithFrameTrackWidget::GetComparisonInput()
    const {
  return Comparison<SamplingWithFrameTrackInputWidget*>(ui_->comparison_input_);
}

void SamplingWithFrameTrackWidget::OnSignificanceLevelSelected(int index) {
  constexpr int kIndexOfFivePercent = 0;
  significance_level_ =
      (index == kIndexOfFivePercent) ? kDefaultSignificanceLevel : kAlternativeSignificanceLevel;
}

void SamplingWithFrameTrackWidget::OnMultiplicityCorrectionCheckBoxClicked(bool checked) {
  is_multiplicity_correction_enabled_ = checked;

  const QString text = is_multiplicity_correction_enabled_ ? kMultiplicityCorrectionEnabledLabel
                                                           : kMultiplicityCorrectionDisabledLabel;
  ui_->significance_level_label_->setText(text);
}

}  // namespace orbit_mizar_widgets