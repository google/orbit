// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/ModulesDataView.h"

#include <absl/hash/hash.h>
#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <tuple>
#include <utility>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataView.h"
#include "DataViews/DataViewType.h"
#include "DataViews/SymbolLoadingState.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"

using orbit_client_data::ModuleData;
using orbit_client_data::ModuleInMemory;
using orbit_client_data::ProcessData;

namespace orbit_data_views {

ModulesDataView::ModulesDataView(AppInterface* app) : DataView(DataViewType::kModules, app) {}

const std::vector<DataView::Column>& ModulesDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSymbols] = {"Symbols", .175f, SortingOrder::kDescending};
    columns[kColumnName] = {"Name", .2f, SortingOrder::kAscending};
    columns[kColumnPath] = {"Path", .45f, SortingOrder::kAscending};
    columns[kColumnAddressRange] = {"Address Range", .075f, SortingOrder::kAscending};
    columns[kColumnFileSize] = {"File Size", .1f, SortingOrder::kDescending};
    return columns;
  }();
  return columns;
}

std::string ModulesDataView::GetValue(int row, int col) {
  uint64_t start_address = indices_[row];
  const ModuleData* module = start_address_to_module_.at(start_address);
  const ModuleInMemory& memory_space = start_address_to_module_in_memory_.at(start_address);

  switch (col) {
    case kColumnSymbols:
      return app_->GetSymbolLoadingStateForModule(module).GetName();
    case kColumnName:
      return std::filesystem::path(module->file_path()).filename().string();
    case kColumnPath:
      return module->file_path();
    case kColumnAddressRange:
      return memory_space.FormattedAddressRange();
    case kColumnFileSize:
      return orbit_display_formats::GetDisplaySize(module->file_size());
    default:
      return "";
  }
}

std::string ModulesDataView::GetToolTip(int row, int column) {
  uint64_t start_address = indices_[row];
  const ModuleData* module = start_address_to_module_.at(start_address);

  if (column == kColumnSymbols) {
    return app_->GetSymbolLoadingStateForModule(module).GetDescription();
  }
  return DataView::GetToolTip(row, column);
}

#define ORBIT_PROC_SORT(Member)                                                         \
  [&](uint64_t a, uint64_t b) {                                                         \
    return orbit_data_views_internal::CompareAscendingOrDescending(                     \
        start_address_to_module_.at(a)->Member, start_address_to_module_.at(b)->Member, \
        ascending);                                                                     \
  }

#define ORBIT_MODULE_SPACE_SORT(Member)                              \
  [&](uint64_t a, uint64_t b) {                                      \
    return orbit_data_views_internal::CompareAscendingOrDescending(  \
        start_address_to_module_in_memory_.at(a).Member,             \
        start_address_to_module_in_memory_.at(b).Member, ascending); \
  }

void ModulesDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(uint64_t, uint64_t)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnSymbols:
      sorter = ORBIT_PROC_SORT(GetLoadedSymbolsCompleteness());
      break;
    case kColumnName:
      sorter = [&](uint64_t a, uint64_t b) {
        return orbit_data_views_internal::CompareAscendingOrDescending(
            std::filesystem::path(start_address_to_module_.at(a)->file_path()).filename(),
            std::filesystem::path(start_address_to_module_.at(b)->file_path()).filename(),
            ascending);
      };
      break;
    case kColumnPath:
      sorter = ORBIT_PROC_SORT(file_path());
      break;
    case kColumnAddressRange:
      sorter = ORBIT_MODULE_SPACE_SORT(start());
      break;
    case kColumnFileSize:
      sorter = ORBIT_PROC_SORT(file_size());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

