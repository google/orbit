// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackWidget.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_cat.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMetaType>
#include <QNonConstOverload>
#include <QObject>
#include <QPushButton>
#include <QVariant>
#include <QWidget>
#include <memory>
#include <utility>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/Titles.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarModels/SamplingWithFrameTrackReportModel.h"
#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"
#include "MizarWidgets/SamplingWithFrameTrackOutputWidget.h"
#include "MizarWidgets/SamplingWithFrameTrackReportConfigValidator.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Typedef.h"
#include "ui_SamplingWithFrameTrackWidget.h"

using ::orbit_base::LiftAndApply;
using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;
using ::orbit_mizar_base::kBaselineTitle;
using ::orbit_mizar_base::kComparisonTitle;
using FunctionNameToShow =
    ::orbit_mizar_models::SamplingWithFrameTrackReportModel::FunctionNameToShow;

Q_DECLARE_METATYPE(FunctionNameToShow);

namespace orbit_mizar_widgets {

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

  ui_->use_symbols_toggle_->setItemData(0, QVariant::fromValue(FunctionNameToShow::kBaseline));
  ui_->use_symbols_toggle_->setItemData(1, QVariant::fromValue(FunctionNameToShow::kComparison));

  QObject::connect(ui_->use_symbols_toggle_, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   [this](int /*index*/) { SetFunctionNameToShow(); });

  ui_->output_->SetMultiplicityCorrectionEnabled(true);
  ui_->output_->OnSignificanceLevelChanged(kDefaultSignificanceLevel);
  SetFunctionNameToShow();
}

void SamplingWithFrameTrackWidget::SetFunctionNameToShow() {
  ui_->output_->SetFunctionNameToShow(
      ui_->use_symbols_toggle_->currentData().value<FunctionNameToShow>());
}

void SamplingWithFrameTrackWidget::Init(
    const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison,
    const Baseline<QString>& baseline_file_name, const Comparison<QString>& comparison_file_name) {
  Baseline<ErrorMessageOr<void>> baseline_validation_outcome =
      LiftAndApply(&SamplingWithFrameTrackWidget::IsDataValid,
                   baseline_and_comparison->GetBaselineData(), kBaselineTitle);
  Comparison<ErrorMessageOr<void>> comparison_validation_outcome =
      LiftAndApply(&SamplingWithFrameTrackWidget::IsDataValid,
                   baseline_and_comparison->GetComparisonData(), kComparisonTitle);
  ProcessDataValidationOutcome(*baseline_validation_outcome);
  ProcessDataValidationOutcome(*comparison_validation_outcome);

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

ErrorMessageOr<void> SamplingWithFrameTrackWidget::IsDataValid(
    const orbit_mizar_data::MizarPairedData& data, std::string_view data_title) {
  const bool is_valid = !data.GetFrameTracks().empty();
  if (!is_valid) {
    return ErrorMessage(absl::StrCat(data_title,
                                     " has no frame tracks.\n"
                                     "Sampling with comparison is not possible."
                                     "A frame track may be either:\n"
                                     "ETW events,\n"
                                     "Dynamically instrumented function or\n"
                                     "Manually (synchronous) instrumented scope"));
  }
  return outcome::success();
}

void SamplingWithFrameTrackWidget::ProcessDataValidationOutcome(
    const ErrorMessageOr<void>& outcome) const {
  if (outcome.has_error()) {
    emit ReportError(outcome.error().message());
    ui_->update_button_->setEnabled(false);
  }
}

}  // namespace orbit_mizar_widgets