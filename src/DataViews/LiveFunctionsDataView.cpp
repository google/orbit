// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/LiveFunctionsDataView.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/time/time.h>
#include <llvm/Demangle/Demangle.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionUtils.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ScopeIdConstants.h"
#include "ClientData/ScopeStats.h"
#include "ClientProtos/capture_data.pb.h"
#include "CompareAscendingOrDescending.h"
#include "DataViews/DataView.h"
#include "DataViews/DataViewType.h"
#include "DataViews/FunctionsDataView.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"
#include "Statistics/Histogram.h"

using orbit_client_data::CaptureData;
using orbit_client_data::ModuleData;
using orbit_client_data::ScopeStats;

using orbit_client_data::ModuleManager;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::InstrumentedFunction;

namespace orbit_data_views {

LiveFunctionsDataView::LiveFunctionsDataView(
    LiveFunctionsInterface* live_functions, AppInterface* app,
    orbit_metrics_uploader::MetricsUploader* metrics_uploader)
    : DataView(DataViewType::kLiveFunctions, app),
      live_functions_(live_functions),
      selected_function_id_(orbit_grpc_protos::kInvalidFunctionId),
      metrics_uploader_(metrics_uploader) {
  update_period_ms_ = 300;
}

const std::vector<DataView::Column>& LiveFunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnName] = {"Function", .4f, SortingOrder::kAscending};
    columns[kColumnCount] = {"Count", .0f, SortingOrder::kDescending};
    columns[kColumnTimeTotal] = {"Total", .075f, SortingOrder::kDescending};
    columns[kColumnTimeAvg] = {"Avg", .075f, SortingOrder::kDescending};
    columns[kColumnTimeMin] = {"Min", .075f, SortingOrder::kDescending};
    columns[kColumnTimeMax] = {"Max", .075f, SortingOrder::kDescending};
    columns[kColumnStdDev] = {"Std Dev", .075f, SortingOrder::kDescending};
    columns[kColumnModule] = {"Module", .1f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Address", .1f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string LiveFunctionsDataView::GetValue(int row, int column) {
  if (!app_->HasCaptureData()) {
    return "";
  }
  if (row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  const uint64_t function_id = GetInstrumentedFunctionId(row);
  const ScopeStats& stats = app_->GetCaptureData().GetScopeStatsOrDefault(function_id);

  const FunctionInfo& function = *GetFunctionInfoFromRow(row);
  switch (column) {
    case kColumnSelected:
      return FunctionsDataView::BuildSelectedColumnsString(app_, function);
    case kColumnName:
      return orbit_client_data::function_utils::GetDisplayName(function);
    case kColumnCount:
      return absl::StrFormat("%lu", stats.count());
    case kColumnTimeTotal:
      return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats.total_time_ns()));
    case kColumnTimeAvg:
      return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats.ComputeAverageTimeNs()));
    case kColumnTimeMin:
      return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats.min_ns()));
    case kColumnTimeMax:
      return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats.max_ns()));
    case kColumnStdDev:
      return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats.std_dev_ns()));
    case kColumnModule:
      return function.module_path();
    case kColumnAddress:
      return absl::StrFormat("%#x", function.address());
    default:
      return "";
  }
}

std::vector<int> LiveFunctionsDataView::GetVisibleSelectedIndices() {
  std::optional<int> visible_selected_index = GetRowFromFunctionId(selected_function_id_);
  if (!visible_selected_index.has_value()) return {};
  return {visible_selected_index.value()};
}

void LiveFunctionsDataView::UpdateHighlightedFunctionId(const std::vector<int>& rows) {
  app_->DeselectTimer();
  if (rows.empty()) {
    app_->SetHighlightedFunctionId(orbit_grpc_protos::kInvalidFunctionId);
  } else {
    app_->SetHighlightedFunctionId(GetInstrumentedFunctionId(rows[0]));
  }
}

void LiveFunctionsDataView::UpdateSelectedFunctionId() {
  selected_function_id_ = app_->GetHighlightedFunctionId();
}