DataView::ActionStatus ModulesDataView::GetActionStatus(std::string_view action, int clicked_index,
                                                        absl::Span<const int> selected_indices) {
  // transform selected_indices into modules
  std::vector<const ModuleData*> modules;
  modules.reserve(selected_indices.size());
  for (int index : selected_indices) {
    const ModuleData* module = GetModuleDataFromRow(index);
    ORBIT_CHECK(module);
    modules.emplace_back(module);
  }

  bool at_least_one_module_can_be_loaded =
      std::any_of(modules.begin(), modules.end(), [this](const ModuleData* module) {
        return !module->AreDebugSymbolsLoaded() &&
               !app_->IsSymbolLoadingInProgressForModule(module);
      });

  bool at_least_one_module_is_downloading =
      std::any_of(modules.begin(), modules.end(),
                  [this](const ModuleData* module) { return app_->IsModuleDownloading(module); });

  if (action == kMenuActionLoadSymbols) {
    if (at_least_one_module_can_be_loaded) {
      return ActionStatus::kVisibleAndEnabled;
    }
    // if no module can be loaded, but there are current downloads in progress, do *not* show the
    // "Load Symbols" action at all. The "Stop Download..." action will be there instead of "Load
    // Symbols"
    if (at_least_one_module_is_downloading) {
      return ActionStatus::kInvisible;
    }
    // Otherwise (no module can be loaded and no module is currently downloading), then show the
    // *disabled* "Load Symbols" action.
    return ActionStatus::kVisibleButDisabled;
  }

  if (action == kMenuActionStopDownload) {
    return at_least_one_module_is_downloading ? ActionStatus::kVisibleAndEnabled
                                              : ActionStatus::kInvisible;
  }

  return DataView::GetActionStatus(action, clicked_index, selected_indices);
}

void ModulesDataView::OnDoubleClicked(int index) {
  ModuleData* module_data = GetModuleDataFromRow(index);
  if (!module_data->AreDebugSymbolsLoaded()) {
    std::vector<ModuleData*> modules_to_load = {module_data};
    app_->LoadSymbolsManually(modules_to_load);
  }
}

void ModulesDataView::DoFilter() {
  std::vector<uint64_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (const auto& [start_address, memory_space] : start_address_to_module_in_memory_) {
    std::string module_string = absl::StrFormat("%s %s", memory_space.FormattedAddressRange(),
                                                absl::AsciiStrToLower(memory_space.file_path()));

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (!absl::StrContains(module_string, filter_token)) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(start_address);
    }
  }

  indices_ = std::move(indices);
}

void ModulesDataView::AddModule(uint64_t start_address, ModuleData* module,
                                ModuleInMemory module_in_memory) {
  start_address_to_module_.insert_or_assign(start_address, std::move(module));
  start_address_to_module_in_memory_.insert_or_assign(start_address, std::move(module_in_memory));
  indices_.push_back(start_address);
}

void ModulesDataView::UpdateModules(const ProcessData* process) {
  start_address_to_module_.clear();
  start_address_to_module_in_memory_.clear();
  auto memory_map = process->GetMemoryMapCopy();
  indices_.clear();
  for (const auto& [start_address, module_in_memory] : memory_map) {
    ModuleData* module = app_->GetMutableModuleByModuleIdentifier(module_in_memory.module_id());

    // The module here cannot be a nullptr, because the call
    // `app_->GetMutableModuleByModuleIdentifier` is relaying the call to
    // `ModuleManager::GetMutableModuleByModuleIdentifier` and ModuleManager never deletes modules,
    // only changes modules or adds new modules. And the memory_map cannot contain modules that are
    // not yet in ModuleManager, since these two locations get updated simultaneously.
    ORBIT_CHECK(module != nullptr);
    AddModule(start_address, module, module_in_memory);
  }

  OnDataChanged();
}

void ModulesDataView::OnRefreshButtonClicked() {
  const ProcessData* process = app_->GetTargetProcess();
  if (process == nullptr) {
    ORBIT_LOG("Unable to refresh module list, no process selected");
    return;
  }
  std::ignore = app_->UpdateProcessAndModuleList();
}

bool ModulesDataView::GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                      unsigned char& green, unsigned char& blue) {
  const ModuleData* module = GetModuleDataFromRow(row);
  return app_->GetSymbolLoadingStateForModule(module).GetDisplayColor(red, green, blue);
}

}  // namespace orbit_data_views
