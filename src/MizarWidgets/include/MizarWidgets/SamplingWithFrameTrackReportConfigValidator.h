// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_CONFIG_VALIDATOR_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_CONFIG_VALIDATOR_

#include <absl/functional/bind_front.h>

#include <QMessageBox>
#include <QString>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Result.h"

namespace orbit_mizar_widgets {

// Checks if the  pair of `HalfOfSamplingWithFrameTrackReportConfig` is malformed. If so, reports an
// error
template <typename BaselineAndComparison, typename PairedData, auto ReportError>
class SamplingWithFrameTrackReportConfigValidatorTmpl {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;

  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;

  using HalfConfig = orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig;

 public:
  explicit SamplingWithFrameTrackReportConfigValidatorTmpl(
      const Baseline<QString>& baseline_title, const Comparison<QString>& comparison_title)
      : baseline_title_(baseline_title), comparison_title_(comparison_title) {}

  [[nodiscard]] bool Validate(const BaselineAndComparison* baseline_and_comparison,
                              const Baseline<HalfConfig>& baseline_config,
                              const Comparison<HalfConfig>& comparison_config) const {
    const auto validator =
        absl::bind_front(&SamplingWithFrameTrackReportConfigValidatorTmpl::ValidateAndReport, this);

    const Baseline<bool> baseline_ok = LiftAndApply(
        validator, baseline_config, baseline_and_comparison->GetBaselineData(), baseline_title_);

    const Comparison<bool> comparison_ok =
        LiftAndApply(validator, comparison_config, baseline_and_comparison->GetComparisonData(),
                     comparison_title_);

    return *baseline_ok && *comparison_ok;
  }

 private:
  bool ValidateAndReport(const HalfConfig& config, const PairedData& data,
                         const QString& title) const {
    ErrorMessageOr<void> validation_result = ValidateConfig(config, data);
    if (validation_result.has_error()) {
      ReportError(title + ": " + QString::fromStdString(validation_result.error().message()));
      return false;
    }
    return true;
  }

  static ErrorMessageOr<void> ValidateConfig(const HalfConfig& config, const PairedData& data) {
    if (config.tids.empty()) {
      return ErrorMessage{"No threads selected"};
    }
    if (config.start_relative_ns > data.CaptureDuration()) {
      return ErrorMessage{"Start > capture duration"};
    }
    return outcome::success();
  }

  const BaselineAndComparison* baseline_and_comparison_;
  const Baseline<QString>& baseline_title_;
  const Comparison<QString>& comparison_title_;
};

inline void QtErrorReporter(const QString& message) {
  QMessageBox::critical(nullptr, "Invalid input", message);
}

using SamplingWithFrameTrackReportConfigValidator = SamplingWithFrameTrackReportConfigValidatorTmpl<
    orbit_mizar_data::BaselineAndComparison, orbit_mizar_data::MizarPairedData, QtErrorReporter>;

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_CONFIG_VALIDATOR_