void LiveFunctionsDataView::UpdateHistogramWithIndices(
    const std::vector<int>& visible_selected_indices) {
  std::vector<uint64_t> function_ids;
  std::transform(std::begin(visible_selected_indices), std::end(visible_selected_indices),
                 std::back_inserter(function_ids),
                 [this](const int index) { return indices_[index]; });

  UpdateHistogramWithScopeIds(function_ids);
}

void LiveFunctionsDataView::UpdateHistogramWithScopeIds(const std::vector<uint64_t>& scope_ids) {
  const std::vector<uint64_t>* timer_durations =
      (app_->HasCaptureData() && !scope_ids.empty())
          ? app_->GetCaptureData().GetSortedTimerDurationsForScopeId(scope_ids[0])
          : nullptr;

  if (timer_durations == nullptr) {
    app_->ShowHistogram(nullptr, "", orbit_client_data::kInvalidScopeId);
    return;
  }

  const uint64_t scope_id = scope_ids[0];
  const std::string& scope_name = app_->GetCaptureData().GetScopeName(scope_id);
  app_->ShowHistogram(timer_durations, scope_name, scope_id);
}

void LiveFunctionsDataView::OnSelect(const std::vector<int>& rows) {
  UpdateHighlightedFunctionId(rows);
  UpdateSelectedFunctionId();

  UpdateHistogramWithIndices(GetVisibleSelectedIndices());
}

#define ORBIT_FUNC_SORT(Member)                                                         \
  [&](uint64_t a, uint64_t b) {                                                         \
    return CompareAscendingOrDescending(functions.at(a).Member, functions.at(b).Member, \
                                        ascending);                                     \
  }
#define ORBIT_STAT_SORT(Member)                                                     \
  [&](uint64_t a, uint64_t b) {                                                     \
    const ScopeStats& stats_a = app_->GetCaptureData().GetScopeStatsOrDefault(a);   \
    const ScopeStats& stats_b = app_->GetCaptureData().GetScopeStatsOrDefault(b);   \
    return CompareAscendingOrDescending(stats_a.Member, stats_b.Member, ascending); \
  }
#define ORBIT_CUSTOM_FUNC_SORT(Func)                                                              \
  [&](uint64_t a, uint64_t b) {                                                                   \
    return CompareAscendingOrDescending(Func(functions.at(a)), Func(functions.at(b)), ascending); \
  }

