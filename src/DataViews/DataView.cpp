// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/DataView.h"

#include <absl/strings/str_replace.h>

#include <memory>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

using orbit_client_data::ModuleData;
using orbit_client_data::ProcessData;
using orbit_client_protos::FunctionInfo;

namespace orbit_data_views {

std::string FormatValueForCsv(std::string_view value) {
  std::string result;
  result.append("\"");
  result.append(absl::StrReplaceAll(value, {{"\"", "\"\""}}));
  result.append("\"");
  return result;
}

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
  OnSort(sorting_column_, {});
}

void DataView::SetUiFilterString(const std::string& filter) {
  if (filter_callback_) {
    filter_callback_(filter);
  }
}

void DataView::OnDataChanged() {
  DoFilter();
  OnSort(sorting_column_, std::optional<SortingOrder>{});
}

std::vector<std::vector<std::string>> DataView::GetContextMenuWithGrouping(
    int /*clicked_index*/, const std::vector<int>& selected_indices) {
  // GetContextmenuWithGrouping is called when OrbitTreeView::indexAt returns a valid index and
  // hence the selected_indices retrieved from OrbitTreeView::selectionModel()->selectedIndexes()
  // should not be empty.
  CHECK(!selected_indices.empty());

  static std::vector<std::string> default_group = {std::string{kMenuActionCopySelection},
                                                   std::string{kMenuActionExportToCsv}};
  return {default_group};
}

void DataView::OnContextMenu(const std::string& action, int /*menu_index*/,
                             const std::vector<int>& item_indices) {
  if (action == kMenuActionLoadSymbols) {
    OnLoadSymbolsRequested(item_indices);
  } else if (action == kMenuActionSelect) {
    OnSelectRequested(item_indices);
  } else if (action == kMenuActionUnselect) {
    OnUnselectRequested(item_indices);
  } else if (action == kMenuActionEnableFrameTrack) {
    OnEnableFrameTrackRequested(item_indices);
  } else if (action == kMenuActionDisableFrameTrack) {
    OnDisableFrameTrackRequested(item_indices);
  } else if (action == kMenuActionAddIterator) {
    OnIteratorRequested(item_indices);
  } else if (action == kMenuActionVerifyFramePointers) {
    OnVerifyFramePointersRequested(item_indices);

  } else if (action == kMenuActionDisassembly) {
    OnDisassemblyRequested(item_indices);
  } else if (action == kMenuActionSourceCode) {
    OnSourceCodeRequested(item_indices);

  } else if (action == kMenuActionJumpToFirst || action == kMenuActionJumpToLast ||
             action == kMenuActionJumpToMin || action == kMenuActionJumpToMax) {
    OnJumpToRequested(action, item_indices);

  } else if (action == kMenuActionLoadPreset) {
    OnLoadPresetRequested(item_indices);
  } else if (action == kMenuActionDeletePreset) {
    OnDeletePresetRequested(item_indices);
  } else if (action == kMenuActionShowInExplorer) {
    OnShowInExplorerRequested(item_indices);

  } else if (action == kMenuActionExportToCsv) {
    OnExportToCsvRequested();
  } else if (action == kMenuActionCopySelection) {
    OnCopySelectionRequested(item_indices);
  } else if (action == kMenuActionExportEventsToCsv) {
    OnExportEventsToCsvRequested(item_indices);
  }
}

std::vector<int> DataView::GetVisibleSelectedIndices() {
  std::vector<int> visible_selected_indices;
  for (size_t row = 0; row < indices_.size(); ++row) {
    if (selected_indices_.contains(indices_[row])) {
      visible_selected_indices.push_back(static_cast<int>(row));
    }
  }
  return visible_selected_indices;
}

void DataView::OnLoadSymbolsRequested(const std::vector<int>& selection) {
  std::vector<ModuleData*> modules_to_load;
  for (int index : selection) {
    ModuleData* module_data = GetModuleDataFromRow(index);
    if (module_data != nullptr && !module_data->is_loaded()) {
      modules_to_load.push_back(module_data);
    }
  }
  app_->RetrieveModulesAndLoadSymbols(modules_to_load);
}

void DataView::OnSelectRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    // Only hook functions for which we have symbols loaded.
    if (function != nullptr) {
      app_->SelectFunction(*function);
    }
  }
}

void DataView::OnUnselectRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    // If the frame belongs to a function for which no symbol is loaded 'function' is nullptr and
    // we can skip it since it can't be instrumented.
    if (function != nullptr) {
      app_->DeselectFunction(*function);
      // Unhooking a function implies disabling (and removing) the frame track for this function.
      // While it would be possible to keep the current frame track in the capture data, this would
      // lead to a somewhat inconsistent state where the frame track for this function is enabled
      // for the current capture but disabled for the next one.
      app_->DisableFrameTrack(*function);
      app_->RemoveFrameTrack(*function);
    }
  }
}

