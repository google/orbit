// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_FRAMETRACK_DATA_VIEW_H_
#define DATA_VIEWS_FRAMETRACK_DATA_VIEW_H_

#include <cstdint>

#include "DataViews/DataView.h"

namespace orbit_data_views {

// The abstract class for all the DataView that can enable/disable frametracks
class FrametrackDataView : public DataView {
 public:
  FrametrackDataView(DataViewType type, AppInterface* app,
                     orbit_metrics_uploader::MetricsUploader* metrics_uploader)
      : DataView(type, app, metrics_uploader) {}

  void OnEnableFrameTrackRequested(const std::vector<int>& selection) override;

  void OnDisableFrameTrackRequested(const std::vector<int>& selection) override;

 protected:
  [[nodiscard]] virtual bool IsRowFunction(uint32_t row) const = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_FRAMETRACK_DATA_VIEW_H_