// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/DataView.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <algorithm>
#include <iterator>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ProcessData.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::ProcessData;

namespace orbit_data_views {

std::string FormatValueForCsv(std::string_view value) {
  std::string result;
  result.append("\"");
  result.append(absl::StrReplaceAll(value, {{"\"", "\"\""}}));
  result.append("\"");
  return result;
}

void DataView::Init() { InitSortingOrders(); }

void DataView::InitSortingOrders() {
  ORBIT_CHECK(sorting_orders_.empty());
  for (const auto& column : GetColumns()) {
    sorting_orders_.push_back(column.initial_order);
  }

  sorting_column_ = GetDefaultSortingColumn();
}

void DataView::OnSort(int column, std::optional<SortingOrder> new_order) {
  ORBIT_SCOPE_FUNCTION;

  if (!IsSortingAllowed()) {
    return;
  }

  ORBIT_CHECK(column >= 0);
  ORBIT_CHECK(static_cast<size_t>(column) < sorting_orders_.size());

  sorting_column_ = column;
  if (new_order.has_value()) {
    sorting_orders_[column] = new_order.value();
  }

  {
    ORBIT_SCOPE(absl::StrFormat("DoSort column[%i]", sorting_column_).c_str());
    DoSort();
  }
}

void DataView::OnFilter(std::string filter) {
  filter_ = std::move(filter);
  DoFilter();
  OnSort(sorting_column_, {});
}

void DataView::SetUiFilterString(std::string_view filter) {
  if (filter_callback_) {
    filter_callback_(filter);
  }
}

void DataView::OnDataChanged() {
  ORBIT_SCOPE_FUNCTION;
  DoFilter();
  OnSort(sorting_column_, std::optional<SortingOrder>{});
}

DataView::ActionStatus DataView::GetActionStatus(std::string_view action, int /* clicked_index */,
                                                 absl::Span<const int> /* selected_indices */) {
  if (action == kMenuActionCopySelection || action == kMenuActionExportToCsv) {
    return ActionStatus::kVisibleAndEnabled;
  }

  return ActionStatus::kInvisible;
}

std::vector<DataView::ActionGroup> DataView::GetContextMenuWithGrouping(
    int clicked_index, absl::Span<const int> selected_indices) {
  // GetContextmenuWithGrouping is called when OrbitTreeView::indexAt returns a valid index and
  // hence the selected_indices retrieved from OrbitTreeView::selectionModel()->selectedIndexes()
  // should not be empty.
  ORBIT_CHECK(!selected_indices.empty());

  std::vector<ActionGroup> menu;
  auto try_add_action_group = [&](const std::vector<std::string_view>& action_names) {
    ActionGroup action_group;
    for (std::string_view action_name : action_names) {
      switch (GetActionStatus(action_name, clicked_index, selected_indices)) {
        case ActionStatus::kVisibleAndEnabled:
          action_group.emplace_back(action_name, /*enabled=*/true);
          break;
        case ActionStatus::kVisibleButDisabled:
          action_group.emplace_back(action_name, /*enabled=*/false);
          break;
        case ActionStatus::kInvisible:
          break;
      }
    }
    if (!action_group.empty()) menu.push_back(std::move(action_group));
  };

  // Hooking related actions
  try_add_action_group({kMenuActionLoadSymbols, kMenuActionStopDownload, kMenuActionSelect,
                        kMenuActionUnselect, kMenuActionEnableFrameTrack,
                        kMenuActionDisableFrameTrack});

  try_add_action_group({kMenuActionDisassembly, kMenuActionSourceCode});

  // Navigating related actions
  try_add_action_group({kMenuActionAddIterator, kMenuActionJumpToFirst, kMenuActionJumpToLast,
                        kMenuActionJumpToMin, kMenuActionJumpToMax});

  // Preset related actions
  try_add_action_group({kMenuActionLoadPreset, kMenuActionDeletePreset, kMenuActionShowInExplorer});

  // Exporting relate actions
  try_add_action_group(
      {kMenuActionCopySelection, kMenuActionExportToCsv, kMenuActionExportEventsToCsv});
  return menu;
}

void DataView::OnContextMenu(std::string_view action, int /*menu_index*/,
                             absl::Span<const int> item_indices) {
  if (action == kMenuActionLoadSymbols) {
    OnLoadSymbolsRequested(item_indices);
  } else if (action == kMenuActionStopDownload) {
    OnStopDownloadRequested(item_indices);
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

void DataView::OnLoadSymbolsRequested(absl::Span<const int> selection) {
  std::vector<const ModuleData*> modules_to_load;
  for (int index : selection) {
    const ModuleData* module_data = GetModuleDataFromRow(index);
    if (module_data != nullptr && !module_data->AreDebugSymbolsLoaded()) {
      modules_to_load.push_back(module_data);
    }
  }
  app_->LoadSymbolsManually(modules_to_load);
}

void DataView::OnStopDownloadRequested(absl::Span<const int> selection) {
  std::vector<const ModuleData*> modules_to_stop;
  for (int index : selection) {
    const ModuleData* module = GetModuleDataFromRow(index);
    ORBIT_CHECK(module != nullptr);
    if (app_->IsModuleDownloading(module)) {
      modules_to_stop.push_back(module);
    }
  }
  app_->RequestSymbolDownloadStop(modules_to_stop);
}

void DataView::OnSelectRequested(absl::Span<const int> selection) {
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    // Only hook functions for which we have symbols loaded.
    if (function != nullptr) {
      app_->SelectFunction(*function);
    }
  }
}

void DataView::OnUnselectRequested(absl::Span<const int> selection) {
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

void DataView::OnEnableFrameTrackRequested(absl::Span<const int> selection) {
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    if (function == nullptr) continue;
    // Functions used as frame tracks must be hooked (selected), otherwise the
    // data to produce the frame track will not be captured.
    // The condition is supposed to prevent "selecting" a function when a capture
    // is loaded with no connection to a process being established.
    if (GetActionStatus(kMenuActionSelect, i, {i}) == ActionStatus::kVisibleAndEnabled) {
      app_->SelectFunction(*function);
    }

    app_->EnableFrameTrack(*function);
    app_->AddFrameTrack(*function);
  }
}

void DataView::OnDisableFrameTrackRequested(absl::Span<const int> selection) {
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    if (function == nullptr) continue;

    // When we remove a frame track, we do not unhook (deselect) the function as
    // it may have been selected manually (not as part of adding a frame track).
    // However, disable the frame track, so it is not recreated on the next capture.
    app_->DisableFrameTrack(*function);
    app_->RemoveFrameTrack(*function);
  }
}

void DataView::OnDisassemblyRequested(absl::Span<const int> selection) {
  const ProcessData* process_data = app_->GetTargetProcess();
  const uint32_t pid =
      process_data == nullptr ? app_->GetCaptureData().process_id() : process_data->pid();
  constexpr int kMaxNumberOfWindowsToOpen = 10;
  int j = 0;
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    if (function != nullptr) {
      app_->Disassemble(pid, *function);
      if (++j >= kMaxNumberOfWindowsToOpen) break;
    }
  }
}

