// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/FrametrackDataView.h"

#include <vector>

namespace orbit_data_views {

void FrametrackDataView::OnEnableFrameTrackRequested(const std::vector<int>& selection) {
  metrics_uploader_->SendLogEvent(
      orbit_metrics_uploader::OrbitLogEvent::ORBIT_FRAME_TRACK_ENABLE_CLICKED);

  for (int i : selection) {
    if (!IsRowFunction(i)) continue;

    const orbit_client_data::FunctionInfo* function = GetFunctionInfoFromRow(i);
    ORBIT_CHECK(function != nullptr);
    // Functions used as frame tracks must be hooked (selected), otherwise the
    // data to produce the frame track will not be captured.
    // The condition is supposed to prevent "selecting" a function when a capture
    // is loaded with no connection to a process being established.
    if (GetActionStatus(kMenuActionSelect, i, {i}) == ActionStatus::kVisibleAndEnabled) {
      app_->SelectFunction(*function);
    }

    app_->EnableFrameTrack(*function);
    app_->AddFrameTrack(*function);
  }
}

void FrametrackDataView::OnDisableFrameTrackRequested(const std::vector<int>& selection) {
  metrics_uploader_->SendLogEvent(
      orbit_metrics_uploader::OrbitLogEvent::ORBIT_FRAME_TRACK_DISABLE_CLICKED);

  for (int i : selection) {
    if (!IsRowFunction(i)) continue;

    const orbit_client_data::FunctionInfo* function = GetFunctionInfoFromRow(i);
    ORBIT_CHECK(function != nullptr);

    // When we remove a frame track, we do not unhook (deselect) the function as
    // it may have been selected manually (not as part of adding a frame track).
    // However, disable the frame track, so it is not recreated on the next capture.
    app_->DisableFrameTrack(*function);
    app_->RemoveFrameTrack(*function);
  }
}

}  // namespace orbit_data_views