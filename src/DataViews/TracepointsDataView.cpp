// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/TracepointsDataView.h"

#include <absl/strings/ascii.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <stddef.h>

#include <algorithm>
#include <functional>
#include <utility>

#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"

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

#define ORBIT_PROC_SORT(Member)                                     \
  [&](uint64_t a, uint64_t b) {                                     \
    return orbit_data_views_internal::CompareAscendingOrDescending( \
        tracepoints_[a].Member, tracepoints_[b].Member, ascending); \
  }

void TracepointsDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(uint64_t a, uint64_t b)> sorter = nullptr;

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

DataView::ActionStatus TracepointsDataView::GetActionStatus(
    std::string_view action, int clicked_index, absl::Span<const int> selected_indices) {
  std::function<bool(const TracepointInfo&)> is_visible_action_enabled;
  if (action == kMenuActionSelect) {
    is_visible_action_enabled = [this](const TracepointInfo& tracepoint) {
      return !app_->IsTracepointSelected(tracepoint);
    };

  } else if (action == kMenuActionUnselect) {
    is_visible_action_enabled = [this](const TracepointInfo& tracepoint) {
      return app_->IsTracepointSelected(tracepoint);
    };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  for (int index : selected_indices) {
    const TracepointInfo& tracepoint = GetTracepoint(index);
    if (is_visible_action_enabled(tracepoint)) return ActionStatus::kVisibleAndEnabled;
  }
  return ActionStatus::kVisibleButDisabled;
}

void TracepointsDataView::OnSelectRequested(absl::Span<const int> selection) {
  for (int i : selection) {
    app_->SelectTracepoint(GetTracepoint(i));
  }
}

void TracepointsDataView::OnUnselectRequested(absl::Span<const int> selection) {
  for (int i : selection) {
    app_->DeselectTracepoint(GetTracepoint(i));
  }
}

void TracepointsDataView::SetTracepoints(absl::Span<const TracepointInfo> tracepoints) {
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