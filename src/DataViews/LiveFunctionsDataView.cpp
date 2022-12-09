// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/LiveFunctionsDataView.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/ScopeStats.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataView.h"
#include "DataViews/DataViewType.h"
#include "DataViews/FunctionsDataView.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "SymbolProvider/ModuleIdentifier.h"

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleManager;
using orbit_client_data::ScopeId;
using orbit_client_data::ScopeStats;
using orbit_symbol_provider::ModuleIdentifier;

using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::InstrumentedFunction;

namespace orbit_data_views {

LiveFunctionsDataView::LiveFunctionsDataView(LiveFunctionsInterface* live_functions,
                                             AppInterface* app)
    : DataView(DataViewType::kLiveFunctions, app),
      live_functions_(live_functions),
      selected_scope_id_(orbit_grpc_protos::kInvalidFunctionId) {
  update_period_ms_ = 300;
}

const std::vector<DataView::Column>& LiveFunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnType] = {"Type", .0f, SortingOrder::kDescending};
    columns[kColumnName] = {"Name", .4f, SortingOrder::kAscending};
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

[[nodiscard]] static std::string BuildTypePartOfTypeColumnString(
    const orbit_client_data::ScopeInfo& scope_info) {
  switch (scope_info.GetType()) {
    case orbit_client_data::ScopeType::kApiScope:
      return FunctionsDataView::kApiScopeTypeString;
    case orbit_client_data::ScopeType::kApiScopeAsync:
      return FunctionsDataView::kApiScopeAsyncTypeString;
    case orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction:
      return FunctionsDataView::kDynamicallyInstrumentedFunctionTypeString;
    default:
      return "";
  }
}

std::string LiveFunctionsDataView::GetValue(int row, int column) {
  if (!app_->HasCaptureData()) {
    return "";
  }
  if (row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  const ScopeId scope_id = GetScopeId(row);
  const ScopeStats& stats = scope_stats_collection_->GetScopeStatsOrDefault(scope_id);
  const orbit_client_data::ScopeInfo& scope_info = GetScopeInfo(scope_id);

  const FunctionInfo* function = GetFunctionInfoFromRow(row);
  switch (column) {
    case kColumnType: {
      const std::string state_string =
          function == nullptr
              ? ""
              : FunctionsDataView::BuildSelectedAndFrameTrackString(app_, *function);
      const std::string type_string = BuildTypePartOfTypeColumnString(scope_info);
      if (state_string.empty()) return type_string;
      return absl::StrFormat("%s [%s]", type_string, state_string);
    }
    case kColumnName:
      return scope_info.GetName();
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
      return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats.ComputeStdDevNs()));
    case kColumnModule:
      return function == nullptr
                 ? ""
                 : std::filesystem::path(function->module_path()).filename().string();
    case kColumnAddress:
      return function == nullptr ? "" : absl::StrFormat("%#x", function->address());
    default:
      return "";
  }
}

std::vector<int> LiveFunctionsDataView::GetVisibleSelectedIndices() {
  if (!selected_scope_id_.has_value()) return {};
  std::optional<int> visible_selected_index = GetRowFromScopeId(selected_scope_id_.value());
  if (!visible_selected_index.has_value()) return {};
  return {visible_selected_index.value()};
}

void LiveFunctionsDataView::UpdateHighlightedFunctionId(absl::Span<const int> rows) {
  app_->DeselectTimer();
  if (rows.empty()) {
    app_->SetHighlightedScopeId(std::nullopt);
  } else {
    app_->SetHighlightedScopeId(GetScopeId(rows[0]));
  }
}

void LiveFunctionsDataView::UpdateSelectedFunctionId() {
  selected_scope_id_ = app_->GetHighlightedScopeId();
}

void LiveFunctionsDataView::UpdateHistogramWithIndices(
    absl::Span<const int> visible_selected_indices) {
  std::vector<ScopeId> scope_ids;
  std::transform(std::begin(visible_selected_indices), std::end(visible_selected_indices),
                 std::back_inserter(scope_ids),
                 [this](const int index) { return GetScopeId(index); });

  UpdateHistogramWithScopeIds(scope_ids);
}

