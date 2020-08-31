// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PresetsDataView.h"

#include <OrbitBase/Logging.h>
#include <OrbitBase/SafeStrerror.h>

#include <cstdio>

#include "App.h"
#include "Callstack.h"
#include "ModulesDataView.h"
#include "Path.h"
#include "Pdb.h"
#include "PresetLoadState.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"

using orbit_client_protos::PresetFile;
using orbit_client_protos::PresetInfo;

constexpr const char* kLoadableColumnName = "Loadable";
constexpr const char* kPresetColumnName = "Preset";
constexpr const char* kModulesColumnName = "Modules";
constexpr const char* kHookedFunctionsColumnName = "Hooked Functions";

constexpr const float kLoadableColumnWidth = 0.14f;
constexpr const float kPresetColumnWidth = 0.34f;
constexpr const float kModulesColumnWidth = 0.34f;
constexpr const float kHookedFunctionsColumnWidth = 0.16f;

namespace {
std::string GetLoadStateString(std::shared_ptr<orbit_client_protos::PresetFile> preset) {
  PresetLoadState load_state = GOrbitApp->GetPresetLoadState(preset);
  return load_state.GetName();
}
}  // namespace

PresetsDataView::PresetsDataView() : DataView(DataViewType::kPresets) {}

std::string PresetsDataView::GetModulesList(const std::vector<ModuleView>& modules) const {
  return absl::StrJoin(modules, "\n", [](std::string* out, const ModuleView& module) {
    absl::StrAppend(out, module.module_name);
  });
}

std::string PresetsDataView::GetFunctionCountList(const std::vector<ModuleView>& modules) const {
  return absl::StrJoin(modules, "\n", [](std::string* out, const ModuleView& module) {
    absl::StrAppend(out, module.function_count);
  });
}

const std::vector<DataView::Column>& PresetsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnLoadState] = {kLoadableColumnName, kLoadableColumnWidth,
                                 SortingOrder::kAscending};
    columns[kColumnSessionName] = {kPresetColumnName, kPresetColumnWidth, SortingOrder::kAscending};
    columns[kColumnModules] = {kModulesColumnName, kModulesColumnWidth, SortingOrder::kAscending};
    columns[kColumnFunctionCount] = {kHookedFunctionsColumnName, kHookedFunctionsColumnWidth,
                                     SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string PresetsDataView::GetValue(int row, int column) {
  const std::shared_ptr<PresetFile>& preset = GetPreset(row);

  switch (column) {
    case kColumnLoadState:
      return GetLoadStateString(preset);
    case kColumnSessionName:
      return Path::GetFileName(preset->file_name());
    case kColumnModules:
      return GetModulesList(GetModules(row));
    case kColumnFunctionCount:
      return GetFunctionCountList(GetModules(row));
    default:
      return "";
  }
}

std::string PresetsDataView::GetToolTip(int row, int /*column*/) {
  const PresetFile& preset = *GetPreset(row);
  return preset.file_name();
}

void PresetsDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnLoadState:
      sorter = [&](int a, int b) {
        return OrbitUtils::Compare(GOrbitApp->GetPresetLoadState(presets_[a]).state,
                                   GOrbitApp->GetPresetLoadState(presets_[b]).state, ascending);
      };
      break;
    case kColumnSessionName:
      sorter = [&](int a, int b) {
        return OrbitUtils::Compare(presets_[a]->file_name(), presets_[b]->file_name(), ascending);
      };
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

const std::string PresetsDataView::kMenuActionLoad = "Load Preset";
const std::string PresetsDataView::kMenuActionDelete = "Delete Preset";

std::vector<std::string> PresetsDataView::GetContextMenu(int clicked_index,
                                                         const std::vector<int>& selected_indices) {
  std::vector<std::string> menu;
  // Note that the UI already enforces a single selection.
  if (selected_indices.size() == 1) {
    Append(menu, {kMenuActionLoad, kMenuActionDelete});
  }
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void PresetsDataView::OnContextMenu(const std::string& action, int menu_index,
                                    const std::vector<int>& item_indices) {
  if (action == kMenuActionLoad) {
    if (item_indices.size() != 1) {
      return;
    }
    const std::shared_ptr<PresetFile>& preset = GetPreset(item_indices[0]);
    GOrbitApp->LoadPreset(preset);

  } else if (action == kMenuActionDelete) {
    if (item_indices.size() != 1) {
      return;
    }
    int row = item_indices[0];
    const std::shared_ptr<PresetFile>& preset = GetPreset(row);
    const std::string& filename = preset->file_name();
    int ret = remove(filename.c_str());
    if (ret == 0) {
      presets_.erase(presets_.begin() + indices_[row]);
      OnDataChanged();
    } else {
      ERROR("Deleting preset \"%s\": %s", filename, SafeStrerror(errno));
      GOrbitApp->SendErrorToUi("Error deleting preset",
                               absl::StrFormat("Could not delete preset \"%s\".", filename));
    }

  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void PresetsDataView::DoFilter() {
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(ToLower(filter_), ' ');

  for (size_t i = 0; i < presets_.size(); ++i) {
    const PresetFile& preset = *presets_[i];
    std::string name = Path::GetFileName(ToLower(preset.file_name()));

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (!(name.find(filter_token) != std::string::npos)) {
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

void PresetsDataView::OnDataChanged() {
  indices_.resize(presets_.size());
  modules_.resize(presets_.size());
  for (size_t i = 0; i < presets_.size(); ++i) {
    indices_[i] = i;
    std::vector<ModuleView> modules;
    for (const auto& pair : presets_[i]->preset_info().path_to_module()) {
      modules.push_back(
          ModuleView(Path::GetFileName(pair.first), pair.second.function_hashes_size()));
    }
    modules_[i] = std::move(modules);
  }

  DataView::OnDataChanged();
}

bool PresetsDataView::GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                      unsigned char& green, unsigned char& blue) {
  const std::shared_ptr<PresetFile> preset = GetPreset(row);
  PresetLoadState load_state = GOrbitApp->GetPresetLoadState(preset);
  load_state.GetDisplayColor(red, green, blue);
  return true;
}

void PresetsDataView::SetPresets(const std::vector<std::shared_ptr<PresetFile> >& presets) {
  presets_ = presets;
  OnDataChanged();
}

const std::shared_ptr<PresetFile>& PresetsDataView::GetPreset(unsigned int row) const {
  return presets_[indices_[row]];
}
const std::vector<PresetsDataView::ModuleView>& PresetsDataView::GetModules(uint32_t row) const {
  return modules_[indices_[row]];
}
