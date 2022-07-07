// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_OUTPUT_WIDGET_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_OUTPUT_WIDGET_H_

#include <QWidget>

#include "MizarData/SamplingWithFrameTrackComparisonReport.h"

namespace Ui {
class SamplingWithFrameTrackOutputWidget;
}

namespace orbit_mizar_widgets {

class SamplingWithFrameTrackOutputWidget : public QWidget {
  using Report = ::orbit_mizar_data::SamplingWithFrameTrackComparisonReport;

 public:
  explicit SamplingWithFrameTrackOutputWidget(QWidget* parent = nullptr);
  void UpdateReport(Report report);

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void ResizeReportColumns(int width) const;

  std::unique_ptr<Ui::SamplingWithFrameTrackOutputWidget> ui_;
};

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_OUTPUT_WIDGET_H_