void LiveFunctionsDataView::UpdateHistogramWithScopeIds(absl::Span<const ScopeId> scope_ids) {
  const std::vector<uint64_t>* timer_durations =
      (app_->HasCaptureData() && !scope_ids.empty())
          ? scope_stats_collection_->GetSortedTimerDurationsForScopeId(scope_ids[0])
          : nullptr;

  if (timer_durations == nullptr) {
    app_->ShowHistogram(nullptr, "", std::nullopt);
    return;
  }

  const ScopeId scope_id = scope_ids[0];
  const std::string& scope_name = GetScopeInfo(scope_id).GetName();
  app_->ShowHistogram(timer_durations, scope_name, scope_id);
}

void LiveFunctionsDataView::OnSelect(absl::Span<const int> rows) {
  UpdateHighlightedFunctionId(rows);
  UpdateSelectedFunctionId();

  UpdateHistogramWithIndices(GetVisibleSelectedIndices());
}

#define ORBIT_STAT_SORT(Member)                                                                    \
  [&](ScopeId a, ScopeId b) {                                                                      \
    const ScopeStats& stats_a = scope_stats_collection_->GetScopeStatsOrDefault(a);                \
    const ScopeStats& stats_b = scope_stats_collection_->GetScopeStatsOrDefault(b);                \
    return orbit_data_views_internal::CompareAscendingOrDescending(stats_a.Member, stats_b.Member, \
                                                                   ascending);                     \
  }

void LiveFunctionsDataView::DoSort() {
  if (!app_->HasCaptureData()) {
    return;
  }
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(ScopeId a, ScopeId b)> sorter = nullptr;

  switch (sorting_column_) {
    case kColumnType: {
      // We order by type first, then by whether the function is selected, then by whether a frame
      // track is enabled
      sorter = MakeSorter(
          [this](ScopeId id) -> std::tuple<orbit_client_data::ScopeType, bool, bool> {
            bool is_selected = false;
            bool is_frame_track_enabled = false;
            const FunctionInfo* function_info = app_->GetCaptureData().GetFunctionInfoByScopeId(id);
            if (function_info != nullptr) {
              is_selected = app_->IsFunctionSelected(*function_info);
              is_frame_track_enabled =
                  FunctionsDataView::ShouldShowFrameTrackIcon(app_, *function_info);
            }
            const orbit_client_data::ScopeType type = GetScopeInfo(id).GetType();
            return std::make_tuple(type, is_selected, is_frame_track_enabled);
          },
          ascending);
      break;
    }
    case kColumnName:
      sorter = MakeSorter(
          [this](ScopeId id) { return absl::AsciiStrToLower(GetScopeInfo(id).GetName()); },
          ascending);
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
      sorter = ORBIT_STAT_SORT(ComputeStdDevNs());
      break;
    case kColumnModule: {
      sorter = MakeFunctionSorter(
          [](const FunctionInfo& function_info) {
            return std::filesystem::path(function_info.module_path()).filename().string();
          },
          ascending, "");
      break;
    }
    case kColumnAddress:
      sorter = MakeFunctionSorter(
          [](const FunctionInfo& function_info) { return function_info.address(); }, ascending, 0);
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), MakeIndexSorter(sorter));
  }
}

