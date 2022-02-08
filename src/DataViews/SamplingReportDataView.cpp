// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/SamplingReportDataView.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <utility>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionUtils.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "CompareAscendingOrDescending.h"
#include "DataViews/DataViewType.h"
#include "DataViews/FunctionsDataView.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "Statistics/BinomialConfidenceInterval.h"
#include "Statistics/StatisticsUtils.h"

using orbit_client_data::CaptureData;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleManager;
using orbit_client_data::ProcessData;
using orbit_client_data::SampledFunction;
using orbit_client_data::ThreadID;

using orbit_client_protos::FunctionInfo;

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
    columns[kColumnUnwindErrors] = {"Unwind errors", .0f, SortingOrder::kDescending};
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
      return (func.unwind_errors > 0)
                 ? absl::StrFormat("%.2f%% (%d)", func.unwind_errors_percent, func.unwind_errors)
                 : "";
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

#define ORBIT_PROC_SORT(Member)                                                               \
  [&](int a, int b) {                                                                         \
    return CompareAscendingOrDescending(functions[a].Member, functions[b].Member, ascending); \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                                        \
  [&](int a, int b) {                                                                       \
    return CompareAscendingOrDescending(Func(functions[a]), Func(functions[b]), ascending); \
  }

#define ORBIT_MODULE_NAME_FUNC_SORT                                             \
  [&](int a, int b) {                                                           \
    return CompareAscendingOrDescending(                                        \
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

std::optional<std::pair<std::string, std::string>>
SamplingReportDataView::GetModulePathAndBuildIdFromRow(int row) const {
  const ProcessData* process = app_->GetCaptureData().process();
  ORBIT_CHECK(process != nullptr);

  const SampledFunction& sampled_function = GetSampledFunction(row);
  ORBIT_CHECK(sampled_function.absolute_address != 0);
  auto result = process->FindModuleByAddress(sampled_function.absolute_address);
  if (result.has_error()) {
    ORBIT_ERROR("result %s", result.error().message());
    return std::nullopt;
  }

  return std::make_pair(result.value().file_path(), result.value().build_id());
}

std::vector<std::vector<std::string>> SamplingReportDataView::GetContextMenuWithGrouping(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_load = false;
  for (int index : selected_indices) {
    const ModuleData* module = GetModuleDataFromRow(index);
    if (module != nullptr && !module->is_loaded()) enable_load = true;
  }

  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_disassembly = false;
  bool enable_source_code = false;
  if (app_->IsCaptureConnected(app_->GetCaptureData())) {
    for (int index : selected_indices) {
      const FunctionInfo* function = GetFunctionInfoFromRow(index);
      if (function != nullptr) {
        enable_select |= !app_->IsFunctionSelected(*function) &&
                         orbit_client_data::function_utils::IsFunctionSelectable(*function);
        enable_unselect |= app_->IsFunctionSelected(*function);
        enable_disassembly = true;
        enable_source_code = true;
      }
    }
  }

  std::vector<std::string> action_group;
  if (enable_load) action_group.emplace_back(std::string{kMenuActionLoadSymbols});
  if (enable_select) action_group.emplace_back(std::string{kMenuActionSelect});
  if (enable_unselect) action_group.emplace_back(std::string{kMenuActionUnselect});
  if (enable_disassembly) action_group.emplace_back(std::string{kMenuActionDisassembly});
  if (enable_source_code) action_group.emplace_back(std::string{kMenuActionSourceCode});

  std::vector<std::vector<std::string>> menu =
      DataView::GetContextMenuWithGrouping(clicked_index, selected_indices);
  menu.insert(menu.begin(), action_group);

  return menu;
}

ModuleData* SamplingReportDataView::GetModuleDataFromRow(int row) const {
  std::optional<std::pair<std::string, std::string>> module_path_and_build_id =
      GetModulePathAndBuildIdFromRow(row);
  if (!module_path_and_build_id.has_value()) return nullptr;

  return app_->GetMutableModuleByPathAndBuildId(module_path_and_build_id.value().first,
                                                module_path_and_build_id.value().second);
}

void SamplingReportDataView::UpdateSelectedIndicesAndFunctionIds(
    const std::vector<int>& selected_indices) {
  selected_indices_.clear();
  selected_function_ids_.clear();
  for (int row : selected_indices) {
    selected_indices_.insert(indices_[row]);
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
    const std::vector<int>& visible_selected_indices) {
  absl::flat_hash_set<uint64_t> addresses;
  for (int index : visible_selected_indices) {
    addresses.insert(GetSampledFunction(index).absolute_address);
  }
  sampling_report_->OnSelectAddresses(addresses, tid_);
}

void SamplingReportDataView::OnSelect(const std::vector<int>& indices) {
  UpdateSelectedIndicesAndFunctionIds(indices);
  UpdateVisibleSelectedAddressesAndTid(indices);
}

void SamplingReportDataView::OnRefresh(const std::vector<int>& visible_selected_indices,
                                       const RefreshMode& mode) {
  if (mode != RefreshMode::kOnFilter && mode != RefreshMode::kOnSort) return;
  UpdateVisibleSelectedAddressesAndTid(visible_selected_indices);
}

void SamplingReportDataView::LinkDataView(DataView* data_view) {
  if (data_view->GetType() != DataViewType::kCallstack) return;

  sampling_report_->SetCallstackDataView(static_cast<CallstackDataView*>(data_view));
}

void SamplingReportDataView::SetSampledFunctions(const std::vector<SampledFunction>& functions) {
  functions_ = functions;
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

std::string SamplingReportDataView::GetToolTip(int row, int column) {
  if (column != kColumnInclusive && column != kColumnExclusive) {
    return "";
  }
  const SampledFunction& function = GetSampledFunction(row);
  uint32_t raw_count = {};
  float percentage = {};
  if (column == kColumnInclusive) {
    raw_count = function.inclusive;
    percentage = function.inclusive_percent;
  } else {
    raw_count = function.exclusive;
    percentage = function.exclusive_percent;
  }

  orbit_statistics::BinomialConfidenceInterval interval =
      app_->GetConfidenceIntervalEstimator().Estimate(percentage / 100.0f, stack_events_count_);

  return absl::StrFormat(
      "The function %s\n"
      "has been encountered %u times in a total of %u stack samples.\n"
      "This makes up for %.2f%% of samples.\n"
      "The 95%% confidence interval for the true percentage is\n"
      "(%.2f%%, %.2f%%).",
      function.name, raw_count, stack_events_count_, percentage, interval.lower * 100.0f,
      interval.upper * 100.0f);
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

}  // namespace orbit_data_views