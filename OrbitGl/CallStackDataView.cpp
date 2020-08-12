// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallStackDataView.h"

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "FunctionUtils.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;

CallStackDataView::CallStackDataView()
    : DataView(DataViewType::CALLSTACK), callstack_(nullptr) {}

void CallStackDataView::SetAsMainInstance() {}

const std::vector<DataView::Column>& CallStackDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SELECTED] = {"Hooked", .0f, SortingOrder::Descending};
    columns[COLUMN_NAME] = {"Function", .65f, SortingOrder::Ascending};
    columns[COLUMN_SIZE] = {"Size", .0f, SortingOrder::Ascending};
    columns[COLUMN_FILE] = {"File", .0f, SortingOrder::Ascending};
    columns[COLUMN_LINE] = {"Line", .0f, SortingOrder::Ascending};
    columns[COLUMN_MODULE] = {"Module", .0f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Sampled Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

std::string CallStackDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  CallStackDataViewFrame frame = GetFrameFromRow(a_Row);
  FunctionInfo* function = frame.function;
  Module* module = frame.module.get();

  switch (a_Column) {
    case COLUMN_SELECTED:
      return (function != nullptr && FunctionUtils::IsSelected(*function))
                 ? "X"
                 : "-";
    case COLUMN_NAME:
      return function != nullptr ? FunctionUtils::GetDisplayName(*function)
                                 : frame.fallback_name;
    case COLUMN_SIZE:
      return function != nullptr ? absl::StrFormat("%lu", function->size())
                                 : "";
    case COLUMN_FILE:
      return function != nullptr ? function->file() : "";
    case COLUMN_LINE:
      return function != nullptr ? absl::StrFormat("%d", function->line()) : "";
    case COLUMN_MODULE:
      if (function != nullptr &&
          !FunctionUtils::GetLoadedModuleName(*function).empty()) {
        return FunctionUtils::GetLoadedModuleName(*function);
      }
      if (module != nullptr) {
        return module->m_Name;
      }
      if (Capture::GSamplingProfiler != nullptr) {
        return Capture::GSamplingProfiler->GetModuleNameByAddress(
            frame.address);
      }
      return "";
    case COLUMN_ADDRESS:
      return absl::StrFormat("%#llx", frame.address);
    default:
      return "";
  }
}

const std::string CallStackDataView::MENU_ACTION_MODULES_LOAD = "Load Symbols";
const std::string CallStackDataView::MENU_ACTION_SELECT = "Hook";
const std::string CallStackDataView::MENU_ACTION_UNSELECT = "Unhook";
const std::string CallStackDataView::MENU_ACTION_DISASSEMBLY =
    "Go to Disassembly";

std::vector<std::string> CallStackDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_load = false;
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_disassembly = false;
  for (int index : a_SelectedIndices) {
    CallStackDataViewFrame frame = GetFrameFromRow(index);
    FunctionInfo* function = frame.function;
    Module* module = frame.module.get();

    if (frame.function != nullptr) {
      enable_select |= !FunctionUtils::IsSelected(*function);
      enable_unselect |= FunctionUtils::IsSelected(*function);
      enable_disassembly = true;
    } else if (module != nullptr && module->IsLoadable() &&
               !module->IsLoaded()) {
      enable_load = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_load) menu.emplace_back(MENU_ACTION_MODULES_LOAD);
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  if (enable_disassembly) menu.emplace_back(MENU_ACTION_DISASSEMBLY);
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

void CallStackDataView::OnContextMenu(const std::string& a_Action,
                                      int a_MenuIndex,
                                      const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_MODULES_LOAD) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      std::shared_ptr<Module> module = frame.module;
      if (module != nullptr && module->IsLoadable() && !module->IsLoaded()) {
        GOrbitApp->LoadModules(Capture::GTargetProcess->GetID(), {module});
      }
    }

  } else if (a_Action == MENU_ACTION_SELECT) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      FunctionInfo* function = frame.function;
      FunctionUtils::Select(function);
    }

  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      FunctionInfo* function = frame.function;
      FunctionUtils::UnSelect(function);
    }

  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    int32_t pid = Capture::GTargetProcess->GetID();
    for (int i : a_ItemIndices) {
      GOrbitApp->Disassemble(pid, *GetFrameFromRow(i).function);
    }

  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

void CallStackDataView::DoFilter() {
  if (!callstack_) return;

  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = absl::StrSplit(ToLower(m_Filter), ' ');

  for (size_t i = 0; i < callstack_->GetFramesCount(); ++i) {
    CallStackDataViewFrame frame = GetFrameFromIndex(i);
    FunctionInfo* function = frame.function;
    std::string name =
        ToLower(function != nullptr ? FunctionUtils::GetDisplayName(*function)
                                    : frame.fallback_name);
    bool match = true;

    for (std::string& filterToken : tokens) {
      if (name.find(filterToken) == std::string::npos) {
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
  size_t numFunctions = callstack_ ? callstack_->GetFramesCount() : 0;
  indices_.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    indices_[i] = i;
  }

  DataView::OnDataChanged();
}

CallStackDataView::CallStackDataViewFrame CallStackDataView::GetFrameFromRow(
    int row) {
  return GetFrameFromIndex(indices_[row]);
}

CallStackDataView::CallStackDataViewFrame CallStackDataView::GetFrameFromIndex(
    int index_in_callstack) {
  if (callstack_ == nullptr ||
      index_in_callstack >= static_cast<int>(callstack_->GetFramesCount())) {
    return CallStackDataViewFrame();
  }

  uint64_t address = callstack_->GetFrame(index_in_callstack);
  FunctionInfo* function = nullptr;
  std::shared_ptr<Module> module = nullptr;

  if (Capture::GTargetProcess != nullptr) {
    ScopeLock lock(Capture::GTargetProcess->GetDataMutex());
    function = Capture::GTargetProcess->GetFunctionFromAddress(address, false);
    module = Capture::GTargetProcess->GetModuleFromAddress(address);
  }

  if (function != nullptr) {
    return CallStackDataViewFrame(address, function, module);
  } else {
    std::string fallback_name;
    if (Capture::GSamplingProfiler != nullptr) {
      fallback_name =
          Capture::GSamplingProfiler->GetFunctionNameByAddress(address);
    }
    return CallStackDataViewFrame(address, fallback_name, module);
  }
}
