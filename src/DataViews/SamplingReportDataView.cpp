// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/SamplingReportDataView.h"

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "DataViews/CallstackDataView.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"
#include "DataViews/FunctionsDataView.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "Statistics/BinomialConfidenceInterval.h"
#include "Statistics/StatisticsUtils.h"

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleManager;
using orbit_client_data::ProcessData;
using orbit_client_data::SampledFunction;
using orbit_client_data::ThreadID;
using orbit_symbol_provider::ModuleIdentifier;

namespace orbit_data_views {

SamplingReportDataView::SamplingReportDataView(AppInterface* app)
    : DataView(DataViewType::kSampling, app) {}

const std::vector<DataView::Column>& SamplingReportDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnFunctionName] = {"Name", .4f, SortingOrder::kAscending};
    columns[kColumnInclusive] = {"Inclusive, %", .0f, SortingOrder::kDescending};
    columns[kColumnExclusive] = {"Exclusive, %", .0f, SortingOrder::kDescending};
    columns[kColumnModuleName] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Address", .0f, SortingOrder::kAscending};
    columns[kColumnUnwindErrors] = {"Unwind errors, %", .0f, SortingOrder::kDescending};
    return columns;
  }();
  return columns;
}

[[nodiscard]] std::string SamplingReportDataView::BuildPercentageString(float percentage) const {
  const float rate = percentage / 100.0f;
  orbit_statistics::BinomialConfidenceInterval interval =
      app_->GetConfidenceIntervalEstimator().Estimate(rate, stack_events_count_);
  const float plus_minus_percentage =
      orbit_statistics::HalfWidthOfSymmetrizedConfidenceInterval(interval, rate) * 100.0f;
  return absl::StrFormat("%.1f ±%.1f", percentage, plus_minus_percentage);
}

std::string SamplingReportDataView::GetValue(int row, int column) {
  const SampledFunction& func = GetSampledFunction(row);

  switch (column) {
    case kColumnSelected:
      return app_->IsFunctionSelected(func) ? FunctionsDataView::kSelectedFunctionString
                                            : FunctionsDataView::kUnselectedFunctionString;
    case kColumnFunctionName:
      return func.name;
    case kColumnInclusive:
      return BuildPercentageString(func.inclusive_percent);
    case kColumnExclusive:
      return BuildPercentageString(func.exclusive_percent);
    case kColumnModuleName:
      return std::filesystem::path(func.module_path).filename().string();
    case kColumnAddress:
      return absl::StrFormat("%#llx", func.absolute_address);
    case kColumnUnwindErrors:
      return (func.unwind_errors > 0) ? BuildPercentageString(func.unwind_errors_percent) : "";
    default:
      return "";
  }
}

// For columns with two values, a percentage and a raw number, only copy the percentage, so that it
// can be interpreted as a number by a spreadsheet.
std::string SamplingReportDataView::GetValueForCopy(int row, int column) {
  const SampledFunction& func = GetSampledFunction(row);
  switch (column) {
    case kColumnInclusive:
      return absl::StrFormat("%.2f%%", func.inclusive_percent);
    case kColumnExclusive:
      return absl::StrFormat("%.2f%%", func.exclusive_percent);
    case kColumnUnwindErrors:
      return (func.unwind_errors > 0) ? absl::StrFormat("%.2f%%", func.unwind_errors_percent) : "";
    default:
      return GetValue(row, column);
  }
}

#define ORBIT_PROC_SORT(Member)                                     \
  [&](int a, int b) {                                               \
    return orbit_data_views_internal::CompareAscendingOrDescending( \
        functions[a].Member, functions[b].Member, ascending);       \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                                               \
  [&](int a, int b) {                                                                              \
    return orbit_data_views_internal::CompareAscendingOrDescending(Func(functions[a]),             \
                                                                   Func(functions[b]), ascending); \
  }

#define ORBIT_MODULE_NAME_FUNC_SORT                                             \
  [&](int a, int b) {                                                           \
    return orbit_data_views_internal::CompareAscendingOrDescending(             \
        std::filesystem::path(functions[a].module_path).filename(),             \
        std::filesystem::path(functions[b].module_path).filename(), ascending); \
  }

void SamplingReportDataView::DoSort() {
  ORBIT_SCOPE("SamplingReportDataView::DoSort");
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  std::vector<SampledFunction>& functions = functions_;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(app_->IsFunctionSelected);
      break;
    case kColumnFunctionName:
      sorter = ORBIT_PROC_SORT(name);
      break;
    case kColumnInclusive:
      sorter = ORBIT_PROC_SORT(inclusive);
      break;
    case kColumnExclusive:
      sorter = ORBIT_PROC_SORT(exclusive);
      break;
    case kColumnModuleName:
      sorter = ORBIT_MODULE_NAME_FUNC_SORT;
      break;
    case kColumnAddress:
      sorter = ORBIT_PROC_SORT(absolute_address);
      break;
    case kColumnUnwindErrors:
      sorter = ORBIT_PROC_SORT(unwind_errors);
      break;
    default:
      break;
  }

  if (!sorter) {
    return;
  }

  const auto fallback_sorter = [&](const auto& ind_left, const auto& ind_right) {
    // `SampledFunction::address` is the absolute function address. Hence it is unique and qualifies
    // for total ordering.
    return functions[ind_left].absolute_address < functions[ind_right].absolute_address;
  };

  const auto combined_sorter = [&](const auto& ind_left, const auto& ind_right) {
    if (sorter(ind_left, ind_right)) {
      return true;
    }

    if (sorter(ind_right, ind_left)) {
      return false;
    }

    return fallback_sorter(ind_left, ind_right);
  };

  std::sort(indices_.begin(), indices_.end(), combined_sorter);
}

