// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViews/CallstackDataView.h"

#include <absl/hash/hash.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleManager.h"
#include "DataViews/DataViewType.h"
#include "DataViews/FunctionsDataView.h"
#include "OrbitBase/Logging.h"

using orbit_client_data::CallstackInfo;
using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleManager;

namespace orbit_data_views {

CallstackDataView::CallstackDataView(AppInterface* app) : DataView(DataViewType::kCallstack, app) {}

const std::vector<DataView::Column>& CallstackDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnName] = {"Function", .65f, SortingOrder::kAscending};
    columns[kColumnSize] = {"Size", .0f, SortingOrder::kAscending};
    columns[kColumnModule] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Sampled Address", .0f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string CallstackDataView::GetValue(int row, int column) {
  if (row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  CallstackDataViewFrame frame = GetFrameFromRow(row);
  const FunctionInfo* function = frame.function;

  switch (column) {
    case kColumnSelected:
      return (function != nullptr && app_->IsFunctionSelected(*function))
                 ? FunctionsDataView::kSelectedFunctionString
                 : FunctionsDataView::kUnselectedFunctionString;
    case kColumnName:
      return absl::StrCat(functions_to_highlight_.contains(frame.address)
                              ? kHighlightedFunctionString
                              : kHighlightedFunctionBlankString,
                          function != nullptr ? function->pretty_name() : frame.fallback_name);
    case kColumnSize:
      return function != nullptr ? absl::StrFormat("%lu", function->size()) : "";
    case kColumnModule: {
      std::string module_name =
          (function == nullptr)
              ? ""
              : std::filesystem::path(function->module_path()).filename().string();
      if (!module_name.empty()) {
        return module_name;
      }
      const CaptureData& capture_data = app_->GetCaptureData();
      const ModuleManager* module_manager = app_->GetModuleManager();
      return std::filesystem::path(orbit_client_data::GetModulePathByAddress(
                                       *module_manager, capture_data, frame.address))
          .filename()
          .string();
    }
    case kColumnAddress:
      return absl::StrFormat("%#llx", frame.address);
    default:
      return "";
  }
}

std::string CallstackDataView::GetToolTip(int row, int column) {
  if (column != kColumnName) {
    return DataView::GetToolTip(row, column);
  }
  CallstackDataViewFrame frame = GetFrameFromRow(row);
  const std::string& function_name =
      frame.function != nullptr ? frame.function->pretty_name() : frame.fallback_name;
  if (functions_to_highlight_.find(frame.address) != functions_to_highlight_.end()) {
    return absl::StrFormat(
        "%s\n\nFunctions marked with %s are part of the selection in the sampling report above",
        function_name, CallstackDataView::kHighlightedFunctionString);
  } else {
    return function_name;
  }
}

const std::string CallstackDataView::kHighlightedFunctionString = "➜ ";
const std::string CallstackDataView::kHighlightedFunctionBlankString =
    std::string(kHighlightedFunctionString.size(), ' ');

DataView::ActionStatus CallstackDataView::GetActionStatus(std::string_view action,
                                                          int clicked_index,
                                                          absl::Span<const int> selected_indices) {
  bool is_capture_connected = app_->IsCaptureConnected(app_->GetCaptureData());
  if (!is_capture_connected &&
      (action == kMenuActionSelect || action == kMenuActionUnselect ||
       action == kMenuActionDisassembly || action == kMenuActionSourceCode)) {
    return ActionStatus::kVisibleButDisabled;
  }

  std::function<bool(const FunctionInfo*, const ModuleData*)> is_visible_action_enabled;
  if (action == kMenuActionLoadSymbols) {
    is_visible_action_enabled = [](const FunctionInfo* /*function*/, const ModuleData* module) {
      return module != nullptr && !module->AreDebugSymbolsLoaded();
    };

  } else if (action == kMenuActionSelect) {
    is_visible_action_enabled = [this](const FunctionInfo* function, const ModuleData* /*module*/) {
      return function != nullptr && !app_->IsFunctionSelected(*function) &&
             function->IsFunctionSelectable();
    };

  } else if (action == kMenuActionUnselect) {
    is_visible_action_enabled = [this](const FunctionInfo* function, const ModuleData* /*module*/) {
      return function != nullptr && app_->IsFunctionSelected(*function);
    };

  } else if (action == kMenuActionDisassembly || action == kMenuActionSourceCode) {
    is_visible_action_enabled = [](const FunctionInfo* function, const ModuleData* /*module*/) {
      return function != nullptr;
    };

  } else {
    return DataView::GetActionStatus(action, clicked_index, selected_indices);
  }

  for (int index : selected_indices) {
    CallstackDataViewFrame frame = GetFrameFromRow(index);
    const FunctionInfo* function = frame.function;
    const ModuleData* module = frame.module;
    if (is_visible_action_enabled(function, module)) return ActionStatus::kVisibleAndEnabled;
  }
  return ActionStatus::kVisibleButDisabled;
}