void DataView::OnEnableFrameTrackRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    const FunctionInfo& function = *GetFunctionInfoFromRow(i);
    // Functions used as frame tracks must be hooked (selected), otherwise the
    // data to produce the frame track will not be captured.
    if (app_->IsCaptureConnected(app_->GetCaptureData())) {
      app_->SelectFunction(function);
    }
    app_->EnableFrameTrack(function);
    app_->AddFrameTrack(function);
  }
}

void DataView::OnDisableFrameTrackRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    const FunctionInfo& function = *GetFunctionInfoFromRow(i);
    // When we remove a frame track, we do not unhook (deselect) the function as
    // it may have been selected manually (not as part of adding a frame track).
    // However, disable the frame track, so it is not recreated on the next capture.
    app_->DisableFrameTrack(function);
    app_->RemoveFrameTrack(function);
  }
}

void DataView::OnVerifyFramePointersRequested(const std::vector<int>& selection) {
  std::vector<const ModuleData*> modules_to_validate;
  modules_to_validate.reserve(selection.size());
  for (int i : selection) {
    const ModuleData* module = GetModuleDataFromRow(i);
    modules_to_validate.push_back(module);
  }

  if (!modules_to_validate.empty()) {
    app_->OnValidateFramePointers(modules_to_validate);
  }
}

void DataView::OnDisassemblyRequested(const std::vector<int>& selection) {
  const ProcessData* process_data = app_->GetTargetProcess();
  const uint32_t pid =
      process_data == nullptr ? app_->GetCaptureData().process_id() : process_data->pid();
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    if (function != nullptr) app_->Disassemble(pid, *function);
  }
}

void DataView::OnSourceCodeRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    if (function != nullptr) app_->ShowSourceCode(*function);
  }
}

void DataView::OnExportToCsvRequested() {
  std::string save_file = app_->GetSaveFile(".csv");
  if (save_file.empty()) return;

  auto send_error = [&](const std::string& error_msg) {
    app_->SendErrorToUi(std::string{kMenuActionExportToCsv}, error_msg);
  };

  ErrorMessageOr<orbit_base::unique_fd> result = orbit_base::OpenFileForWriting(save_file);
  if (result.has_error()) {
    send_error(
        absl::StrFormat("Failed to open \"%s\" file: %s", save_file, result.error().message()));
    return;
  }

  const orbit_base::unique_fd& fd = result.value();

  constexpr const char* kFieldSeparator = ",";
  // CSV RFC requires lines to end with CRLF
  constexpr const char* kLineSeparator = "\r\n";

  size_t num_columns = GetColumns().size();
  {
    std::string header_line;
    for (size_t i = 0; i < num_columns; ++i) {
      header_line.append(FormatValueForCsv(GetColumns()[i].header));
      if (i < num_columns - 1) header_line.append(kFieldSeparator);
    }

    header_line.append(kLineSeparator);
    auto write_result = orbit_base::WriteFully(fd, header_line);
    if (write_result.has_error()) {
      send_error(absl::StrFormat("Error writing to \"%s\": %s", save_file,
                                 write_result.error().message()));
      return;
    }
  }

  size_t num_elements = GetNumElements();
  for (size_t i = 0; i < num_elements; ++i) {
    std::string line;
    for (size_t j = 0; j < num_columns; ++j) {
      line.append(FormatValueForCsv(GetValueForCopy(i, j)));
      if (j < num_columns - 1) line.append(kFieldSeparator);
    }
    line.append(kLineSeparator);
    auto write_result = orbit_base::WriteFully(fd, line);
    if (write_result.has_error()) {
      send_error(absl::StrFormat("Error writing to \"%s\": %s", save_file,
                                 write_result.error().message()));
      return;
    }
  }
}

void DataView::OnCopySelectionRequested(const std::vector<int>& selection) {
  constexpr const char* kFieldSeparator = "\t";
  constexpr const char* kLineSeparator = "\n";

  std::string clipboard;
  size_t num_columns = GetColumns().size();
  for (size_t i = 0; i < num_columns; ++i) {
    clipboard += GetColumns()[i].header;
    if (i < num_columns - 1) clipboard.append(kFieldSeparator);
  }
  clipboard.append(kLineSeparator);

  size_t num_elements = GetNumElements();
  for (size_t i : selection) {
    if (i >= num_elements) continue;
    for (size_t j = 0; j < num_columns; ++j) {
      clipboard += GetValueForCopy(i, j);
      if (j < num_columns - 1) clipboard.append(kFieldSeparator);
    }
    clipboard.append(kLineSeparator);
  }

  app_->SetClipboard(clipboard);
}

}  // namespace orbit_data_views