// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/FunctionsDataView.h"

#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "DataViews/AppInterface.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"
#include "OrbitBase/Chunk.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/TaskGroup.h"

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;

namespace orbit_data_views {

FunctionsDataView::FunctionsDataView(AppInterface* app) : DataView(DataViewType::kFunctions, app) {}

const std::string FunctionsDataView::kUnselectedFunctionString = "";
const std::string FunctionsDataView::kSelectedFunctionString = "H";
const std::string FunctionsDataView::kFrameTrackString = "F";
const std::string FunctionsDataView::kApiScopeTypeString = "MS";
const std::string FunctionsDataView::kApiScopeAsyncTypeString = "MA";
const std::string FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString = "D";

const std::vector<DataView::Column>& FunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnName] = {"Function", .65f, SortingOrder::kAscending};
    columns[kColumnSize] = {"Size", .0f, SortingOrder::kAscending};
    columns[kColumnModule] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnAddressInModule] = {"Address in module", .0f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

bool FunctionsDataView::ShouldShowSelectedFunctionIcon(AppInterface* app,
                                                       const FunctionInfo& function) {
  return app->IsFunctionSelected(function);
}

bool FunctionsDataView::ShouldShowFrameTrackIcon(AppInterface* app, const FunctionInfo& function) {
  if (app->IsFrameTrackEnabled(function)) {
    return true;
  }

  if (!app->HasCaptureData()) {
    return false;
  }

  std::optional<uint64_t> instrumented_function_id =
      app->GetCaptureData().FindFunctionIdSlow(function);

  return instrumented_function_id &&
         app->HasFrameTrackInCaptureData(instrumented_function_id.value());
}

std::string FunctionsDataView::BuildSelectedAndFrameTrackString(AppInterface* app,
                                                                const FunctionInfo& function) {
  std::string result = kUnselectedFunctionString;
  if (ShouldShowSelectedFunctionIcon(app, function)) {
    absl::StrAppend(&result, kSelectedFunctionString);
    if (ShouldShowFrameTrackIcon(app, function)) {
      absl::StrAppend(&result, " ", kFrameTrackString);
    }
  } else if (ShouldShowFrameTrackIcon(app, function)) {
    absl::StrAppend(&result, kFrameTrackString);
  }
  return result;
}

std::string FunctionsDataView::GetValue(int row, int column) {
  if (row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  const FunctionInfo& function = *GetFunctionInfoFromRow(row);

  switch (column) {
    case kColumnSelected:
      return BuildSelectedAndFrameTrackString(app_, function);
    case kColumnName:
      return function.pretty_name();
    case kColumnSize:
      return absl::StrFormat("%lu", function.size());
    case kColumnModule:
      return std::filesystem::path(function.module_path()).filename().string();
    case kColumnAddressInModule:
      return absl::StrFormat("%#x", function.address());
    default:
      return "";
  }
}

#define ORBIT_FUNC_SORT(Member)                                     \
  [&](uint64_t a, uint64_t b) {                                     \
    return orbit_data_views_internal::CompareAscendingOrDescending( \
        functions_[a]->Member, functions_[b]->Member, ascending);   \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                \
  [&](uint64_t a, uint64_t b) {                                     \
    return orbit_data_views_internal::CompareAscendingOrDescending( \
        Func(*functions_[a]), Func(*functions_[b]), ascending);     \
  }

void FunctionsDataView::DoSort() {
  // TODO(antonrohr): This sorting function can take a lot of time when a large
  // number of functions is used (several seconds). This function is currently
  // executed on the main thread and therefore freezes the UI and interrupts the
  // ssh watchdog signals that are sent to the service. Therefore this should
  // not be called on the main thread and as soon as this is done the watchdog
  // timeout should be rolled back from 25 seconds to 10 seconds in
  // OrbitService.h
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(uint64_t a, uint64_t b)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(app_->IsFunctionSelected);
      break;
    case kColumnName:
      sorter = ORBIT_FUNC_SORT(pretty_name());
      break;
    case kColumnSize:
      sorter = ORBIT_FUNC_SORT(size());
      break;
    case kColumnModule: {
      auto module_name = [](const orbit_client_data::FunctionInfo& function) {
        return std::filesystem::path(function.module_path()).filename().string();
      };
      sorter = ORBIT_CUSTOM_FUNC_SORT(module_name);
      break;
    }
    case kColumnAddressInModule:
      sorter = ORBIT_FUNC_SORT(address());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

DataView::ActionStatus FunctionsDataView::GetActionStatus(std::string_view action,
                                                          int clicked_index,
                                                          absl::Span<const int> selected_indices) {
  if (action == kMenuActionDisassembly || action == kMenuActionSourceCode) {
    return ActionStatus::kVisibleAndEnabled;
  }

  std::function<bool(const FunctionInfo&)> is_visible_action_enabled;
  if (action == kMenuActionSelect) {
    is_visible_action_enabled = [this](const FunctionInfo& function) {
      return !app_->IsFunctionSelected(function) && function.IsFunctionSelectable();
    };

  } else if (action == kMenuActionUnselect) {
    is_visible_action_enabled = [this](const FunctionInfo& function) {
      return app_->IsFunctionSelected(function);
    };

  } else if (action == kMenuActionEnableFrameTrack) {
    is_visible_action_enabled = [this](const FunctionInfo& function) {
      return !app_->IsFrameTrackEnabled(function);
    };

  } else if (action == kMenuActionDisableFrameTrack) {
    is_visible_action_enabled = [this](const FunctionInfo& function) {
      return app_->IsFrameTrackEnabled(function);
    };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  for (int index : selected_indices) {
    const FunctionInfo& function = *GetFunctionInfoFromRow(index);
    if (is_visible_action_enabled(function)) return ActionStatus::kVisibleAndEnabled;
  }
  return ActionStatus::kVisibleButDisabled;
}

void FunctionsDataView::DoFilter() {
  ORBIT_SCOPE(absl::StrFormat("FunctionsDataView::DoFilter [%u]", functions_.size()).c_str());
  filter_tokens_ = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  constexpr size_t kNumFunctionsPerTask = 1024;
  std::vector<absl::Span<const FunctionInfo*>> chunks =
      orbit_base::CreateChunksOfSize(functions_, kNumFunctionsPerTask);
  std::vector<std::vector<uint64_t>> task_results(chunks.size());
  orbit_base::TaskGroup task_group;

  for (size_t i = 0; i < chunks.size(); ++i) {
    task_group.AddTask([&chunk = chunks[i], &result = task_results[i], this]() {
      ORBIT_SCOPE("FunctionsDataView::DoFilter Task");
      for (const FunctionInfo*& function : chunk) {
        ORBIT_CHECK(function != nullptr);
        std::string name = absl::AsciiStrToLower(function->pretty_name());
        std::string module = absl::AsciiStrToLower(
            std::filesystem::path(function->module_path()).filename().string());

        const auto is_token_found = [&name, &module](std::string_view token) {
          return name.find(token) != std::string::npos || module.find(token) != std::string::npos;
        };

        if (std::all_of(filter_tokens_.begin(), filter_tokens_.end(), is_token_found)) {
          size_t function_index = &function - functions_.data();
          ORBIT_CHECK(function_index < functions_.size());
          result.push_back(function_index);
        }
      }
    });
  }

  task_group.Wait();
  indices_.clear();
  for (std::vector<uint64_t>& result : task_results) {
    indices_.insert(indices_.end(), result.begin(), result.end());
  }
}

void FunctionsDataView::AddFunctions(
    std::vector<const orbit_client_data::FunctionInfo*> functions) {
  functions_.insert(functions_.end(), functions.begin(), functions.end());
  OnDataChanged();
}

void FunctionsDataView::RemoveFunctionsOfModule(std::string_view module_path) {
  functions_.erase(std::remove_if(functions_.begin(), functions_.end(),
                                  [&module_path](const FunctionInfo* function_info) {
                                    return function_info->module_path() == module_path;
                                  }),
                   functions_.end());
  OnDataChanged();
}

void FunctionsDataView::ClearFunctions() {
  functions_.clear();
  OnDataChanged();
}

}  // namespace orbit_data_views