void LiveFunctionsDataView::DoSort() {
  if (!app_->HasCaptureData()) {
    ORBIT_CHECK(functions_.empty());
    return;
  }
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(uint64_t a, uint64_t b)> sorter = nullptr;

  const absl::flat_hash_map<uint64_t, FunctionInfo>& functions = functions_;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(app_->IsFunctionSelected);
      break;
    case kColumnName:
      sorter = ORBIT_CUSTOM_FUNC_SORT(orbit_client_data::function_utils::GetDisplayName);
      break;
    case kColumnCount:
      sorter = ORBIT_STAT_SORT(count());
      break;
    case kColumnTimeTotal:
      sorter = ORBIT_STAT_SORT(total_time_ns());
      break;
    case kColumnTimeAvg:
      sorter = ORBIT_STAT_SORT(ComputeAverageTimeNs());
      break;
    case kColumnTimeMin:
      sorter = ORBIT_STAT_SORT(min_ns());
      break;
    case kColumnTimeMax:
      sorter = ORBIT_STAT_SORT(max_ns());
      break;
    case kColumnStdDev:
      sorter = ORBIT_STAT_SORT(std_dev_ns());
      break;
    case kColumnModule:
      sorter = ORBIT_CUSTOM_FUNC_SORT(orbit_client_data::function_utils::GetLoadedModuleName);
      break;
    case kColumnAddress:
      sorter = ORBIT_FUNC_SORT(address());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

DataView::ActionStatus LiveFunctionsDataView::GetActionStatus(
    std::string_view action, int clicked_index, const std::vector<int>& selected_indices) {
  if (action == kMenuActionExportEventsToCsv) return ActionStatus::kVisibleAndEnabled;

  const CaptureData& capture_data = app_->GetCaptureData();
  if (action == kMenuActionJumpToFirst || action == kMenuActionJumpToLast ||
      action == kMenuActionJumpToMin || action == kMenuActionJumpToMax) {
    if (selected_indices.size() != 1) return ActionStatus::kVisibleButDisabled;

    uint64_t instrumented_function_id = GetInstrumentedFunctionId(selected_indices[0]);
    const ScopeStats& stats = capture_data.GetScopeStatsOrDefault(instrumented_function_id);
    if (stats.count() == 0) return ActionStatus::kVisibleButDisabled;

    return ActionStatus::kVisibleAndEnabled;
  }

  bool is_capture_connected = app_->IsCaptureConnected(capture_data);
  if (action == kMenuActionDisassembly || action == kMenuActionSourceCode) {
    return is_capture_connected ? ActionStatus::kVisibleAndEnabled
                                : ActionStatus::kVisibleButDisabled;
  }

  if ((action == kMenuActionSelect || action == kMenuActionUnselect) && !is_capture_connected) {
    return ActionStatus::kVisibleButDisabled;
  }

  std::function<bool(uint64_t, const FunctionInfo&)> is_visible_action_enabled;
  if (action == kMenuActionSelect) {
    is_visible_action_enabled = [this](uint64_t /*instrumented_function_id*/,
                                       const FunctionInfo& instrumented_function) {
      return !app_->IsFunctionSelected(instrumented_function) &&
             orbit_client_data::function_utils::IsFunctionSelectable(instrumented_function);
    };

  } else if (action == kMenuActionUnselect) {
    is_visible_action_enabled = [this](uint64_t /*instrumented_function_id*/,
                                       const FunctionInfo& instrumented_function) {
      return app_->IsFunctionSelected(instrumented_function);
    };

  } else if (action == kMenuActionEnableFrameTrack) {
    is_visible_action_enabled = [this, &is_capture_connected, &capture_data](
                                    uint64_t instrumented_function_id,
                                    const FunctionInfo& instrumented_function) {
      return is_capture_connected ? !app_->IsFrameTrackEnabled(instrumented_function)
                                  : !capture_data.IsFrameTrackEnabled(instrumented_function_id);
    };

  } else if (action == kMenuActionDisableFrameTrack) {
    is_visible_action_enabled = [this, &is_capture_connected, &capture_data](
                                    uint64_t instrumented_function_id,
                                    const FunctionInfo& instrumented_function) {
      return is_capture_connected ? app_->IsFrameTrackEnabled(instrumented_function)
                                  : capture_data.IsFrameTrackEnabled(instrumented_function_id);
    };

  } else if (action == kMenuActionAddIterator) {
    is_visible_action_enabled = [&capture_data](uint64_t instrumented_function_id,
                                                const FunctionInfo& /*instrumented_function*/) {
      const ScopeStats& stats = capture_data.GetScopeStatsOrDefault(instrumented_function_id);
      // We need at least one function call to a function so that adding iterators makes sense.
      return stats.count() > 0;
    };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  for (int index : selected_indices) {
    uint64_t instrumented_function_id = GetInstrumentedFunctionId(index);
    const FunctionInfo& instrumented_function = *GetFunctionInfoFromRow(index);
    if (is_visible_action_enabled(instrumented_function_id, instrumented_function)) {
      return ActionStatus::kVisibleAndEnabled;
    }
  }
  return ActionStatus::kVisibleButDisabled;
}

void LiveFunctionsDataView::OnIteratorRequested(const std::vector<int>& selection) {
  for (int i : selection) {
    uint64_t instrumented_function_id = GetInstrumentedFunctionId(i);
    const FunctionInfo* instrumented_function = GetFunctionInfoFromRow(i);
    const ScopeStats& stats =
        app_->GetCaptureData().GetScopeStatsOrDefault(instrumented_function_id);
    if (stats.count() > 0) {
      live_functions_->AddIterator(instrumented_function_id, instrumented_function);
      metrics_uploader_->SendLogEvent(orbit_metrics_uploader::OrbitLogEvent::ORBIT_ITERATOR_ADD);
    }
  }
}

void LiveFunctionsDataView::OnJumpToRequested(const std::string& action,
                                              const std::vector<int>& selection) {
  ORBIT_CHECK(selection.size() == 1);
  auto function_id = GetInstrumentedFunctionId(selection[0]);
  if (action == kMenuActionJumpToFirst) {
    app_->JumpToTimerAndZoom(function_id, AppInterface::JumpToTimerMode::kFirst);
  } else if (action == kMenuActionJumpToLast) {
    app_->JumpToTimerAndZoom(function_id, AppInterface::JumpToTimerMode::kLast);
  } else if (action == kMenuActionJumpToMin) {
    app_->JumpToTimerAndZoom(function_id, AppInterface::JumpToTimerMode::kMin);
  } else if (action == kMenuActionJumpToMax) {
    app_->JumpToTimerAndZoom(function_id, AppInterface::JumpToTimerMode::kMax);
  }
}

void LiveFunctionsDataView::OnExportEventsToCsvRequested(const std::vector<int>& selection) {
  auto send_error = [&](const std::string& error_msg) {
    app_->SendErrorToUi("Export all events to CSV", error_msg);
  };

  std::string file_path = app_->GetSaveFile(".csv");
  if (file_path.empty()) return;

  ErrorMessageOr<orbit_base::unique_fd> result = orbit_base::OpenFileForWriting(file_path);
  if (result.has_error()) {
    send_error(
        absl::StrFormat("Failed to open \"%s\" file: %s", file_path, result.error().message()));
    return;
  }
  const orbit_base::unique_fd& fd = result.value();

  // Write header line
  constexpr const char* kFieldSeparator = ",";
  constexpr const char* kLineSeparator = "\r\n";
  constexpr size_t kNumColumns = 5;
  const std::array<std::string, kNumColumns> kNames{"Name", "Thread", "Start", "End",
                                                    "Duration (ns)"};
  std::string header_line = absl::StrJoin(
      kNames, kFieldSeparator,
      [](std::string* out, const std::string& name) { out->append(FormatValueForCsv(name)); });
  header_line.append(kLineSeparator);
  auto write_result = orbit_base::WriteFully(fd, header_line);
  if (write_result.has_error()) {
    send_error(
        absl::StrFormat("Error writing to \"%s\": %s", file_path, write_result.error().message()));
    return;
  }

  for (int row : selection) {
    const FunctionInfo& function = *GetFunctionInfoFromRow(row);
    std::string function_name = orbit_client_data::function_utils::GetDisplayName(function);

    const uint64_t function_id = GetInstrumentedFunctionId(row);
    const CaptureData& capture_data = app_->GetCaptureData();
    for (const TimerInfo* timer : app_->GetAllTimersForHookedFunction(function_id)) {
      std::string line;
      line.append(FormatValueForCsv(function_name));
      line.append(kFieldSeparator);
      line.append(FormatValueForCsv(absl::StrFormat(
          "%s [%lu]", capture_data.GetThreadName(timer->thread_id()), timer->thread_id())));
      line.append(kFieldSeparator);
      line.append(FormatValueForCsv(absl::StrFormat("%lu", timer->start())));
      line.append(kFieldSeparator);
      line.append(FormatValueForCsv(absl::StrFormat("%lu", timer->end())));
      line.append(kFieldSeparator);
      line.append(FormatValueForCsv(absl::StrFormat("%lu", timer->end() - timer->start())));
      line.append(kLineSeparator);

      auto write_result = orbit_base::WriteFully(fd, line);
      if (write_result.has_error()) {
        send_error(absl::StrFormat("Error writing to \"%s\": %s", file_path,
                                   write_result.error().message()));
        return;
      }
    }
  }
}

void LiveFunctionsDataView::DoFilter() {
  if (!app_->HasCaptureData()) {
    ORBIT_CHECK(functions_.empty());
    return;
  }
  std::vector<uint64_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (const auto& entry : functions_) {
    uint64_t function_id = entry.first;
    const FunctionInfo& function = entry.second;
    std::string name =
        absl::AsciiStrToLower(orbit_client_data::function_utils::GetDisplayName(function));

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (name.find(filter_token) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(function_id);
    }
  }

  indices_ = std::move(indices);

  // Filter drawn textboxes
  absl::flat_hash_set<uint64_t> visible_function_ids;
  for (size_t i = 0; i < indices_.size(); ++i) {
    visible_function_ids.insert(GetInstrumentedFunctionId(i));
  }
  app_->SetVisibleFunctionIds(std::move(visible_function_ids));
}

void LiveFunctionsDataView::AddFunction(uint64_t function_id,
                                        orbit_client_protos::FunctionInfo function_info) {
  functions_.insert_or_assign(function_id, std::move(function_info));
  indices_.push_back(function_id);
}

void LiveFunctionsDataView::OnDataChanged() {
  UpdateHistogramWithScopeIds({});
  functions_.clear();
  indices_.clear();

  if (!app_->HasCaptureData()) {
    DataView::OnDataChanged();
    return;
  }

  const absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>&
      instrumented_functions = app_->GetCaptureData().instrumented_functions();
  for (const auto& [function_id, instrumented_function] : instrumented_functions) {
    const ModuleManager* module_manager = app_->GetModuleManager();
    const FunctionInfo* function_info_from_capture_data =
        orbit_client_data::FindFunctionByModulePathBuildIdAndOffset(
            *module_manager, instrumented_function.file_path(),
            instrumented_function.file_build_id(), instrumented_function.file_offset());

    // This could happen because module has not yet been updated, it also
    // happens when loading capture. In which case we will try to construct
    // function info from instrumented function
    std::optional<FunctionInfo> function_info;
    if (function_info_from_capture_data == nullptr) {
      function_info = CreateFunctionInfoFromInstrumentedFunction(instrumented_function);
    } else {
      function_info = *function_info_from_capture_data;
    }

    if (!function_info.has_value()) {
      return;
    }

    AddFunction(function_id, std::move(*function_info));
  }

  DataView::OnDataChanged();
}

void LiveFunctionsDataView::OnTimer() {
  if (app_->IsCapturing()) {
    OnSort(sorting_column_, {});
  }
}

void LiveFunctionsDataView::OnRefresh(const std::vector<int>& visible_selected_indices,
                                      const RefreshMode& mode) {
  if (mode == RefreshMode::kOnFilter || mode == RefreshMode::kOnSort) {
    UpdateHighlightedFunctionId(visible_selected_indices);
  }
  if (mode != RefreshMode::kOnSort) {
    UpdateHistogramWithIndices(visible_selected_indices);
  }
}

uint64_t LiveFunctionsDataView::GetInstrumentedFunctionId(uint32_t row) const {
  ORBIT_CHECK(row < indices_.size());
  return indices_[row];
}

const FunctionInfo* LiveFunctionsDataView::GetFunctionInfoFromRow(int row) {
  ORBIT_CHECK(static_cast<unsigned int>(row) < indices_.size());
  ORBIT_CHECK(functions_.find(indices_[row]) != functions_.end());
  return &functions_.at(indices_[row]);
}

std::optional<int> LiveFunctionsDataView::GetRowFromFunctionId(uint64_t function_id) {
  for (size_t function_row = 0; function_row < indices_.size(); function_row++) {
    if (indices_[function_row] == function_id) {
      return function_row;
    }
  }
  return std::nullopt;
}

std::optional<FunctionInfo> LiveFunctionsDataView::CreateFunctionInfoFromInstrumentedFunction(
    const InstrumentedFunction& instrumented_function) {
  const ModuleData* module_data = app_->GetModuleByPathAndBuildId(
      instrumented_function.file_path(), instrumented_function.file_build_id());
  if (module_data == nullptr) {
    return std::nullopt;
  }

  const std::string& function_name =
      app_->GetCaptureData().GetScopeName(instrumented_function.function_id());

  FunctionInfo result;
  result.set_pretty_name(function_name);
  result.set_module_path(instrumented_function.file_path());
  result.set_module_build_id(instrumented_function.file_build_id());
  result.set_address(module_data->load_bias() + instrumented_function.file_offset());
  // size is unknown

  return result;
}

}  // namespace orbit_data_views