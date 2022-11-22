// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_OUTPUT_WIDGET_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_OUTPUT_WIDGET_H_

#include <QObject>
#include <QResizeEvent>
#include <QString>
#include <QWidget>
#include <memory>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarModels/SamplingWithFrameTrackReportModel.h"

namespace Ui {
class SamplingWithFrameTrackOutputWidget;
}

namespace orbit_mizar_widgets {

// The widget handles visualization of the comparison report based on sampling data with frame
// track.
class SamplingWithFrameTrackOutputWidget : public QWidget {
  Q_OBJECT
  using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using SamplingWithFrameTrackReportModel = ::orbit_mizar_models::SamplingWithFrameTrackReportModel;
  using FunctionNameToShow =
      ::orbit_mizar_models::SamplingWithFrameTrackReportModel::FunctionNameToShow;

 public:
  explicit SamplingWithFrameTrackOutputWidget(QWidget* parent = nullptr);
  ~SamplingWithFrameTrackOutputWidget();
  void UpdateReport(Report report);

 public slots:
  void SetMultiplicityCorrectionEnabled(bool checked);
  void OnSignificanceLevelChanged(double significance_level);
  void SetFunctionNameToShow(FunctionNameToShow function_name_to_show);

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void ResizeReportColumns(int width) const;

  SamplingWithFrameTrackReportModel* model_{};
  bool is_multiplicity_correction_enabled_ = true;
  double confidence_level_ = 0.05;
  FunctionNameToShow function_name_to_show_ = FunctionNameToShow::kBaseline;
  std::unique_ptr<Ui::SamplingWithFrameTrackOutputWidget> ui_;
};

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_OUTPUT_WIDGET_H_