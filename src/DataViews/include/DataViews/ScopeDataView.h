// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_SCOPE_DATA_VIEW_H_
#define DATA_VIEWS_SCOPE_DATA_VIEW_H_

#include <cstdint>

#include "DataViews/DataView.h"

namespace orbit_data_views {

class ScopeDataView : public DataView {
 public:
  ScopeDataView(DataViewType type, AppInterface* app,
                orbit_metrics_uploader::MetricsUploader* metrics_uploader)
      : DataView(type, app, metrics_uploader) {}

 protected:
  [[nodiscard]] std::optional<uint64_t> GetScopeIdFromRow(uint32_t row) const override {
    return GetScopeId(row);
  }

  [[nodiscard]] uint64_t GetScopeId(uint32_t row) const {
    ORBIT_CHECK(row < indices_.size());
    return indices_[row];
  }
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_SCOPE_DATA_VIEW_H_