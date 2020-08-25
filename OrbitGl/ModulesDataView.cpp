// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ModulesDataView.h"

#include "App.h"
#include "Capture.h"
#include "OrbitModule.h"
#include "Pdb.h"
#include "absl/flags/flag.h"

// TODO(kuebler): remove this once we have the validator complete
ABSL_FLAG(bool, enable_frame_pointer_validator, false, "Enable validation of frame pointers");

ModulesDataView::ModulesDataView() : DataView(DataViewType::kModules) {}

const std::vector<DataView::Column>& ModulesDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnName] = {"Name", .2f, SortingOrder::kAscending};
    columns[kColumnPath] = {"Path", .5f, SortingOrder::kAscending};
    columns[kColumnAddressRange] = {"Address Range", .15f, SortingOrder::kAscending};
    columns[kColumnFileSize] = {"File Size", .0f, SortingOrder::kDescending};
    columns[kColumnLoaded] = {"Loaded", .0f, SortingOrder::kDescending};
    return columns;
  }();
  return columns;
}

std::string ModulesDataView::GetValue(int row, int col) {
  const ModuleData* module = GetModule(row);

  switch (col) {
    case kColumnName:
      return module->name();
    case kColumnPath:
      return module->file_path();
    case kColumnAddressRange:
      return module->address_range();
    case kColumnFileSize:
      return GetPrettySize(module->file_size());
    case kColumnLoaded:
      return module->is_loaded() ? "*" : "";
    default:
      return "";
  }
}

#define ORBIT_PROC_SORT(Member)                                                      \
  [&](int a, int b) {                                                                \
    return OrbitUtils::Compare(modules_[a]->Member, modules_[b]->Member, ascending); \
  }

void ModulesDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnName:
      sorter = ORBIT_PROC_SORT(name());
      break;
    case kColumnPath:
      sorter = ORBIT_PROC_SORT(file_path());
      break;
    case kColumnAddressRange:
      sorter = ORBIT_PROC_SORT(address_start());
      break;
    case kColumnFileSize:
      sorter = ORBIT_PROC_SORT(file_size());
      break;
    case kColumnLoaded:
      sorter = ORBIT_PROC_SORT(is_loaded());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

const std::string ModulesDataView::kMenuActionLoadSymbols = "Load Symbols";
const std::string ModulesDataView::kMenuActionVerifyFramePointers = "Verify Frame Pointers";

std::vector<std::string> ModulesDataView::GetContextMenu(int clicked_index,
                                                         const std::vector<int>& selected_indices) {
  bool enable_load = false;
  bool enable_verify = false;
  for (int index : selected_indices) {
    const ModuleData* module = GetModule(index);
    if (!module->is_loaded()) {
      enable_load = true;
    }

    if (module->is_loaded()) {
      enable_verify = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_load) {
    menu.emplace_back(kMenuActionLoadSymbols);
  }
  if (enable_verify && absl::GetFlag(FLAGS_enable_frame_pointer_validator)) {
    menu.emplace_back(kMenuActionVerifyFramePointers);
  }
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void ModulesDataView::OnContextMenu(const std::string& action, int menu_index,
                                    const std::vector<int>& item_indices) {
  const std::shared_ptr<Process>& process = GOrbitApp->GetSelectedProcess();
  if (action == kMenuActionLoadSymbols) {
    std::vector<std::shared_ptr<Module>> modules;
    for (int index : item_indices) {
      const ModuleData* module_data = GetModule(index);
      if (!module_data->is_loaded()) {
        modules.push_back(process->GetModuleFromPath(module_data->file_path()));
      }
    }
    GOrbitApp->LoadModules(process, GOrbitApp->GetSelectedProcessId(), modules);

  } else if (action == kMenuActionVerifyFramePointers) {
    std::vector<std::shared_ptr<Module>> modules_to_validate;
    for (int index : item_indices) {
      const ModuleData* module = GetModule(index);
      modules_to_validate.push_back(process->GetModuleFromPath(module->file_path()));
    }

    if (!modules_to_validate.empty()) {
      GOrbitApp->OnValidateFramePointers(modules_to_validate);
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void ModulesDataView::DoFilter() {
  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(ToLower(filter_), ' ');

  for (size_t i = 0; i < modules_.size(); ++i) {
    const ModuleData* module = modules_[i];
    std::string module_string = absl::StrFormat("%s %s", module->address_range(),
                                                absl::AsciiStrToLower(module->file_path()));

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (module_string.find(filter_token) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  indices_ = indices;

  OnSort(sorting_column_, {});
}

void ModulesDataView::SetModules(int32_t process_id, const std::vector<ModuleData*>& modules) {
  process_id_ = process_id;
  modules_ = modules;

  indices_.resize(modules_.size());
  for (size_t i = 0; i < indices_.size(); ++i) {
    indices_[i] = i;
  }

  OnDataChanged();
}

void ModulesDataView::OnRefreshButtonClicked() {
  GOrbitApp->UpdateProcessAndModuleList(GOrbitApp->GetSelectedProcessId(), nullptr);
}

const ModuleData* ModulesDataView::GetModule(uint32_t row) const { return modules_[indices_[row]]; }

bool ModulesDataView::GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                      unsigned char& green, unsigned char& blue) {
  const ModuleData* module = GetModule(row);
  if (module->is_loaded()) {
    red = 42;
    green = 218;
    blue = 130;
    return true;
  } else {
    red = 42;
    green = 130;
    blue = 218;
    return true;
  }
}
