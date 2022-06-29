// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackWidget.h"

#include <QWidget>
#include <memory>

#include "MizarData/BaselineAndComparison.h"
#include "OrbitBase/Typedef.h"
#include "ui_SamplingWithFrameTrackWidget.h"

namespace orbit_mizar_widgets {

template <typename T>
using Baseline = ::orbit_mizar_base::Baseline<T>;

template <typename T>
using Comparison = ::orbit_mizar_base::Comparison<T>;

using orbit_base::LiftAndApply;

SamplingWithFrameTrackWidget::SamplingWithFrameTrackWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackWidget>()) {
  ui_->setupUi(this);
}

void SamplingWithFrameTrackWidget::Init(
    const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison) {
  LiftAndApply(&SamplingWithFrameTrackInputWidget::Init, GetBaselineInput(),
               baseline_and_comparison->GetBaselineData(), kBaselineTitle);
  LiftAndApply(&SamplingWithFrameTrackInputWidget::Init, GetComparisonInput(),
               baseline_and_comparison->GetComparisonData(), kComparisonTitle);
}

SamplingWithFrameTrackWidget::~SamplingWithFrameTrackWidget() = default;

[[nodiscard]] Baseline<SamplingWithFrameTrackInputWidget*>
SamplingWithFrameTrackWidget::GetBaselineInput() const {
  return Baseline<SamplingWithFrameTrackInputWidget*>(ui_->baseline_input_);
}

[[nodiscard]] Comparison<SamplingWithFrameTrackInputWidget*>
SamplingWithFrameTrackWidget::GetComparisonInput() const {
  return Comparison<SamplingWithFrameTrackInputWidget*>(ui_->comparison_input_);
}

const Baseline<QString> SamplingWithFrameTrackWidget::kBaselineTitle =
    Baseline<QString>(QStringLiteral("Baseline"));
const Comparison<QString> SamplingWithFrameTrackWidget::kComparisonTitle =
    Comparison<QString>(QStringLiteral("Comparison"));

}  // namespace orbit_mizar_widgets