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

void DataView::OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                             const std::vector<int>& a_ItemIndices) {
  UNUSED(a_MenuIndex);

  if (a_Action == kMenuActionExportToCsv) {
    std::string save_file = GOrbitApp->GetSaveFile(".csv");
    if (!save_file.empty()) {
      ExportCSV(save_file);
    }
  } else if (a_Action == kMenuActionCopySelection) {
    CopySelection(a_ItemIndices);
  }
}

void DataView::ExportCSV(const std::string& a_FileName) {
  std::ofstream out(a_FileName);
  if (out.fail()) return;

  size_t numColumns = GetColumns().size();
  for (size_t i = 0; i < numColumns; ++i) {
    out << GetColumns()[i].header;
    if (i < numColumns - 1) out << ", ";
  }
  out << "\n";

  size_t numElements = GetNumElements();
  for (size_t i = 0; i < numElements; ++i) {
    for (size_t j = 0; j < numColumns; ++j) {
      out << GetValue(i, j);
      if (j < numColumns - 1) out << ", ";
    }
    out << "\n";
  }

  out.close();
}

void DataView::CopySelection(const std::vector<int>& selection) {
  std::string clipboard;
  size_t numColumns = GetColumns().size();
  for (size_t i = 0; i < numColumns; ++i) {
    clipboard += GetColumns()[i].header;
    if (i < numColumns - 1) clipboard += ", ";
  }
  clipboard += "\n";

  size_t numElements = GetNumElements();
  for (size_t i : selection) {
    if (i < numElements) {
      for (size_t j = 0; j < numColumns; ++j) {
        clipboard += GetValue(i, j);
        if (j < numColumns - 1) clipboard += ", ";
      }
      clipboard += "\n";
    }
  }

  GOrbitApp->SetClipboard(clipboard);
}
