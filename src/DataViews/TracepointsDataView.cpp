// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/TracepointsDataView.h"

#include <absl/strings/str_split.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>

#include "CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"
#include "OrbitBase/Append.h"

using orbit_grpc_protos::TracepointInfo;

namespace orbit_data_views {

TracepointsDataView::TracepointsDataView(AppInterface* app)
    : DataView(DataViewType::kTracepoints, app) {}

const std::vector<DataView::Column>& TracepointsDataView::GetColumns() {
  static const std::vector<Column>& columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Selected", .0f, SortingOrder::kDescending};
    columns[kColumnCategory] = {"Category", .5f, SortingOrder::kAscending};
    columns[kColumnName] = {"Name", .2f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string TracepointsDataView::GetValue(int row, int col) {
  const TracepointInfo& tracepoint = GetTracepoint(row);

  switch (col) {
    case kColumnSelected:
      return app_->IsTracepointSelected(tracepoint) ? "X" : "-";
    case kColumnName:
      return tracepoint.name();
    case kColumnCategory:
      return tracepoint.category();
    default:
      return "";
  }
}

#define ORBIT_PROC_SORT(Member)                                                         \
  [&](int a, int b) {                                                                   \
    return CompareAscendingOrDescending(tracepoints_[a].Member, tracepoints_[b].Member, \
                                        ascending);                                     \
  }

void TracepointsDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnName:
      sorter = ORBIT_PROC_SORT(name());
      break;
    case kColumnCategory:
      sorter = ORBIT_PROC_SORT(category());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

void TracepointsDataView::DoFilter() {
  std::vector<uint64_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (size_t i = 0; i < tracepoints_.size(); ++i) {
    const TracepointInfo& tracepoint = tracepoints_[i];
    const std::string& tracepoint_string = tracepoint.name();

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (tracepoint_string.find(filter_token) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.emplace_back(i);
    }
  }

  indices_ = std::move(indices);
}

std::vector<std::vector<std::string>> TracepointsDataView::GetContextMenuWithGrouping(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;
  for (int index : selected_indices) {
    const TracepointInfo& tracepoint = GetTracepoint(index);
    enable_select |= !app_->IsTracepointSelected(tracepoint);
    enable_unselect |= app_->IsTracepointSelected(tracepoint);
  }

  std::vector<std::string> action_group;
  if (enable_select) action_group.emplace_back(std::string{kMenuActionSelect});
  if (enable_unselect) action_group.emplace_back(std::string{kMenuActionUnselect});

  std::vector<std::vector<std::string>> menu =
      DataView::GetContextMenuWithGrouping(clicked_index, selected_indices);
  menu.insert(menu.begin(), action_group);

  return menu;
}

void TracepointsDataView::OnSelectRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    app_->SelectTracepoint(GetTracepoint(i));
  }
}

void TracepointsDataView::OnUnselectRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    app_->DeselectTracepoint(GetTracepoint(i));
  }
}

void TracepointsDataView::SetTracepoints(const std::vector<TracepointInfo>& tracepoints) {
  tracepoints_.assign(tracepoints.cbegin(), tracepoints.cend());

  indices_.resize(tracepoints_.size());
  for (size_t i = 0; i < indices_.size(); ++i) {
    indices_[i] = i;
  }
}

const TracepointInfo& TracepointsDataView::GetTracepoint(uint32_t row) const {
  return tracepoints_.at(indices_[row]);
}

}  // namespace orbit_data_views