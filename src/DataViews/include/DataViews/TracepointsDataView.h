// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_TRACEPOINTS_DATA_VIEW_H_
#define DATA_VIEWS_TRACEPOINTS_DATA_VIEW_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "DataViews/AppInterface.h"
#include "DataViews/DataView.h"
#include "GrpcProtos/tracepoint.pb.h"

namespace orbit_data_views {

class TracepointsDataView : public DataView {
 public:
  explicit TracepointsDataView(AppInterface* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnCategory; }
  std::string GetValue(int row, int column) override;

  void OnSelectRequested(absl::Span<const int> selection) override;
  void OnUnselectRequested(absl::Span<const int> selection) override;

  void SetTracepoints(absl::Span<const orbit_grpc_protos::TracepointInfo> tracepoints);

 private:
  [[nodiscard]] ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                             absl::Span<const int> selected_indices) override;
  void DoSort() override;
  void DoFilter() override;

  std::deque<orbit_grpc_protos::TracepointInfo> tracepoints_;

  enum ColumnIndex { kColumnSelected, kColumnCategory, kColumnName, kNumColumns };

  [[nodiscard]] const orbit_grpc_protos::TracepointInfo& GetTracepoint(uint32_t row) const;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_TRACEPOINTS_DATA_VIEW_H_
