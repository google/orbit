// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_CONFIG_VALIDATOR_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_CONFIG_VALIDATOR_

#include <QMessageBox>
#include <QObject>
#include <QString>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Result.h"

namespace orbit_mizar_widgets {

// Implements `Validate` method that checks if a pair of `HalfOfSamplingWithFrameTrackReportConfig`
// is malformed. That is, it checks if at least one thread is chosen for each of the configs and if
// the `start_relative_ns` does not exceed the total duration capture.
template <typename BaselineAndComparison, typename PairedData>
class SamplingWithFrameTrackReportConfigValidatorTmpl {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;

  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;

  using HalfConfig = ::orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig;

 public:
  explicit SamplingWithFrameTrackReportConfigValidatorTmpl(
      const Baseline<QString>& baseline_title, const Comparison<QString>& comparison_title)
      : baseline_title_(baseline_title), comparison_title_(comparison_title) {}

  [[nodiscard]] ErrorMessageOr<void> Validate(
      const BaselineAndComparison* baseline_and_comparison,
      const Baseline<HalfConfig>& baseline_config,
      const Comparison<HalfConfig>& comparison_config) const {
    OUTCOME_TRY(*LiftAndApply(&ValidateConfig, baseline_config,
                              baseline_and_comparison->GetBaselineData(), baseline_title_));

    OUTCOME_TRY(*LiftAndApply(&ValidateConfig, comparison_config,
                              baseline_and_comparison->GetComparisonData(), comparison_title_));

    return outcome::success();
  }

 private:
  static ErrorMessageOr<void> ValidateConfig(const HalfConfig& config, const PairedData& data,
                                             const QString& title) {
    const std::string title_std_string = title.toStdString();
    if (config.tids.empty()) {
      return ErrorMessage{title_std_string + ": No threads selected"};
    }
    if (config.start_relative_ns > data.CaptureDurationNs()) {
      return ErrorMessage{title_std_string + ": Start > capture duration"};
    }
    return outcome::success();
  }

  const BaselineAndComparison* baseline_and_comparison_;
  const Baseline<QString>& baseline_title_;
  const Comparison<QString>& comparison_title_;
};

using SamplingWithFrameTrackReportConfigValidator =
    SamplingWithFrameTrackReportConfigValidatorTmpl<orbit_mizar_data::BaselineAndComparison,
                                                    orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_CONFIG_VALIDATOR_
