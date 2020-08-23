// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTSDATAVIEW_H
#define ORBIT_TRACEPOINTSDATAVIEW_H

#include "DataView.h"
#include "TracepointData.h"
#include "tracepoint.pb.h"

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
  // void OnDataChanged() override;
  void SetTracepoints(const std::vector<TracepointData*>& tracepoints);

 protected:
  void DoSort() override;
  void DoFilter() override;

  std::vector<std::string> m_FilterTokens;
  std::vector<TracepointData*> tracepoints_;

  enum ColumnIndex { kColumnSelected, kColumnCategory, kColumnName, kNumColumns };

  const TracepointData* GetTracepoint(uint32_t row) const;
};

#endif  // ORBIT_TRACEPOINTSDATAVIEW_H