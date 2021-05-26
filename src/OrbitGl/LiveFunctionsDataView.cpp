// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LiveFunctionsDataView.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/time/time.h>
#include <llvm/Demangle/Demangle.h>
#include <stddef.h>

#include <functional>
#include <limits>
#include <memory>

#include "App.h"
#include "ClientData/FunctionUtils.h"
#include "ClientModel/CaptureData.h"
#include "CoreUtils.h"
#include "DataViewTypes.h"
#include "FunctionsDataView.h"
#include "GrpcProtos/Constants.h"
#include "LiveFunctionsController.h"
#include "OrbitBase/Logging.h"
#include "TextBox.h"
#include "TimerChain.h"
#include "capture_data.pb.h"

using orbit_client_data::ModuleData;
using orbit_client_model::CaptureData;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_grpc_protos::InstrumentedFunction;

ABSL_DECLARE_FLAG(bool, enable_source_code_view);

LiveFunctionsDataView::LiveFunctionsDataView(
    LiveFunctionsController* live_functions, OrbitApp* app,
    orbit_metrics_uploader::MetricsUploader* metrics_uploader)
    : DataView(DataViewType::kLiveFunctions, app),
      live_functions_(live_functions),
      selected_function_id_(orbit_grpc_protos::kInvalidFunctionId),
      metrics_uploader_(metrics_uploader) {
  update_period_ms_ = 300;
  LiveFunctionsDataView::OnDataChanged();
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
    columns[kColumnModule] = {"Module", .1f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Address", .0f, SortingOrder::kAscending};
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
  const FunctionStats& stats = app_->GetCaptureData().GetFunctionStatsOrDefault(function_id);

  const FunctionInfo& function = GetInstrumentedFunction(row);
  switch (column) {
    case kColumnSelected:
      return FunctionsDataView::BuildSelectedColumnsString(app_, function);
    case kColumnName:
      return orbit_client_data::function_utils::GetDisplayName(function);
    case kColumnCount:
      return absl::StrFormat("%lu", stats.count());
    case kColumnTimeTotal:
      return GetPrettyTime(absl::Nanoseconds(stats.total_time_ns()));
    case kColumnTimeAvg:
      return GetPrettyTime(absl::Nanoseconds(stats.average_time_ns()));
    case kColumnTimeMin:
      return GetPrettyTime(absl::Nanoseconds(stats.min_ns()));
    case kColumnTimeMax:
      return GetPrettyTime(absl::Nanoseconds(stats.max_ns()));
    case kColumnModule:
      return function.module_path();
    case kColumnAddress: {
      const CaptureData& capture_data = app_->GetCaptureData();
      return absl::StrFormat("0x%" PRIx64, capture_data.GetAbsoluteAddress(function).value_or(0));
    }
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
  app_->DeselectTextBox();
  if (rows.empty()) {
    app_->set_highlighted_function_id(orbit_grpc_protos::kInvalidFunctionId);
  } else {
    app_->set_highlighted_function_id(GetInstrumentedFunctionId(rows[0]));
  }
}

void LiveFunctionsDataView::UpdateSelectedFunctionId() {
  selected_function_id_ = app_->highlighted_function_id();
}

void LiveFunctionsDataView::OnSelect(const std::vector<int>& rows) {
  UpdateHighlightedFunctionId(rows);
  UpdateSelectedFunctionId();
}

#define ORBIT_FUNC_SORT(Member)                                                            \
  [&](uint64_t a, uint64_t b) {                                                            \
    return orbit_core::Compare(functions.at(a).Member, functions.at(b).Member, ascending); \
  }
#define ORBIT_STAT_SORT(Member)                                                         \
  [&](uint64_t a, uint64_t b) {                                                         \
    const FunctionStats& stats_a = app_->GetCaptureData().GetFunctionStatsOrDefault(a); \
    const FunctionStats& stats_b = app_->GetCaptureData().GetFunctionStatsOrDefault(b); \
    return orbit_core::Compare(stats_a.Member, stats_b.Member, ascending);              \
  }
#define ORBIT_CUSTOM_FUNC_SORT(Func)                                                     \
  [&](uint64_t a, uint64_t b) {                                                          \
    return orbit_core::Compare(Func(functions.at(a)), Func(functions.at(b)), ascending); \
  }

void LiveFunctionsDataView::DoSort() {
  if (!app_->HasCaptureData()) {
    CHECK(functions_.empty());
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
      sorter = ORBIT_STAT_SORT(average_time_ns());
      break;
    case kColumnTimeMin:
      sorter = ORBIT_STAT_SORT(min_ns());
      break;
    case kColumnTimeMax:
      sorter = ORBIT_STAT_SORT(max_ns());
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

const std::string LiveFunctionsDataView::kMenuActionSelect = "Hook";
const std::string LiveFunctionsDataView::kMenuActionUnselect = "Unhook";
const std::string LiveFunctionsDataView::kMenuActionJumpToFirst = "Jump to first";
const std::string LiveFunctionsDataView::kMenuActionJumpToLast = "Jump to last";
const std::string LiveFunctionsDataView::kMenuActionJumpToMin = "Jump to min";
const std::string LiveFunctionsDataView::kMenuActionJumpToMax = "Jump to max";
const std::string LiveFunctionsDataView::kMenuActionDisassembly = "Go to Disassembly";
const std::string LiveFunctionsDataView::kMenuActionIterate = "Add iterator(s)";
const std::string LiveFunctionsDataView::kMenuActionEnableFrameTrack = "Enable frame track(s)";
const std::string LiveFunctionsDataView::kMenuActionDisableFrameTrack = "Disable frame track(s)";
const std::string LiveFunctionsDataView::kMenuActionSourceCode = "Go to Source code";

std::vector<std::string> LiveFunctionsDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_disassembly = false;
  bool enable_source_code = false;
  bool enable_iterator = false;
  bool enable_enable_frame_track = false;
  bool enable_disable_frame_track = false;

  const CaptureData& capture_data = app_->GetCaptureData();
  for (int index : selected_indices) {
    uint64_t instrumented_function_id = GetInstrumentedFunctionId(index);
    const FunctionInfo& instrumented_function = GetInstrumentedFunction(index);

    if (app_->IsCaptureConnected(capture_data)) {
      enable_select |= !app_->IsFunctionSelected(instrumented_function);
      enable_unselect |= app_->IsFunctionSelected(instrumented_function);
      enable_disassembly = true;
      enable_source_code = absl::GetFlag(FLAGS_enable_source_code_view);
    }

    const FunctionStats& stats = capture_data.GetFunctionStatsOrDefault(instrumented_function_id);
    // We need at least one function call to a function so that adding iterators makes sense.
    enable_iterator |= stats.count() > 0;

    if (app_->IsCaptureConnected(capture_data)) {
      enable_enable_frame_track |= !app_->IsFrameTrackEnabled(instrumented_function);
      enable_disable_frame_track |= app_->IsFrameTrackEnabled(instrumented_function);
    } else {
      enable_enable_frame_track |= !capture_data.IsFrameTrackEnabled(instrumented_function_id);
      enable_disable_frame_track |= capture_data.IsFrameTrackEnabled(instrumented_function_id);
    }
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(kMenuActionSelect);
  if (enable_unselect) menu.emplace_back(kMenuActionUnselect);
  if (enable_disassembly) menu.emplace_back(kMenuActionDisassembly);
  if (enable_source_code) menu.emplace_back(kMenuActionSourceCode);

  if (enable_iterator) {
    menu.emplace_back(kMenuActionIterate);
  }
  if (enable_enable_frame_track) {
    menu.emplace_back(kMenuActionEnableFrameTrack);
  }
  if (enable_disable_frame_track) {
    menu.emplace_back(kMenuActionDisableFrameTrack);
  }

  // For now, these actions only make sense when one function is selected,
  // so we don't show them otherwise.
  if (selected_indices.size() == 1) {
    uint64_t instrumented_function_id = GetInstrumentedFunctionId(selected_indices[0]);
    const FunctionStats& stats = capture_data.GetFunctionStatsOrDefault(instrumented_function_id);
    if (stats.count() > 0) {
      menu.insert(menu.end(), {kMenuActionJumpToFirst, kMenuActionJumpToLast, kMenuActionJumpToMin,
                               kMenuActionJumpToMax});
    }
  }
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void LiveFunctionsDataView::OnContextMenu(const std::string& action, int menu_index,
                                          const std::vector<int>& item_indices) {
  const CaptureData& capture_data = app_->GetCaptureData();
  if (action == kMenuActionSelect || action == kMenuActionUnselect ||
      action == kMenuActionDisassembly || action == kMenuActionSourceCode) {
    for (int i : item_indices) {
      const FunctionInfo selected_function = GetInstrumentedFunction(i);
      if (action == kMenuActionSelect) {
        app_->SelectFunction(selected_function);
      } else if (action == kMenuActionUnselect) {
        app_->DeselectFunction(selected_function);
        // Unhooking a function implies disabling (and removing) the frame
        // track for this function. While it would be possible to keep the
        // current frame track in the capture data, this would lead to a
        // somewhat inconsistent state where the frame track for this function
        // is enabled for the current capture but disabled for the next one.
        app_->DisableFrameTrack(selected_function);
        app_->RemoveFrameTrack(selected_function);
      } else if (action == kMenuActionDisassembly) {
        int32_t pid = capture_data.process_id();
        app_->Disassemble(pid, selected_function);
      } else if (action == kMenuActionSourceCode) {
        app_->ShowSourceCode(selected_function);
      }
    }
  } else if (action == kMenuActionJumpToFirst) {
    CHECK(item_indices.size() == 1);
    auto function_id = GetInstrumentedFunctionId(item_indices[0]);
    const auto* first_box = app_->GetTimeGraph()->FindNextFunctionCall(
        function_id, std::numeric_limits<uint64_t>::lowest());
    if (first_box != nullptr) {
      app_->GetMutableTimeGraph()->SelectAndZoom(first_box);
    }
  } else if (action == kMenuActionJumpToLast) {
    CHECK(item_indices.size() == 1);
    auto function_id = GetInstrumentedFunctionId(item_indices[0]);
    const auto* last_box = app_->GetTimeGraph()->FindPreviousFunctionCall(
        function_id, std::numeric_limits<uint64_t>::max());
    if (last_box != nullptr) {
      app_->GetMutableTimeGraph()->SelectAndZoom(last_box);
    }
  } else if (action == kMenuActionJumpToMin) {
    CHECK(item_indices.size() == 1);
    uint64_t function_id = GetInstrumentedFunctionId(item_indices[0]);
    auto [min_box, _] = GetMinMax(function_id);
    if (min_box != nullptr) {
      app_->GetMutableTimeGraph()->SelectAndZoom(min_box);
    }
  } else if (action == kMenuActionJumpToMax) {
    CHECK(item_indices.size() == 1);
    uint64_t function_id = GetInstrumentedFunctionId(item_indices[0]);
    auto [_, max_box] = GetMinMax(function_id);
    if (max_box != nullptr) {
      app_->GetMutableTimeGraph()->SelectAndZoom(max_box);
    }
  } else if (action == kMenuActionIterate) {
    for (int i : item_indices) {
      uint64_t instrumented_function_id = GetInstrumentedFunctionId(i);
      const FunctionInfo& instrumented_function = GetInstrumentedFunction(i);
      const FunctionStats& stats =
          app_->GetCaptureData().GetFunctionStatsOrDefault(instrumented_function_id);
      if (stats.count() > 0) {
        live_functions_->AddIterator(instrumented_function_id, &instrumented_function);
        metrics_uploader_->SendLogEvent(
            orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_ITERATOR_ADD);
      }
    }
  } else if (action == kMenuActionEnableFrameTrack) {
    for (int i : item_indices) {
      const FunctionInfo& function = GetInstrumentedFunction(i);
      if (app_->IsCaptureConnected(capture_data)) {
        app_->SelectFunction(function);
      }
      app_->EnableFrameTrack(function);
      app_->AddFrameTrack(GetInstrumentedFunctionId(i));
    }
  } else if (action == kMenuActionDisableFrameTrack) {
    for (int i : item_indices) {
      app_->DisableFrameTrack(GetInstrumentedFunction(i));
      app_->RemoveFrameTrack(GetInstrumentedFunctionId(i));
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void LiveFunctionsDataView::DoFilter() {
  if (!app_->HasCaptureData()) {
    CHECK(functions_.empty());
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

void LiveFunctionsDataView::OnDataChanged() {
  functions_.clear();
  indices_.clear();

  if (!app_->HasCaptureData()) {
    DataView::OnDataChanged();
    return;
  }

  const absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>&
      instrumented_functions = app_->GetCaptureData().instrumented_functions();
  for (const auto& [function_id, instrumented_function] : instrumented_functions) {
    const FunctionInfo* function_info_from_capture_data =
        app_->GetCaptureData().FindFunctionByModulePathBuildIdAndOffset(
            instrumented_function.file_path(), instrumented_function.file_build_id(),
            instrumented_function.file_offset());

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

    if (orbit_client_data::function_utils::IsOrbitFunctionFromName(function_info->pretty_name())) {
      continue;
    }

    functions_.insert_or_assign(function_id, std::move(*function_info));
    indices_.push_back(function_id);
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
  if (mode != RefreshMode::kOnFilter && mode != RefreshMode::kOnSort) return;
  UpdateHighlightedFunctionId(visible_selected_indices);
}

uint64_t LiveFunctionsDataView::GetInstrumentedFunctionId(uint32_t row) const {
  CHECK(row < indices_.size());
  return indices_[row];
}

const FunctionInfo& LiveFunctionsDataView::GetInstrumentedFunction(uint32_t row) const {
  CHECK(row < indices_.size());
  CHECK(functions_.find(indices_[row]) != functions_.end());
  return functions_.at(indices_[row]);
}

std::pair<const TextBox*, const TextBox*> LiveFunctionsDataView::GetMinMax(
    uint64_t function_id) const {
  const TextBox* min_box = nullptr;
  const TextBox* max_box = nullptr;
  std::vector<std::shared_ptr<TimerChain>> chains =
      app_->GetTimeGraph()->GetAllThreadTrackTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (auto& block : *chain) {
      for (size_t i = 0; i < block.size(); i++) {
        const TextBox& box = block[i];
        if (box.GetTimerInfo().function_id() == function_id) {
          uint64_t elapsed_nanos = box.GetTimerInfo().end() - box.GetTimerInfo().start();
          if (min_box == nullptr ||
              elapsed_nanos < (min_box->GetTimerInfo().end() - min_box->GetTimerInfo().start())) {
            min_box = &box;
          }
          if (max_box == nullptr ||
              elapsed_nanos > (max_box->GetTimerInfo().end() - max_box->GetTimerInfo().start())) {
            max_box = &box;
          }
        }
      }
    }
  }
  return std::make_pair(min_box, max_box);
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

  FunctionInfo result;
  result.set_name(instrumented_function.function_name());
  result.set_pretty_name(llvm::demangle(instrumented_function.function_name()));
  result.set_module_path(instrumented_function.file_path());
  result.set_module_build_id(instrumented_function.file_build_id());
  result.set_address(module_data->load_bias() + instrumented_function.file_offset());
  // size is unknown
  orbit_client_data::function_utils::SetOrbitTypeFromName(&result);

  return result;
}
