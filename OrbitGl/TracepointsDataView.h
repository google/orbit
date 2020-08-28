// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTSDATAVIEW_H
#define ORBIT_TRACEPOINTSDATAVIEW_H

#include <deque>
#include <string>
#include <vector>

#include "DataView.h"
#include "tracepoint.pb.h"

using orbit_grpc_protos::TracepointInfo;

class TracepointsDataView : public DataView {
 public:
  TracepointsDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnCategory; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;

  void SetTracepoints(const std::vector<TracepointInfo>& tracepoints);

 private:
  void DoSort() override;
  void DoFilter() override;

  std::deque<std::string> m_FilterTokens;
  std::deque<TracepointInfo> tracepoints_;

  enum ColumnIndex { kColumnSelected, kColumnCategory, kColumnName, kNumColumns };

  const TracepointInfo& GetTracepoint(uint32_t row) const;
};

#endif  // ORBIT_TRACEPOINTSDATAVIEW_H