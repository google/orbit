// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_WIDGET_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_WIDGET_H_

#include <QObject>
#include <QStringLiteral>
#include <QWidget>
#include <memory>
#include <string_view>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarWidgets/MizarMainWindow.h"
#include "MizarWidgets/SamplingWithFrameTrackReportConfigValidator.h"
#include "SamplingWithFrameTrackInputWidget.h"

namespace Ui {
class SamplingWithFrameTrackWidget;
}

namespace orbit_mizar_widgets {

class SamplingWithFrameTrackWidget : public QWidget {
  Q_OBJECT

  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;

  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;

  using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;

 public:
  explicit SamplingWithFrameTrackWidget(QWidget* parent = nullptr);
  ~SamplingWithFrameTrackWidget() override;

  void Init(const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison);

 public slots:
  void OnMultiplicityCorrectionCheckBoxClicked(bool checked);
  void OnUpdateButtonClicked();

 signals:
  void ReportError(const std::string& message);

 private:
  [[nodiscard]] Baseline<SamplingWithFrameTrackInputWidget*> GetBaselineInput() const;
  [[nodiscard]] Comparison<SamplingWithFrameTrackInputWidget*> GetComparisonInput() const;
  [[nodiscard]] bool IsMultiplicityCorrectionEnabled() const;
  void OnSignificanceLevelSelected(int index);

  const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison_;
  bool is_multiplicity_correction_enabled_ = true;
  double significance_level_ = kDefaultSignificanceLevel;
  std::unique_ptr<Ui::SamplingWithFrameTrackWidget> ui_;

  static constexpr inline double kDefaultSignificanceLevel = 0.05;
  static constexpr inline double kAlternativeSignificanceLevel = 0.01;
  static const inline Baseline<QString> kBaselineTitle =
      Baseline<QString>(QStringLiteral("Baseline"));
  static const inline Comparison<QString> kComparisonTitle =
      Comparison<QString>(QStringLiteral("Comparison"));
  static const inline QString kMultiplicityCorrectionEnabledLabel =
      QStringLiteral("Probability of false-alarm for at least one function:");
  static const inline QString kMultiplicityCorrectionDisabledLabel =
      QStringLiteral("Probability of false-alarm for an individual function:");

  static const inline SamplingWithFrameTrackReportConfigValidator kConfigValidator{
      kBaselineTitle, kComparisonTitle};
};

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_WIDGET_H_