void CallstackDataView::DoFilter() {
  if (!callstack_.has_value()) {
    return;
  }

  std::vector<uint64_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (size_t i = 0; i < callstack_->frames().size(); ++i) {
    CallstackDataViewFrame frame = GetFrameFromIndex(i);
    const FunctionInfo* function = frame.function;
    std::string name =
        absl::AsciiStrToLower(function != nullptr ? function->pretty_name() : frame.fallback_name);
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

void CallstackDataView::OnDataChanged() {
  size_t num_functions = callstack_.has_value() ? callstack_->frames().size() : 0;
  indices_.resize(num_functions);
  for (size_t i = 0; i < num_functions; ++i) {
    indices_[i] = i;
  }

  DataView::OnDataChanged();
}

void CallstackDataView::SetFunctionsToHighlight(
    const absl::flat_hash_set<uint64_t>& absolute_addresses) {
  const CaptureData& capture_data = app_->GetCaptureData();
  const ModuleManager* module_manager = app_->GetModuleManager();
  functions_to_highlight_.clear();

  for (uint64_t index : indices_) {
    CallstackDataViewFrame frame = GetFrameFromIndex(index);
    std::optional<uint64_t> callstack_function_absolute_address =
        orbit_client_data::FindFunctionAbsoluteAddressByInstructionAbsoluteAddress(
            *module_manager, capture_data, frame.address);
    if (callstack_function_absolute_address.has_value() &&
        absolute_addresses.contains(callstack_function_absolute_address.value())) {
      functions_to_highlight_.insert(frame.address);
    }
  }
}

bool CallstackDataView::GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                        unsigned char& green, unsigned char& blue) {
  // Row "0" refers to the program counter and is always "correct".
  if (callstack_->IsUnwindingError() && row != 0) {
    constexpr std::array<unsigned char, 3> kUnwindingErrorColor{255, 128, 0};
    red = kUnwindingErrorColor[0];
    green = kUnwindingErrorColor[1];
    blue = kUnwindingErrorColor[2];
    return true;
  }
  CallstackDataViewFrame frame = GetFrameFromRow(row);
  if (functions_to_highlight_.contains(frame.address)) {
    red = 200;
    green = 240;
    blue = 200;
    return true;
  }
  return false;
}

CallstackDataView::CallstackDataViewFrame CallstackDataView::GetFrameFromRow(int row) const {
  return GetFrameFromIndex(indices_[row]);
}

CallstackDataView::CallstackDataViewFrame CallstackDataView::GetFrameFromIndex(
    size_t index_in_callstack) const {
  ORBIT_CHECK(callstack_.has_value());
  ORBIT_CHECK(index_in_callstack < callstack_->frames().size());
  uint64_t address = callstack_->frames()[index_in_callstack];

  const CaptureData& capture_data = app_->GetCaptureData();
  ModuleManager* module_manager = app_->GetMutableModuleManager();
  const FunctionInfo* function = orbit_client_data::FindFunctionByAddress(
      *capture_data.process(), *module_manager, address, false);
  const ModuleData* module =
      orbit_client_data::FindModuleByAddress(*capture_data.process(), *module_manager, address);

  if (function != nullptr) {
    return {address, function, module};
  }
  const std::string& fallback_name =
      orbit_client_data::GetFunctionNameByAddress(*module_manager, capture_data, address);
  return {address, fallback_name, module};
}

}  // namespace orbit_data_views
