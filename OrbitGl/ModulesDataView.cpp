// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ModulesDataView.h"

#include "App.h"
#include "Core.h"
#include "OrbitModule.h"
#include "Pdb.h"
#include "absl/flags/flag.h"

// TODO(kuebler): remove this once we have the validator complete
ABSL_FLAG(bool, enable_frame_pointer_validator, false,
          "Enable validation of frame pointers");

//-----------------------------------------------------------------------------
ModulesDataView::ModulesDataView() : DataView(DataViewType::MODULES) {}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& ModulesDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_INDEX] = {"Index", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Name", .2f, SortingOrder::Ascending};
    columns[COLUMN_PATH] = {"Path", .3f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS_RANGE] = {"Address Range", .15f,
                                     SortingOrder::Ascending};
    columns[COLUMN_FILE_SIZE] = {"File Size", .0f, SortingOrder::Descending};
    columns[COLUMN_LOADED] = {"Loaded", .0f, SortingOrder::Descending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string ModulesDataView::GetValue(int row, int col) {
  const ModuleData* module = GetModule(row);

  switch (col) {
    case COLUMN_INDEX:
      return std::to_string(row);
    case COLUMN_NAME:
      return module->name();
    case COLUMN_PATH:
      return module->file_path();
    case COLUMN_ADDRESS_RANGE:
      return module->address_range();
    case COLUMN_FILE_SIZE:
      return GetPrettySize(module->file_size());
    case COLUMN_LOADED:
      return module->is_loaded() ? "*" : "";
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member)                                          \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(modules_[a]->Member, modules_[b]->Member, \
                               ascending);                               \
  }

//-----------------------------------------------------------------------------
void ModulesDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (m_SortingColumn) {
    case COLUMN_NAME:
      sorter = ORBIT_PROC_SORT(name());
      break;
    case COLUMN_PATH:
      sorter = ORBIT_PROC_SORT(file_path());
      break;
    case COLUMN_ADDRESS_RANGE:
      sorter = ORBIT_PROC_SORT(address_start());
      break;
    case COLUMN_FILE_SIZE:
      sorter = ORBIT_PROC_SORT(file_size());
      break;
    case COLUMN_LOADED:
      sorter = ORBIT_PROC_SORT(is_loaded());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
const std::string ModulesDataView::MENU_ACTION_MODULES_LOAD = "Load Symbols";
const std::string ModulesDataView::MENU_ACTION_MODULES_VERIFY =
    "Verify Frame Pointers";

//-----------------------------------------------------------------------------
std::vector<std::string> ModulesDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
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
    menu.emplace_back(MENU_ACTION_MODULES_LOAD);
  }
  if (enable_verify && absl::GetFlag(FLAGS_enable_frame_pointer_validator)) {
    menu.emplace_back(MENU_ACTION_MODULES_VERIFY);
  }
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

//-----------------------------------------------------------------------------
void ModulesDataView::OnContextMenu(const std::string& action, int menu_index,
                                    const std::vector<int>& item_indices) {
  if (action == MENU_ACTION_MODULES_LOAD) {
    std::vector<std::shared_ptr<Module>> modules;
    for (int index : item_indices) {
      const ModuleData* module_data = GetModule(index);
      if (!module_data->is_loaded()) {
        modules.push_back(Capture::GTargetProcess->GetModuleFromPath(
            module_data->file_path()));
      }
    }
    GOrbitApp->LoadModules(Capture::GTargetProcess->GetID(), modules);

  } else if (action == MENU_ACTION_MODULES_VERIFY) {
    std::vector<std::shared_ptr<Module>> modules_to_validate;
    for (int index : item_indices) {
      const ModuleData* module = GetModule(index);
      modules_to_validate.push_back(
          Capture::GTargetProcess->GetModuleFromName(module->name()));
    }

    if (!modules_to_validate.empty()) {
      GOrbitApp->OnValidateFramePointers(modules_to_validate);
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

//-----------------------------------------------------------------------------
void ModulesDataView::DoFilter() {
  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(ToLower(m_Filter), ' ');

  for (size_t i = 0; i < modules_.size(); ++i) {
    const ModuleData* module = modules_[i];
    std::string module_string =
        absl::StrFormat("%s %s", module->address_range(),
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

  m_Indices = indices;

  OnSort(m_SortingColumn, {});
}

//-----------------------------------------------------------------------------
void ModulesDataView::SetModules(int32_t process_id,
                                 const std::vector<ModuleData*>& modules) {
  process_id_ = process_id;
  modules_ = modules;

  m_Indices.resize(modules_.size());
  for (size_t i = 0; i < m_Indices.size(); ++i) {
    m_Indices[i] = i;
  }

  OnDataChanged();
}

//-----------------------------------------------------------------------------
const ModuleData* ModulesDataView::GetModule(uint32_t row) const {
  return modules_[m_Indices[row]];
}

//-----------------------------------------------------------------------------
bool ModulesDataView::GetDisplayColor(int row, int /*column*/,
                                      unsigned char& red, unsigned char& green,
                                      unsigned char& blue) {
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

  return false;
}
