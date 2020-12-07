// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallStackDataView.h"

#include <absl/strings/str_split.h>
#include <stddef.h>

#include <cstdint>
#include <ext/alloc_traits.h>
#include <memory>

#include "App.h"
#include "CoreUtils.h"
#include "DataViewTypes.h"
#include "FunctionsDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientModel/CaptureData.h"
#include "Path.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;

CallStackDataView::CallStackDataView() : DataView(DataViewType::kCallstack) {}

void CallStackDataView::SetAsMainInstance() {}

const std::vector<DataView::Column>& CallStackDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnName] = {"Function", .65f, SortingOrder::kAscending};
    columns[kColumnSize] = {"Size", .0f, SortingOrder::kAscending};
    columns[kColumnFile] = {"File", .0f, SortingOrder::kAscending};
    columns[kColumnLine] = {"Line", .0f, SortingOrder::kAscending};
    columns[kColumnModule] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Sampled Address", .0f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string CallStackDataView::GetValue(int row, int column) {
  if (row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  CallStackDataViewFrame frame = GetFrameFromRow(row);
  const FunctionInfo* function = frame.function;
  const ModuleData* module = frame.module;

  switch (column) {
    case kColumnSelected:
      return (function != nullptr && GOrbitApp->IsFunctionSelected(*function))
                 ? FunctionsDataView::kSelectedFunctionString
                 : FunctionsDataView::kUnselectedFunctionString;
    case kColumnName:
      return function != nullptr ? FunctionUtils::GetDisplayName(*function) : frame.fallback_name;
    case kColumnSize:
      return function != nullptr ? absl::StrFormat("%lu", function->size()) : "";
    case kColumnFile:
      return function != nullptr ? function->file() : "";
    case kColumnLine:
      return function != nullptr ? absl::StrFormat("%d", function->line()) : "";
    case kColumnModule: {
      if (function != nullptr && !FunctionUtils::GetLoadedModuleName(*function).empty()) {
        return FunctionUtils::GetLoadedModuleName(*function);
      }
      if (module != nullptr) {
        return module->name();
      }
      const CaptureData& capture_data = GOrbitApp->GetCaptureData();
      return Path::GetFileName(capture_data.GetModulePathByAddress(frame.address));
    }
    case kColumnAddress:
      return absl::StrFormat("%#llx", frame.address);
    default:
      return "";
  }
}

const std::string CallStackDataView::kMenuActionLoadSymbols = "Load Symbols";
const std::string CallStackDataView::kMenuActionSelect = "Hook";
const std::string CallStackDataView::kMenuActionUnselect = "Unhook";
const std::string CallStackDataView::kMenuActionDisassembly = "Go to Disassembly";

std::vector<std::string> CallStackDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_load = false;
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_disassembly = false;
  for (int index : selected_indices) {
    CallStackDataViewFrame frame = GetFrameFromRow(index);
    const FunctionInfo* function = frame.function;
    const ModuleData* module = frame.module;

    if (frame.function != nullptr && GOrbitApp->IsCaptureConnected(GOrbitApp->GetCaptureData())) {
      enable_select |= !GOrbitApp->IsFunctionSelected(*function);
      enable_unselect |= GOrbitApp->IsFunctionSelected(*function);
      enable_disassembly = true;
    } else if (module != nullptr && !module->is_loaded()) {
      enable_load = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_load) menu.emplace_back(kMenuActionLoadSymbols);
  if (enable_select) menu.emplace_back(kMenuActionSelect);
  if (enable_unselect) menu.emplace_back(kMenuActionUnselect);
  if (enable_disassembly) menu.emplace_back(kMenuActionDisassembly);
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void CallStackDataView::OnContextMenu(const std::string& action, int menu_index,
                                      const std::vector<int>& item_indices) {
  if (action == kMenuActionLoadSymbols) {
    std::vector<ModuleData*> modules_to_load;
    for (int i : item_indices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      ModuleData* module = frame.module;
      if (module != nullptr && !module->is_loaded()) {
        modules_to_load.push_back(module);
      }
    }
    GOrbitApp->LoadModules(modules_to_load);

  } else if (action == kMenuActionSelect) {
    for (int i : item_indices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      const FunctionInfo* function = frame.function;
      GOrbitApp->SelectFunction(*function);
    }

  } else if (action == kMenuActionUnselect) {
    for (int i : item_indices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      const FunctionInfo* function = frame.function;
      GOrbitApp->DeselectFunction(*function);
      GOrbitApp->DisableFrameTrack(*function);
    }

  } else if (action == kMenuActionDisassembly) {
    const int32_t pid = GOrbitApp->GetCaptureData().process_id();
    for (int i : item_indices) {
      GOrbitApp->Disassemble(pid, *GetFrameFromRow(i).function);
    }

  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void CallStackDataView::DoFilter() {
  if (callstack_.GetFramesCount() == 0) {
    return;
  }

  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(ToLower(filter_), ' ');

  for (size_t i = 0; i < callstack_.GetFramesCount(); ++i) {
    CallStackDataViewFrame frame = GetFrameFromIndex(i);
    const FunctionInfo* function = frame.function;
    std::string name = ToLower(function != nullptr ? FunctionUtils::GetDisplayName(*function)
                                                   : frame.fallback_name);
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

  indices_ = indices;
}

void CallStackDataView::OnDataChanged() {
  size_t num_functions = callstack_.GetFramesCount();
  indices_.resize(num_functions);
  for (size_t i = 0; i < num_functions; ++i) {
    indices_[i] = i;
  }

  DataView::OnDataChanged();
}

CallStackDataView::CallStackDataViewFrame CallStackDataView::GetFrameFromRow(int row) {
  return GetFrameFromIndex(indices_[row]);
}

CallStackDataView::CallStackDataViewFrame CallStackDataView::GetFrameFromIndex(
    int index_in_callstack) {
  CHECK(index_in_callstack < static_cast<int>(callstack_.GetFramesCount()));
  uint64_t address = callstack_.GetFrame(index_in_callstack);

  const CaptureData& capture_data = GOrbitApp->GetCaptureData();
  const FunctionInfo* function = capture_data.FindFunctionByAddress(address, false);
  ModuleData* module = capture_data.FindModuleByAddress(address);

  if (function != nullptr) {
    return CallStackDataViewFrame(address, function, module);
  }
  const std::string& fallback_name = capture_data.GetFunctionNameByAddress(address);
  return CallStackDataViewFrame(address, fallback_name, module);
}
