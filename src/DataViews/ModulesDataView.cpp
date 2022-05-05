// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/ModulesDataView.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <functional>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientFlags/ClientFlags.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Logging.h"

using orbit_client_data::ModuleData;
using orbit_client_data::ModuleInMemory;
using orbit_client_data::ProcessData;

namespace orbit_data_views {

ModulesDataView::ModulesDataView(AppInterface* app,
                                 orbit_metrics_uploader::MetricsUploader* metrics_uploader,
                                 bool new_ui)
    : DataView(DataViewType::kModules, app, metrics_uploader), new_ui_(new_ui) {}

const std::vector<DataView::Column>& ModulesDataView::GetColumns() {
  static const std::vector<Column> columns = [this] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    if (new_ui_) {
      columns[kColumnSymbols] = {"Symbols", .175f, SortingOrder::kDescending};
    } else {
      columns[kColumnSymbols] = {"Loaded", .0f, SortingOrder::kDescending};
    }
    columns[kColumnName] = {"Name", .2f, SortingOrder::kAscending};
    columns[kColumnPath] = {"Path", .45f, SortingOrder::kAscending};
    columns[kColumnAddressRange] = {"Address Range", .075f, SortingOrder::kAscending};
    columns[kColumnFileSize] = {"File Size", .1f, SortingOrder::kDescending};
    return columns;
  }();
  return columns;
}

std::string ModulesDataView::GetSymbolLoadingStateForModuleString(const ModuleData* module) {
  SymbolLoadingState loading_state = app_->GetSymbolLoadingStateForModule(module);
  return loading_state.GetDescription();
}

std::string ModulesDataView::GetValue(int row, int col) {
  uint64_t start_address = indices_[row];
  const ModuleData* module = start_address_to_module_.at(start_address);
  const ModuleInMemory& memory_space = start_address_to_module_in_memory_.at(start_address);

  switch (col) {
    case kColumnSymbols: {
      if (new_ui_) {
        return GetSymbolLoadingStateForModuleString(module);
      }
      return module->is_loaded() ? "*" : "";
    }
    case kColumnName:
      return module->name();
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
      sorter = ORBIT_PROC_SORT(is_loaded());
      break;
    case kColumnName:
      sorter = ORBIT_PROC_SORT(name());
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
                                                        const std::vector<int>& selected_indices) {
  if (action == kMenuActionVerifyFramePointers &&
      !absl::GetFlag(FLAGS_enable_frame_pointer_validator)) {
    return ActionStatus::kInvisible;
  }

  std::function<bool(const ModuleData*)> is_visible_action_enabled;
  if (action == kMenuActionLoadSymbols) {
    is_visible_action_enabled = [](const ModuleData* module) { return !module->is_loaded(); };

  } else if (action == kMenuActionVerifyFramePointers) {
    is_visible_action_enabled = [](const ModuleData* module) { return module->is_loaded(); };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  for (int index : selected_indices) {
    const ModuleData* module = GetModuleDataFromRow(index);
    if (is_visible_action_enabled(module)) return ActionStatus::kVisibleAndEnabled;
  }
  return ActionStatus::kVisibleButDisabled;
}

void ModulesDataView::OnDoubleClicked(int index) {
  ModuleData* module_data = GetModuleDataFromRow(index);
  if (!module_data->is_loaded()) {
    std::vector<ModuleData*> modules_to_load = {module_data};
    app_->RetrieveModulesAndLoadSymbols(modules_to_load);
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
    ModuleData* module = app_->GetMutableModuleByPathAndBuildId(module_in_memory.file_path(),
                                                                module_in_memory.build_id());

    // The module here cannot be a nullptr, because the call
    // `app_->GetMutableModuleByPathAndBuildId` is relaying the call to
    // `ModuleManager::GetMutableModuleByPathAndBuildId` and ModuleManager never deletes modules,
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
  app_->UpdateProcessAndModuleList();
}

bool ModulesDataView::GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                      unsigned char& green, unsigned char& blue) {
  const ModuleData* module = GetModuleDataFromRow(row);

  if (new_ui_) {
    return app_->GetSymbolLoadingStateForModule(module).GetDisplayColor(red, green, blue);
  }

  if (module->is_loaded()) {
    red = 42;
    green = 218;
    blue = 130;
  } else {
    red = 42;
    green = 130;
    blue = 218;
  }
  return true;
}

}  // namespace orbit_data_views
