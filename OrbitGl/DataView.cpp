// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataView.h"

#include <fstream>

#include "App.h"

void DataView::InitSortingOrders() {
  sorting_orders_.clear();
  for (const auto& column : GetColumns()) {
    sorting_orders_.push_back(column.initial_order);
  }

  sorting_column_ = GetDefaultSortingColumn();
}

void DataView::OnSort(int column, std::optional<SortingOrder> new_order) {
  if (column < 0) {
    return;
  }

  if (!IsSortingAllowed()) {
    return;
  }

  if (sorting_orders_.empty()) {
    InitSortingOrders();
  }

  sorting_column_ = column;
  if (new_order.has_value()) {
    sorting_orders_[column] = new_order.value();
  }

  DoSort();
}

void DataView::OnFilter(const std::string& filter) {
  filter_ = filter;
  DoFilter();
}

void DataView::SetUiFilterString(const std::string& filter) {
  if (filter_callback_) {
    filter_callback_(filter);
  }
}

void DataView::OnDataChanged() {
  OnSort(sorting_column_, std::optional<SortingOrder>{});
  OnFilter(filter_);
}

const std::string DataView::kMenuActionCopySelection = "Copy Selection";
const std::string DataView::kMenuActionExportToCsv = "Export to CSV";

std::vector<std::string> DataView::GetContextMenu(int /*clicked_index*/,
                                                  const std::vector<int>& /*selected_indices*/) {
  static std::vector<std::string> menu = {kMenuActionCopySelection, kMenuActionExportToCsv};
  return menu;
}

void DataView::OnContextMenu(const std::string& action, int /*menu_index*/,
                             const std::vector<int>& item_indices) {
  if (action == kMenuActionExportToCsv) {
    std::string save_file = GOrbitApp->GetSaveFile(".csv");
    if (!save_file.empty()) {
      ExportCSV(save_file);
    }
  } else if (action == kMenuActionCopySelection) {
    CopySelection(item_indices);
  }
}

void DataView::ExportCSV(const std::string& file_path) {
  std::ofstream out(file_path);
  if (out.fail()) return;

  size_t num_columns = GetColumns().size();
  for (size_t i = 0; i < num_columns; ++i) {
    out << GetColumns()[i].header;
    if (i < num_columns - 1) out << ", ";
  }
  out << "\n";

  size_t num_elements = GetNumElements();
  for (size_t i = 0; i < num_elements; ++i) {
    for (size_t j = 0; j < num_columns; ++j) {
      out << GetValue(i, j);
      if (j < num_columns - 1) out << ", ";
    }
    out << "\n";
  }

  out.close();
}

void DataView::CopySelection(const std::vector<int>& selection) {
  std::string clipboard;
  size_t num_columns = GetColumns().size();
  for (size_t i = 0; i < num_columns; ++i) {
    clipboard += GetColumns()[i].header;
    if (i < num_columns - 1) clipboard += ", ";
  }
  clipboard += "\n";

  size_t num_elements = GetNumElements();
  for (size_t i : selection) {
    if (i < num_elements) {
      for (size_t j = 0; j < num_columns; ++j) {
        clipboard += GetValue(i, j);
        if (j < num_columns - 1) clipboard += ", ";
      }
      clipboard += "\n";
    }
  }

  GOrbitApp->SetClipboard(clipboard);
}
