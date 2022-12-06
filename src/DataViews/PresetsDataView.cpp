// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/PresetsDataView.h"

#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <errno.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <functional>

#include "DataViewUtils.h"
#include "DataViews/AppInterface.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"
#include "DataViews/PresetLoadState.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "PresetFile/PresetFile.h"
#include "QtUtils/MainThreadExecutorImpl.h"

using orbit_preset_file::PresetFile;

constexpr const char* kLoadableColumnName = "Loadable";
constexpr const char* kPresetColumnName = "Preset";
constexpr const char* kModulesColumnName = "Modules";
constexpr const char* kHookedFunctionsColumnName = "Hooked Functions";
constexpr const char* kDateModifiedColumnName = "Date Modified";

constexpr const float kLoadableColumnWidth = 0.14f;
constexpr const float kPresetColumnWidth = 0.34f;
constexpr const float kModulesColumnWidth = 0.20f;
constexpr const float kHookedFunctionsColumnWidth = 0.16f;
constexpr const float kDateModifiedColumnWidth = 0.16f;

namespace {

std::string GetLoadStatusAndStateString(orbit_data_views::AppInterface* app,
                                        const PresetFile& preset) {
  std::string_view load_status = preset.IsLoaded()
                                     ? orbit_data_views::PresetsDataView::kLoadedPresetPrefix
                                     : orbit_data_views::PresetsDataView::kNotLoadedPresetPrefix;
  orbit_data_views::PresetLoadState load_state = app->GetPresetLoadState(preset);
  return absl::StrCat(load_status, load_state.GetName());
}

std::string GetLoadStatusAndStateTooltip(orbit_data_views::AppInterface* app,
                                         const PresetFile& preset) {
  std::string_view load_status =
      preset.IsLoaded() ? orbit_data_views::PresetsDataView::kLoadedPresetTooltipSuffix
                        : orbit_data_views::PresetsDataView::kNotLoadedPresetTooltipSuffix;
  orbit_data_views::PresetLoadState load_state = app->GetPresetLoadState(preset);
  return absl::StrCat(load_state.GetName(), load_status);
}

std::string GetDateModifiedString(const PresetFile& preset) {
  auto datetime_or_error = orbit_base::GetFileDateModified(preset.file_path());
  if (datetime_or_error.has_error()) {
    ORBIT_ERROR("%s", datetime_or_error.error().message());
    return "";
  }

  return orbit_data_views::FormatShortDatetime(datetime_or_error.value());
}

}  // namespace

namespace orbit_data_views {

PresetsDataView::PresetsDataView(AppInterface* app)
    : DataView(DataViewType::kPresets, app),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {}

std::string PresetsDataView::GetModulesList(absl::Span<const ModuleView> modules) {
  return absl::StrJoin(modules, "\n", [](std::string* out, const ModuleView& module) {
    absl::StrAppend(out, module.module_name);
  });
}

std::string PresetsDataView::GetFunctionCountList(absl::Span<const ModuleView> modules) {
  return absl::StrJoin(modules, "\n", [](std::string* out, const ModuleView& module) {
    absl::StrAppend(out, module.function_count);
  });
}

std::string PresetsDataView::GetModuleAndFunctionCountList(absl::Span<const ModuleView> modules) {
  return absl::StrJoin(modules, "\n", [](std::string* out, const ModuleView& module) {
    absl::StrAppendFormat(out, "%s: %u function(s)", module.module_name, module.function_count);
  });
}

const std::vector<DataView::Column>& PresetsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnLoadState] = {kLoadableColumnName, kLoadableColumnWidth,
                                 SortingOrder::kAscending};
    columns[kColumnPresetName] = {kPresetColumnName, kPresetColumnWidth, SortingOrder::kAscending};
    columns[kColumnModules] = {kModulesColumnName, kModulesColumnWidth, SortingOrder::kAscending};
    columns[kColumnFunctionCount] = {kHookedFunctionsColumnName, kHookedFunctionsColumnWidth,
                                     SortingOrder::kAscending};
    columns[kColumnDateModified] = {kDateModifiedColumnName, kDateModifiedColumnWidth,
                                    SortingOrder::kDescending};
    return columns;
  }();
  return columns;
}

std::string PresetsDataView::GetValue(int row, int column) {
  const PresetFile& preset = GetPreset(row);

  switch (column) {
    case kColumnLoadState:
      return GetLoadStatusAndStateString(app_, preset);
    case kColumnPresetName:
      return preset.file_path().filename().string();
    case kColumnModules:
      return GetModulesList(GetModules(row));
    case kColumnFunctionCount:
      return GetFunctionCountList(GetModules(row));
    case kColumnDateModified:
      return GetDateModifiedString(preset);
    default:
      return "";
  }
}

std::string PresetsDataView::GetToolTip(int row, int column) {
  const PresetFile& preset = GetPreset(row);
  switch (column) {
    case kColumnLoadState:
      return GetLoadStatusAndStateTooltip(app_, preset);
    case kColumnPresetName:
      return absl::StrCat(preset.file_path().string(),
                          app_->GetPresetLoadState(preset).state == PresetLoadState::kNotLoadable
                              ? "<br/><br/><i>None of the modules in the preset can be loaded.</i>"
                              : "");
    case kColumnModules:
      [[fallthrough]];
    case kColumnFunctionCount:
      return GetModuleAndFunctionCountList(GetModules(row));
    case kColumnDateModified:
      [[fallthrough]];
    default:
      return DataView::GetToolTip(row, column);
  }
}

void PresetsDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(uint64_t, uint64_t)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnLoadState:
      sorter = [&](uint64_t a, uint64_t b) {
        return orbit_data_views_internal::CompareAscendingOrDescending(
            app_->GetPresetLoadState(presets_[a]).state,
            app_->GetPresetLoadState(presets_[b]).state, ascending);
      };
      break;
    case kColumnPresetName:
      sorter = [&](uint64_t a, uint64_t b) {
        return orbit_data_views_internal::CompareAscendingOrDescending(
            presets_[a].file_path(), presets_[b].file_path(), ascending);
      };
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

DataView::ActionStatus PresetsDataView::GetActionStatus(std::string_view action, int clicked_index,
                                                        absl::Span<const int> selected_indices) {
  // Note that the UI already enforces a single selection.
  ORBIT_CHECK(selected_indices.size() == 1);

  if (action == kMenuActionDeletePreset || action == kMenuActionShowInExplorer) {
    return ActionStatus::kVisibleAndEnabled;

  } else if (action == kMenuActionLoadPreset) {
    const PresetFile& preset = GetPreset(selected_indices[0]);
    return app_->GetPresetLoadState(preset).state == PresetLoadState::kNotLoadable
               ? ActionStatus::kVisibleButDisabled
               : ActionStatus::kVisibleAndEnabled;

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }
}

void PresetsDataView::OnLoadPresetRequested(absl::Span<const int> selection) {
  PresetFile& preset = GetMutablePreset(selection[0]);
  (void)app_->LoadPreset(preset).ThenIfSuccess(main_thread_executor_.get(),
                                               [this, preset_file_path = preset.file_path()]() {
                                                 OnLoadPresetSuccessful(preset_file_path);
                                               });
}

void PresetsDataView::OnDeletePresetRequested(absl::Span<const int> selection) {
  int row = selection[0];
  const PresetFile& preset = GetPreset(row);
  const std::string& filename = preset.file_path().string();
  int ret = remove(filename.c_str());
  if (ret == 0) {
    presets_.erase(presets_.begin() + indices_[row]);
    OnDataChanged();
  } else {
    ORBIT_ERROR("Deleting preset \"%s\": %s", filename, SafeStrerror(errno));
    app_->SendErrorToUi("Error deleting preset",
                        absl::StrFormat("Could not delete preset \"%s\".", filename));
  }
}

void PresetsDataView::OnShowInExplorerRequested(absl::Span<const int> selection) {
  const PresetFile& preset = GetPreset(selection[0]);
  app_->ShowPresetInExplorer(preset);
}

void PresetsDataView::OnDoubleClicked(int index) {
  PresetFile& preset = GetMutablePreset(index);
  if (app_->GetPresetLoadState(preset).state != PresetLoadState::kNotLoadable) {
    (void)app_->LoadPreset(preset).ThenIfSuccess(main_thread_executor_.get(),
                                                 [this, preset_file_path = preset.file_path()]() {
                                                   OnLoadPresetSuccessful(preset_file_path);
                                                 });
  }
}

void PresetsDataView::OnLoadPresetSuccessful(const std::filesystem::path& preset_file_path) {
  const auto it = std::find_if(presets_.begin(), presets_.end(), [&](const PresetFile& preset) {
    return preset.file_path() == preset_file_path;
  });
  if (it != presets_.end()) it->SetIsLoaded(true);
}

void PresetsDataView::DoFilter() {
  std::vector<uint64_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (size_t i = 0; i < presets_.size(); ++i) {
    const PresetFile& preset = presets_[i];
    std::string name = absl::AsciiStrToLower(preset.file_path().filename().string());

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (name.find(filter_token) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  indices_ = std::move(indices);
}

void PresetsDataView::OnDataChanged() {
  indices_.resize(presets_.size());
  modules_.resize(presets_.size());
  for (size_t i = 0; i < presets_.size(); ++i) {
    indices_[i] = i;
    std::vector<ModuleView> modules;
    for (const auto& module_path : presets_[i].GetModulePaths()) {
      size_t num_functions = presets_[i].GetNumberOfFunctionsForModule(module_path);
      modules.emplace_back(module_path.filename().string(), num_functions);
    }
    modules_[i] = std::move(modules);
  }

  DataView::OnDataChanged();
}

bool PresetsDataView::GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                      unsigned char& green, unsigned char& blue) {
  const PresetFile& preset = GetPreset(row);
  PresetLoadState load_state = app_->GetPresetLoadState(preset);
  load_state.GetDisplayColor(red, green, blue);
  return true;
}

void PresetsDataView::SetPresets(std::vector<PresetFile> presets) {
  presets_ = std::move(presets);
  OnDataChanged();
}

const PresetFile& PresetsDataView::GetPreset(unsigned int row) const {
  return presets_[indices_[row]];
}

PresetFile& PresetsDataView::GetMutablePreset(unsigned int row) { return presets_[indices_[row]]; }

const std::vector<PresetsDataView::ModuleView>& PresetsDataView::GetModules(uint32_t row) const {
  return modules_[indices_[row]];
}

}  // namespace orbit_data_views