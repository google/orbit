// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/FunctionsDataView.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionUtils.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "CompareAscendingOrDescending.h"
#include "DataViews/AppInterface.h"
#include "DataViews/DataViewType.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadPool.h"

using orbit_client_data::CaptureData;
using orbit_client_data::ModuleData;
using orbit_client_data::ProcessData;
using orbit_client_protos::FunctionInfo;

namespace orbit_data_views {

FunctionsDataView::FunctionsDataView(AppInterface* app, orbit_base::ThreadPool* thread_pool)
    : DataView(DataViewType::kFunctions, app), thread_pool_{thread_pool} {}

const std::string FunctionsDataView::kUnselectedFunctionString = "";
const std::string FunctionsDataView::kSelectedFunctionString = "✓";
const std::string FunctionsDataView::kFrameTrackString = "F";

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

  const CaptureData& capture_data = app->GetCaptureData();
  std::optional<uint64_t> instrumented_function_id =
      capture_data.FindInstrumentedFunctionIdSlow(function);

  return instrumented_function_id &&
         app->HasFrameTrackInCaptureData(instrumented_function_id.value());
}

std::string FunctionsDataView::BuildSelectedColumnsString(AppInterface* app,
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
      return BuildSelectedColumnsString(app_, function);
    case kColumnName:
      return orbit_client_data::function_utils::GetDisplayName(function);
    case kColumnSize:
      return absl::StrFormat("%lu", function.size());
    case kColumnModule:
      return orbit_client_data::function_utils::GetLoadedModuleName(function);
    case kColumnAddressInModule:
      return absl::StrFormat("%#x", function.address());
    default:
      return "";
  }
}

#define ORBIT_FUNC_SORT(Member)                                                                   \
  [&](int a, int b) {                                                                             \
    return CompareAscendingOrDescending(functions_[a]->Member, functions_[b]->Member, ascending); \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                                            \
  [&](int a, int b) {                                                                           \
    return CompareAscendingOrDescending(Func(*functions_[a]), Func(*functions_[b]), ascending); \
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
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(app_->IsFunctionSelected);
      break;
    case kColumnName:
      sorter = ORBIT_CUSTOM_FUNC_SORT(orbit_client_data::function_utils::GetDisplayName);
      break;
    case kColumnSize:
      sorter = ORBIT_FUNC_SORT(size());
      break;
    case kColumnModule:
      sorter = ORBIT_CUSTOM_FUNC_SORT(orbit_client_data::function_utils::GetLoadedModuleName);
      break;
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

std::vector<std::vector<std::string>> FunctionsDataView::GetContextMenuWithGrouping(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_enable_frame_track = false;
  bool enable_disable_frame_track = false;

  for (int index : selected_indices) {
    const FunctionInfo& function = *GetFunctionInfoFromRow(index);
    enable_select |= !app_->IsFunctionSelected(function) &&
                     orbit_client_data::function_utils::IsFunctionSelectable(function);
    enable_unselect |= app_->IsFunctionSelected(function);
    enable_enable_frame_track |= !app_->IsFrameTrackEnabled(function);
    enable_disable_frame_track |= app_->IsFrameTrackEnabled(function);
  }

  std::vector<std::string> action_group;
  if (enable_select) action_group.emplace_back(std::string{kMenuActionSelect});
  if (enable_unselect) action_group.emplace_back(std::string{kMenuActionUnselect});
  action_group.emplace_back(std::string{kMenuActionDisassembly});
  action_group.emplace_back(std::string{kMenuActionSourceCode});
  if (enable_enable_frame_track) {
    action_group.emplace_back(std::string{kMenuActionEnableFrameTrack});
  }
  if (enable_disable_frame_track) {
    action_group.emplace_back(std::string{kMenuActionDisableFrameTrack});
  }

  std::vector<std::vector<std::string>> menu =
      DataView::GetContextMenuWithGrouping(clicked_index, selected_indices);
  menu.insert(menu.begin(), action_group);

  return menu;
}

void FunctionsDataView::DoFilter() {
  filter_tokens_ = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  const size_t number_of_threads_available = thread_pool_->GetPoolSize();
  constexpr size_t kNumberOfTasksPerThread = 7;
  const size_t target_number_of_tasks = kNumberOfTasksPerThread * number_of_threads_available;

  constexpr size_t kMinimumNumberOfFunctionsPerTask = 512;
  const size_t number_of_functions_per_task =
      std::max(kMinimumNumberOfFunctionsPerTask, functions_.size() / target_number_of_tasks);
  const size_t number_of_tasks_needed =
      functions_.size() / number_of_functions_per_task +
      ((functions_.size() % number_of_functions_per_task) > 0 ? 1 : 0);

  std::vector<orbit_base::Future<std::vector<uint64_t>>> filtered_indices_per_thread;
  filtered_indices_per_thread.reserve(number_of_tasks_needed);

  for (size_t task_idx = 0; task_idx < number_of_tasks_needed; ++task_idx) {
    const size_t begin = task_idx * number_of_functions_per_task;
    const size_t end = std::min((task_idx + 1) * number_of_functions_per_task, functions_.size());

    filtered_indices_per_thread.emplace_back(thread_pool_->Schedule([begin, end, this]() {
      std::vector<uint64_t> indices_of_matches;

      for (size_t index = begin; index < end; ++index) {
        const FunctionInfo* function = functions_[index];
        std::string name =
            absl::AsciiStrToLower(orbit_client_data::function_utils::GetDisplayName(*function));
        std::string module = orbit_client_data::function_utils::GetLoadedModuleName(*function);

        const auto is_token_found = [&name, &module](const std::string& token) {
          return name.find(token) != std::string::npos || module.find(token) != std::string::npos;
        };

        if (std::all_of(filter_tokens_.begin(), filter_tokens_.end(), is_token_found)) {
          indices_of_matches.push_back(index);
        }
      }

      return indices_of_matches;
    }));
  }

  std::vector<std::vector<uint64_t>> filtered_indices =
      orbit_base::JoinFutures(absl::MakeConstSpan(filtered_indices_per_thread)).Get();

  std::vector<uint64_t> indices;
  for (const auto& indices_from_one_thread : filtered_indices) {
    indices.insert(indices.end(), indices_from_one_thread.begin(), indices_from_one_thread.end());
  }
  indices_ = std::move(indices);
}

void FunctionsDataView::AddFunctions(
    std::vector<const orbit_client_protos::FunctionInfo*> functions) {
  functions_.insert(functions_.end(), functions.begin(), functions.end());
  indices_.resize(functions_.size());
  for (size_t i = 0; i < indices_.size(); ++i) {
    indices_[i] = i;
  }
  OnDataChanged();
}

void FunctionsDataView::ClearFunctions() {
  functions_.clear();
  OnDataChanged();
}

}  // namespace orbit_data_views