const FunctionInfo* SamplingReportDataView::GetFunctionInfoFromRow(int row) {
  const CaptureData& capture_data = app_->GetCaptureData();
  const ModuleManager* module_manager = app_->GetModuleManager();
  SampledFunction& sampled_function = GetSampledFunction(row);
  if (sampled_function.function == nullptr) {
    const FunctionInfo* func = orbit_client_data::FindFunctionByAddress(
        *capture_data.process(), *module_manager, sampled_function.absolute_address, false);
    sampled_function.function = func;
  }

  return sampled_function.function;
}

std::optional<ModuleIdentifier> SamplingReportDataView::GetModuleIdentifierFromRow(int row) const {
  const ProcessData* process = app_->GetCaptureData().process();
  ORBIT_CHECK(process != nullptr);

  const SampledFunction& sampled_function = GetSampledFunction(row);
  ORBIT_CHECK(sampled_function.absolute_address != 0);
  auto result = process->FindModuleByAddress(sampled_function.absolute_address);
  if (result.has_error()) {
    ORBIT_ERROR("result %s", result.error().message());
    return std::nullopt;
  }

  return result.value().module_id();
}

DataView::ActionStatus SamplingReportDataView::GetActionStatus(
    std::string_view action, int clicked_index, absl::Span<const int> selected_indices) {
  if (action == kMenuActionLoadSymbols) {
    for (int index : selected_indices) {
      const ModuleData* module = GetModuleDataFromRow(index);
      if (module != nullptr && !module->AreDebugSymbolsLoaded()) {
        return ActionStatus::kVisibleAndEnabled;
      }
    }
    return ActionStatus::kVisibleButDisabled;
  }

  bool is_capture_connected = app_->IsCaptureConnected(app_->GetCaptureData());
  if (!is_capture_connected &&
      (action == kMenuActionSelect || action == kMenuActionUnselect ||
       action == kMenuActionDisassembly || action == kMenuActionSourceCode)) {
    return ActionStatus::kVisibleButDisabled;
  }

  if (action == kMenuActionExportEventsToCsv) return ActionStatus::kVisibleAndEnabled;

  std::function<bool(const FunctionInfo*)> is_visible_action_enabled;
  if (action == kMenuActionSelect) {
    is_visible_action_enabled = [this](const FunctionInfo* function) {
      return function != nullptr && !app_->IsFunctionSelected(*function) &&
             function->IsFunctionSelectable();
    };

  } else if (action == kMenuActionUnselect) {
    is_visible_action_enabled = [this](const FunctionInfo* function) {
      return function != nullptr && app_->IsFunctionSelected(*function);
    };

  } else if (action == kMenuActionDisassembly || action == kMenuActionSourceCode) {
    is_visible_action_enabled = [](const FunctionInfo* function) { return function != nullptr; };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  for (int index : selected_indices) {
    const FunctionInfo* function = GetFunctionInfoFromRow(index);
    if (is_visible_action_enabled(function)) return ActionStatus::kVisibleAndEnabled;
  }
  return ActionStatus::kVisibleButDisabled;
}

ModuleData* SamplingReportDataView::GetModuleDataFromRow(int row) const {
  std::optional<ModuleIdentifier> module_id_opt = GetModuleIdentifierFromRow(row);
  if (!module_id_opt.has_value()) return nullptr;

  return app_->GetMutableModuleByModuleIdentifier(module_id_opt.value());
}

void SamplingReportDataView::UpdateSelectedIndicesAndFunctionIds(
    absl::Span<const int> selected_indices) {
  selected_indices_.clear();
  selected_function_ids_.clear();
  for (int row : selected_indices) {
    selected_indices_.insert(static_cast<int>(indices_[row]));
    selected_function_ids_.insert(GetSampledFunction(row).absolute_address);
  }
}

void SamplingReportDataView::RestoreSelectedIndicesAfterFunctionsChanged() {
  selected_indices_.clear();
  for (size_t row = 0; row < functions_.size(); ++row) {
    if (selected_function_ids_.contains(functions_[row].absolute_address)) {
      selected_indices_.insert(static_cast<int>(row));
    }
  }
}

void SamplingReportDataView::UpdateVisibleSelectedAddressesAndTid(
    absl::Span<const int> visible_selected_indices) {
  absl::flat_hash_set<uint64_t> addresses;
  for (int index : visible_selected_indices) {
    addresses.insert(GetSampledFunction(index).absolute_address);
  }
  sampling_report_->OnSelectAddresses(addresses, tid_);
}

void SamplingReportDataView::OnSelect(absl::Span<const int> indices) {
  UpdateSelectedIndicesAndFunctionIds(indices);
  UpdateVisibleSelectedAddressesAndTid(indices);
}

void SamplingReportDataView::OnRefresh(absl::Span<const int> visible_selected_indices,
                                       const RefreshMode& mode) {
  if (mode != RefreshMode::kOnFilter && mode != RefreshMode::kOnSort) return;
  UpdateVisibleSelectedAddressesAndTid(visible_selected_indices);
}

void SamplingReportDataView::LinkDataView(DataView* data_view) {
  if (data_view->GetType() != DataViewType::kCallstack) return;

  sampling_report_->SetCallstackDataView(static_cast<CallstackDataView*>(data_view));
}

void SamplingReportDataView::SetSampledFunctions(absl::Span<const SampledFunction> functions) {
  functions_.assign(functions.begin(), functions.end());
  RestoreSelectedIndicesAfterFunctionsChanged();

  size_t num_functions = functions_.size();
  indices_.resize(num_functions);
  for (size_t i = 0; i < num_functions; ++i) {
    indices_[i] = i;
  }

  OnDataChanged();
}

void SamplingReportDataView::SetThreadID(ThreadID tid) {
  tid_ = tid;
  const orbit_client_data::CaptureData& capture_data = app_->GetCaptureData();

  if (tid == orbit_base::kAllProcessThreadsTid) {
    name_ = absl::StrFormat("%s\n(all threads)", capture_data.process_name());
  } else {
    name_ = absl::StrFormat("%s\n[%d]", capture_data.GetThreadName(tid_), tid_);
  }
}

void SamplingReportDataView::SetStackEventsCount(uint32_t stack_events_count) {
  stack_events_count_ = stack_events_count;
}

[[nodiscard]] static std::string BuildTooltipTail(
    uint32_t stack_events_count, float percentage,
    const orbit_statistics::BinomialConfidenceInterval& interval) {
  return absl::StrFormat(
      "in a total of %u stack samples.\n"
      "This makes up for %.2f%% of samples.\n\n"
      "The 95%% confidence interval for the true percentage is\n"
      "(%.2f%%, %.2f%%).",
      stack_events_count, percentage, interval.lower * 100.0f, interval.upper * 100.0f);
}

[[nodiscard]] std::string SamplingReportDataView::BuildToolTipInclusive(
    const SampledFunction& function) const {
  const orbit_statistics::BinomialConfidenceInterval interval =
      app_->GetConfidenceIntervalEstimator().Estimate(function.inclusive_percent / 100.0f,
                                                      stack_events_count_);
  const std::string head = absl::StrFormat(
      "The function \"%s\"\n"
      "was encountered %u times (inclusive count)\n",
      function.name, function.inclusive);
  return absl::StrCat(head,
                      BuildTooltipTail(stack_events_count_, function.inclusive_percent, interval));
}

[[nodiscard]] std::string SamplingReportDataView::BuildToolTipExclusive(
    const SampledFunction& function) const {
  const orbit_statistics::BinomialConfidenceInterval interval =
      app_->GetConfidenceIntervalEstimator().Estimate(function.exclusive_percent / 100.0f,
                                                      stack_events_count_);

  const std::string head = absl::StrFormat(
      "The function \"%s\"\n"
      "was at the top of the callstack %u times (exclusive count)\n",
      function.name, function.exclusive);

  return absl::StrCat(head,
                      BuildTooltipTail(stack_events_count_, function.exclusive_percent, interval));
}

[[nodiscard]] std::string SamplingReportDataView::BuildToolTipUnwindErrors(
    const SampledFunction& function) const {
  if (function.unwind_errors == 0) return "";

  const orbit_statistics::BinomialConfidenceInterval interval =
      app_->GetConfidenceIntervalEstimator().Estimate(function.unwind_errors_percent / 100.0f,
                                                      stack_events_count_);
  const std::string head = absl::StrFormat(
      "%u samples with the function \"%s\"\n"
      "at the top of the stack could not be unwound\n",
      function.unwind_errors, function.name);

  return absl::StrCat(
      head, BuildTooltipTail(stack_events_count_, function.unwind_errors_percent, interval));
}

std::string SamplingReportDataView::GetToolTip(int row, int column) {
  const SampledFunction& function = GetSampledFunction(row);
  switch (column) {
    case kColumnInclusive:
      return BuildToolTipInclusive(function);
    case kColumnExclusive:
      return BuildToolTipExclusive(function);
    case kColumnUnwindErrors:
      return BuildToolTipUnwindErrors(function);
    default:
      return DataView::GetToolTip(row, column);
  }
}

void SamplingReportDataView::DoFilter() {
  std::vector<uint64_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (size_t i = 0; i < functions_.size(); ++i) {
    SampledFunction& func = functions_[i];
    std::string name = absl::AsciiStrToLower(func.name);
    std::string module_name =
        absl::AsciiStrToLower(std::filesystem::path(func.module_path).filename().string());

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (!(name.find(filter_token) != std::string::npos ||
            module_name.find(filter_token) != std::string::npos)) {
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

const SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) const {
  return functions_[indices_[row]];
}

SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) {
  return functions_[indices_[row]];
}

ErrorMessageOr<void> SamplingReportDataView::WriteStackEventsToCsv(std::string_view file_path) {
  OUTCOME_TRY(auto fd, orbit_base::OpenFileForWriting(file_path));

  static const std::vector<std::string> names{"Thread", "Timestamp (ns)", "Names leaf/foo/main",
                                              "Addresses leaf_addr/foo_addr/main_addr"};
  constexpr std::string_view kFramesSeparator = "/";

  OUTCOME_TRY(WriteLineToCsv(fd, names));
  const orbit_client_data::CallstackData& callstack_data = sampling_report_->GetCallstackData();

  const std::vector<orbit_client_data::CallstackEvent> callstack_events =
      GetThreadID() != orbit_base::kAllProcessThreadsTid
          ? callstack_data.GetCallstackEventsOfTidInTimeRange(GetThreadID(),
                                                              std::numeric_limits<uint64_t>::min(),
                                                              std::numeric_limits<uint64_t>::max())
          : callstack_data.GetCallstackEventsInTimeRange(std::numeric_limits<uint64_t>::min(),
                                                         std::numeric_limits<uint64_t>::max());

  std::optional<absl::flat_hash_set<uint64_t>> selected_callstack_ids =
      sampling_report_->GetSelectedCallstackIds();

  for (const orbit_client_data::CallstackEvent& event : callstack_events) {
    if (selected_callstack_ids.has_value() &&
        !selected_callstack_ids->contains(event.callstack_id())) {
      continue;
    }

    std::vector<std::string> cells;
    const uint32_t thread_id = event.thread_id();
    cells.push_back(
        absl::StrFormat("%s [%u]", app_->GetCaptureData().GetThreadName(thread_id), thread_id));
    cells.push_back(absl::StrFormat("%u", event.timestamp_ns()));

    const orbit_client_data::CallstackInfo* callstack =
        callstack_data.GetCallstack(event.callstack_id());

    std::vector<std::string> names;
    std::vector<std::string> addresses;
    for (const uint64_t address : callstack->frames()) {
      names.push_back(orbit_client_data::GetFunctionNameByAddress(*app_->GetModuleManager(),
                                                                  app_->GetCaptureData(), address));
      addresses.push_back(absl::StrFormat("%#llx", address));
    }
    cells.push_back(absl::StrJoin(names, kFramesSeparator));
    cells.push_back(absl::StrJoin(addresses, kFramesSeparator));

    OUTCOME_TRY(WriteLineToCsv(fd, cells));
  }

  return outcome::success();
}

// The argument `selection` is ignored as the selected functions are more conveniently obtained as
// `sampling_report_->GetSelectedCallstackIds()`
void SamplingReportDataView::OnExportEventsToCsvRequested(absl::Span<const int> /*selection*/) {
  std::string file_path = app_->GetSaveFile(".csv");
  if (file_path.empty()) return;

  ReportErrorIfAny(WriteStackEventsToCsv(file_path), "Export sampled stacks to CSV");
}

}  // namespace orbit_data_views