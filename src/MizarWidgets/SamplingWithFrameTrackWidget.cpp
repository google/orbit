// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackWidget.h"

#include <absl/functional/bind_front.h>

#include <QObject>
#include <QWidget>
#include <memory>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/Titles.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarWidgets/SamplingWithFrameTrackReportConfigValidator.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Typedef.h"
#include "ui_SamplingWithFrameTrackWidget.h"

namespace orbit_mizar_widgets {

using ::orbit_base::LiftAndApply;
using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;
using ::orbit_mizar_base::kBaselineTitle;
using ::orbit_mizar_base::kComparisonTitle;

SamplingWithFrameTrackWidget::SamplingWithFrameTrackWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackWidget>()) {
  ui_->setupUi(this);

  OnMultiplicityCorrectionCheckBoxClicked(/*checked = */ true);

  QObject::connect(ui_->multiplicity_correction_, &QCheckBox::clicked, this,
                   &SamplingWithFrameTrackWidget::OnMultiplicityCorrectionCheckBoxClicked);
  QObject::connect(ui_->multiplicity_correction_, &QCheckBox::clicked, ui_->output_,
                   &SamplingWithFrameTrackOutputWidget::SetMultiplicityCorrectionEnabled);

  QObject::connect(ui_->significance_level_, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &SamplingWithFrameTrackWidget::OnSignificanceLevelSelected);
  QObject::connect(ui_->update_button_, &QPushButton::clicked, this,
                   &SamplingWithFrameTrackWidget::OnUpdateButtonClicked);

  ui_->output_->SetMultiplicityCorrectionEnabled(true);
  ui_->output_->OnSignificanceLevelChanged(kDefaultSignificanceLevel);
}

void SamplingWithFrameTrackWidget::Init(
    const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison,
    const Baseline<QString>& baseline_file_name, const Comparison<QString>& comparison_file_name) {
  const auto validator = absl::bind_front(&SamplingWithFrameTrackWidget::IsDataValid, this);
  Baseline<bool> is_baseline_valid =
      LiftAndApply(validator, baseline_and_comparison->GetBaselineData(), kBaselineTitle);
  Comparison<bool> is_comparison_valid =
      LiftAndApply(validator, baseline_and_comparison->GetComparisonData(), kComparisonTitle);
  if (!(*is_baseline_valid && *is_comparison_valid)) {
    ui_->update_button_->setEnabled(false);
  }

  LiftAndApply(&SamplingWithFrameTrackInputWidget::Init, GetBaselineInput(),
               baseline_and_comparison->GetBaselineData(), orbit_mizar_base::QBaselineTitle(),
               baseline_file_name);
  LiftAndApply(&SamplingWithFrameTrackInputWidget::Init, GetComparisonInput(),
               baseline_and_comparison->GetComparisonData(), orbit_mizar_base::QComparisonTitle(),
               comparison_file_name);
  baseline_and_comparison_ = baseline_and_comparison;
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
  const double significance_level =
      (index == kIndexOfFivePercent) ? kDefaultSignificanceLevel : kAlternativeSignificanceLevel;
  ui_->output_->OnSignificanceLevelChanged(significance_level);
}

void SamplingWithFrameTrackWidget::OnMultiplicityCorrectionCheckBoxClicked(bool checked) {
  const QString text =
      checked ? kMultiplicityCorrectionEnabledLabel : kMultiplicityCorrectionDisabledLabel;
  ui_->significance_level_label_->setText(text);
}

void SamplingWithFrameTrackWidget::OnUpdateButtonClicked() {
  Baseline<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig> baseline_config =
      LiftAndApply(&SamplingWithFrameTrackInputWidget::MakeConfig, GetBaselineInput());

  Comparison<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig> comparison_config =
      LiftAndApply(&SamplingWithFrameTrackInputWidget::MakeConfig, GetComparisonInput());

  ErrorMessageOr<void> validation_result =
      kConfigValidator.Validate(baseline_and_comparison_, baseline_config, comparison_config);
  if (validation_result.has_error()) {
    emit ReportError(validation_result.error().message());
    return;
  }

  Report report = baseline_and_comparison_->MakeSamplingWithFrameTrackReport(baseline_config,
                                                                             comparison_config);
  ui_->output_->UpdateReport(std::move(report));
}

bool SamplingWithFrameTrackWidget::IsDataValid(const orbit_mizar_data::MizarPairedData& data,
                                               std::string_view data_title) const {
  const bool is_valid = !data.GetFrameTracks().empty();
  if (!is_valid) {
    emit ReportError(absl::StrCat(data_title,
                                  " has no frame tracks.\n"
                                  "Sampling with comparison is not possible."
                                  "A frame track may be either:\n"
                                  "ETW events,\n"
                                  "Dynamically instrumented function or\n"
                                  "Manually (synchronous) instrumented scope"));
  }
  return is_valid;
}

}  // namespace orbit_mizar_widgets