DataView::ActionStatus LiveFunctionsDataView::GetActionStatus(
    std::string_view action, int clicked_index, absl::Span<const int> selected_indices) {
  if (action == kMenuActionExportEventsToCsv) return ActionStatus::kVisibleAndEnabled;

  const CaptureData& capture_data = app_->GetCaptureData();
  if (action == kMenuActionJumpToFirst || action == kMenuActionJumpToLast ||
      action == kMenuActionJumpToMin || action == kMenuActionJumpToMax) {
    if (app_->IsCapturing() || selected_indices.size() != 1) {
      return ActionStatus::kVisibleButDisabled;
    }

    ScopeId scope_id = GetScopeId(selected_indices[0]);
    const ScopeStats& stats = scope_stats_collection_->GetScopeStatsOrDefault(scope_id);
    if (stats.count() == 0) return ActionStatus::kVisibleButDisabled;

    return ActionStatus::kVisibleAndEnabled;
  }

  bool is_capture_connected = app_->IsCaptureConnected(capture_data);

  std::function<bool(ScopeId, const FunctionInfo&)> is_visible_action_enabled;
  if (action == kMenuActionDisassembly || action == kMenuActionSourceCode) {
    is_visible_action_enabled = [is_capture_connected](ScopeId /*scope_id*/,
                                                       const FunctionInfo& /*function_info*/) {
      return is_capture_connected;
    };

  } else if (action == kMenuActionSelect) {
    is_visible_action_enabled = [this, is_capture_connected](ScopeId /*scope_id*/,
                                                             const FunctionInfo& function_info) {
      return is_capture_connected && !app_->IsFunctionSelected(function_info) &&
             function_info.IsFunctionSelectable();
    };

  } else if (action == kMenuActionUnselect) {
    is_visible_action_enabled = [this, is_capture_connected](ScopeId /*scope_id*/,
                                                             const FunctionInfo& function_info) {
      return is_capture_connected && app_->IsFunctionSelected(function_info);
    };

  } else if (action == kMenuActionEnableFrameTrack) {
    is_visible_action_enabled = [this, &is_capture_connected, &capture_data](
                                    ScopeId scope_id, const FunctionInfo& function_info) {
      return is_capture_connected
                 ? !app_->IsFrameTrackEnabled(function_info)
                 : !capture_data.IsFrameTrackEnabled(capture_data.ScopeIdToFunctionId(scope_id));
    };

  } else if (action == kMenuActionDisableFrameTrack) {
    is_visible_action_enabled = [this, &is_capture_connected, &capture_data](
                                    ScopeId scope_id, const FunctionInfo& function_info) {
      return is_capture_connected
                 ? app_->IsFrameTrackEnabled(function_info)
                 : capture_data.IsFrameTrackEnabled(capture_data.ScopeIdToFunctionId(scope_id));
    };

  } else if (action == kMenuActionAddIterator) {
    is_visible_action_enabled = [this](ScopeId scope_id, const FunctionInfo& /*function_info*/) {
      const ScopeStats& stats = scope_stats_collection_->GetScopeStatsOrDefault(scope_id);
      // We need at least one function call to a function so that adding iterators makes sense.
      return stats.count() > 0;
    };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  const bool enabled_for_any =
      std::any_of(std::begin(selected_indices), std::end(selected_indices),
                  [this, &is_visible_action_enabled](const int index) {
                    const FunctionInfo* function_info = GetFunctionInfoFromRow(index);
                    if (function_info == nullptr) return false;
                    const ScopeId scope_id = GetScopeId(index);
                    return is_visible_action_enabled(scope_id, *function_info);
                  });

  return enabled_for_any ? ActionStatus::kVisibleAndEnabled : ActionStatus::kVisibleButDisabled;
}

void LiveFunctionsDataView::OnIteratorRequested(absl::Span<const int> selection) {
  for (int i : selection) {
    ScopeId scope_id = GetScopeId(i);
    const FunctionInfo* function_info = GetFunctionInfoFromRow(i);
    if (function_info == nullptr) continue;

    const ScopeStats& stats = scope_stats_collection_->GetScopeStatsOrDefault(scope_id);
    if (stats.count() > 0) {
      live_functions_->AddIterator(scope_id, function_info);
    }
  }
}

void LiveFunctionsDataView::OnJumpToRequested(std::string_view action,
                                              absl::Span<const int> selection) {
  ORBIT_CHECK(selection.size() == 1);
  auto scope_id = GetScopeId(selection[0]);
  if (action == kMenuActionJumpToFirst) {
    app_->JumpToTimerAndZoom(scope_id, AppInterface::JumpToTimerMode::kFirst);
  } else if (action == kMenuActionJumpToLast) {
    app_->JumpToTimerAndZoom(scope_id, AppInterface::JumpToTimerMode::kLast);
  } else if (action == kMenuActionJumpToMin) {
    app_->JumpToTimerAndZoom(scope_id, AppInterface::JumpToTimerMode::kMin);
  } else if (action == kMenuActionJumpToMax) {
    app_->JumpToTimerAndZoom(scope_id, AppInterface::JumpToTimerMode::kMax);
  }
}

[[nodiscard]] ErrorMessageOr<void> LiveFunctionsDataView::WriteEventsToCsv(
    absl::Span<const int> selection, std::string_view file_path) const {
  OUTCOME_TRY(auto fd, orbit_base::OpenFileForWriting(file_path));

  // Write header line
  constexpr size_t kNumColumns = 5;
  const std::array<std::string, kNumColumns> names{"Name", "Thread", "Start", "End",
                                                   "Duration (ns)"};

  OUTCOME_TRY(WriteLineToCsv(fd, names));

  absl::flat_hash_set<ScopeId> selected_scope_ids;
  std::transform(std::begin(selection), std::end(selection),
                 std::inserter(selected_scope_ids, std::begin(selected_scope_ids)),
                 [this](int row) { return GetScopeId(row); });

  const CaptureData& capture_data = app_->GetCaptureData();

  for (const TimerInfo* timer :
       capture_data.GetAllScopeTimers(orbit_client_data::kAllValidScopeTypes)) {
    const std::optional<ScopeId> scope_id = capture_data.ProvideScopeId(*timer);
    ORBIT_CHECK(scope_id.has_value());
    if (!selected_scope_ids.contains(scope_id.value())) continue;

    std::string line;
    line.append(FormatValueForCsv(capture_data.GetScopeInfo(scope_id.value()).GetName()));
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

    OUTCOME_TRY(orbit_base::WriteFully(fd, line));
  }

  return outcome::success();
}

void LiveFunctionsDataView::OnExportEventsToCsvRequested(absl::Span<const int> selection) {
  std::string file_path = app_->GetSaveFile(".csv");
  if (file_path.empty()) return;

  ReportErrorIfAny(WriteEventsToCsv(selection, file_path), "Export all events to CSV");
}

void LiveFunctionsDataView::DoFilter() {
  if (!app_->HasCaptureData()) {
    return;
  }

  indices_.clear();

  const std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  const std::vector<ScopeId> scope_ids = scope_stats_collection_->GetAllProvidedScopeIds();

  for (const ScopeId scope_id : scope_ids) {
    const std::string name = absl::AsciiStrToLower(GetScopeInfo(scope_id).GetName());

    bool match = true;

    for (std::string_view filter_token : tokens) {
      if (name.find(filter_token) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      AddScope(scope_id);
    }
  }

  // Filter drawn textboxes
  absl::flat_hash_set<ScopeId> visible_scope_ids;
  for (size_t i = 0; i < indices_.size(); ++i) {
    visible_scope_ids.insert(GetScopeId(i));
  }
  app_->SetVisibleScopeIds(std::move(visible_scope_ids));
}

void LiveFunctionsDataView::OnDataChanged() {
  UpdateHistogramWithScopeIds({});
  indices_.clear();

  if (!app_->HasCaptureData()) {
    DataView::OnDataChanged();
    return;
  }

  for (ScopeId scope_id : scope_stats_collection_->GetAllProvidedScopeIds()) {
    AddScope(scope_id);
  }

  DataView::OnDataChanged();
}

void LiveFunctionsDataView::OnTimer() {
  if (!app_->IsCapturing()) return;

  const std::vector<ScopeId> missing_scope_ids = FetchMissingScopeIds();

  indices_.reserve(indices_.size() + missing_scope_ids.size());
  for (ScopeId scope_id : missing_scope_ids) {
    AddScope(scope_id);
  }

  OnSort(sorting_column_, {});
}

void LiveFunctionsDataView::OnRefresh(absl::Span<const int> visible_selected_indices,
                                      const RefreshMode& mode) {
  if (mode == RefreshMode::kOnFilter || mode == RefreshMode::kOnSort) {
    UpdateHighlightedFunctionId(visible_selected_indices);
  }
  if (mode != RefreshMode::kOnSort) {
    UpdateHistogramWithIndices(visible_selected_indices);
  }
}

ScopeId LiveFunctionsDataView::GetScopeId(uint32_t row) const {
  ORBIT_CHECK(row < indices_.size());
  return ScopeId(indices_[row]);
}

const FunctionInfo* LiveFunctionsDataView::GetFunctionInfoFromRow(int row) {
  ORBIT_CHECK(static_cast<unsigned int>(row) < indices_.size());
  return app_->GetCaptureData().GetFunctionInfoByScopeId(GetScopeId(row));
}

std::optional<int> LiveFunctionsDataView::GetRowFromScopeId(ScopeId scope_id) {
  for (size_t function_row = 0; function_row < indices_.size(); function_row++) {
    if (GetScopeId(function_row) == scope_id) {
      return static_cast<int>(function_row);
    }
  }
  return std::nullopt;
}

std::optional<FunctionInfo> LiveFunctionsDataView::CreateFunctionInfoFromInstrumentedFunction(
    const InstrumentedFunction& instrumented_function) {
  const ModuleData* module_data = app_->GetModuleByModuleIdentifier(
      ModuleIdentifier{instrumented_function.file_path(), instrumented_function.file_build_id()});
  if (module_data == nullptr) {
    return std::nullopt;
  }

  std::optional<ScopeId> scope_id =
      app_->GetCaptureData().FunctionIdToScopeId(instrumented_function.function_id());
  ORBIT_CHECK(scope_id.has_value());

  const std::string& function_name = GetScopeInfo(scope_id.value()).GetName();

  // size is unknown
  return FunctionInfo{instrumented_function.file_path(),
                      instrumented_function.file_build_id(),
                      instrumented_function.function_virtual_address(),
                      /*size=*/0,
                      function_name,
                      instrumented_function.is_hotpatchable()};
}

[[nodiscard]] const orbit_client_data::ScopeInfo& LiveFunctionsDataView::GetScopeInfo(
    ScopeId scope_id) const {
  ORBIT_CHECK(app_ != nullptr && app_->HasCaptureData());
  return app_->GetCaptureData().GetScopeInfo(scope_id);
}

std::string LiveFunctionsDataView::GetToolTip(int row, int column) {
  if (column == kColumnType) {
    return "Notation:\n"
           "D — Dynamically instrumented function\n"
           "MS — Synchronous manually instrumented scope\n"
           "MA — Asynchronous manually instrumented scope\n"
           "H — The function will be hooked in the next capture\n"
           "F — Frame track enabled";
  }
  return DataView::GetToolTip(row, column);
}

[[nodiscard]] std::vector<ScopeId> LiveFunctionsDataView::FetchMissingScopeIds() const {
  if (!app_->HasCaptureData()) return {};

  std::vector<ScopeId> all_scope_ids = app_->GetCaptureData().GetAllProvidedScopeIds();
  absl::flat_hash_set<uint64_t> known_scope_ids(std::begin(indices_), std::end(indices_));
  const auto last = std::remove_if(
      std::begin(all_scope_ids), std::end(all_scope_ids),
      [&known_scope_ids](ScopeId scope_id) { return known_scope_ids.contains(*scope_id); });
  all_scope_ids.erase(last, std::end(all_scope_ids));
  return all_scope_ids;
}

void LiveFunctionsDataView::SetScopeStatsCollection(
    std::shared_ptr<const orbit_client_data::ScopeStatsCollectionInterface>
        scope_stats_collection) {
  scope_stats_collection_ = std::move(scope_stats_collection);
  OnDataChanged();
  app_->SetHighlightedScopeId(std::nullopt);
  app_->DeselectTimer();
  selected_scope_id_.reset();
  selected_indices_.clear();
}

}  // namespace orbit_data_views