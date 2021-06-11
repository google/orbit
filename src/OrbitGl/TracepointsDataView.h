// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACEPOINTS_DATA_VIEW_H_
#define ORBIT_GL_TRACEPOINTS_DATA_VIEW_H_

#include <stdint.h>

#include <deque>
#include <string>
#include <vector>

#include "DataView.h"
#include "tracepoint.pb.h"

class OrbitApp;

class TracepointsDataView : public DataView {
 public:
  explicit TracepointsDataView(OrbitApp* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnCategory; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;

  void SetTracepoints(const std::vector<orbit_grpc_protos::TracepointInfo>& tracepoints);

 private:
  void DoSort() override;
  void DoFilter() override;

  std::deque<std::string> m_FilterTokens;
  std::deque<orbit_grpc_protos::TracepointInfo> tracepoints_;

  enum ColumnIndex { kColumnSelected, kColumnCategory, kColumnName, kNumColumns };

  const orbit_grpc_protos::TracepointInfo& GetTracepoint(uint32_t row) const;

  // TODO(b/185090791): This is temporary and will be removed once this data view has been ported
  // and move to orbit_data_views.
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_TRACEPOINTS_DATA_VIEW_H_
