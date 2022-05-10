// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_SCOPE_DATA_VIEW_H_
#define DATA_VIEWS_SCOPE_DATA_VIEW_H_

#include <cstdint>

#include "DataViews/DataView.h"

namespace orbit_data_views {

// The abstract class for all the DataView to that deal with scopes to inherit from.
class ScopeDataView : public DataView {
 public:
  ScopeDataView(DataViewType type, AppInterface* app,
                orbit_metrics_uploader::MetricsUploader* metrics_uploader)
      : DataView(type, app, metrics_uploader) {}

  void OnEnableFrameTrackRequested(const std::vector<int>& selection) override;

  void OnDisableFrameTrackRequested(const std::vector<int>& selection) override;

 protected:
  [[nodiscard]] uint64_t GetScopeId(uint32_t row) const;

  [[nodiscard]] bool IsScopeDynamicallyInstrumentedFunction(uint64_t scope_id) const;

  [[nodiscard]] const orbit_client_data::ScopeInfo& GetScopeInfo(uint64_t scope_id) const;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_SCOPE_DATA_VIEW_H_