void DataView::OnSourceCodeRequested(absl::Span<const int> selection) {
  constexpr int kMaxNumberOfWindowsToOpen = 10;
  int j = 0;
  for (int i : selection) {
    const FunctionInfo* function = GetFunctionInfoFromRow(i);
    if (function != nullptr) {
      app_->ShowSourceCode(*function);
      if (++j >= kMaxNumberOfWindowsToOpen) break;
    }
  }
}

ErrorMessageOr<void> DataView::ExportToCsv(const std::string_view file_path) {
  OUTCOME_TRY(auto fd, orbit_base::OpenFileForWriting(file_path));

  std::vector<std::string> column_names;
  const std::vector<Column>& columns = GetColumns();
  std::transform(std::begin(columns), std::end(columns), std::back_inserter(column_names),
                 [](const Column& column) { return column.header; });
  OUTCOME_TRY(WriteLineToCsv(fd, column_names));

  const size_t num_columns = column_names.size();

  size_t num_elements = GetNumElements();
  for (size_t i = 0; i < num_elements; ++i) {
    std::string line;
    for (size_t j = 0; j < num_columns; ++j) {
      line.append(FormatValueForCsv(GetValueForCopy(i, j)));
      if (j < num_columns - 1) line.append(kFieldSeparator);
    }
    line.append(kLineSeparator);
    OUTCOME_TRY(orbit_base::WriteFully(fd, line));
  }
  return outcome::success();
}

void DataView::OnExportToCsvRequested() {
  std::string save_file = app_->GetSaveFile(".csv");
  if (save_file.empty()) return;

  ReportErrorIfAny(ExportToCsv(save_file), kMenuActionExportToCsv);
}

void DataView::OnCopySelectionRequested(absl::Span<const int> selection) {
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
