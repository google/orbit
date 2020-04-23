//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CallStackDataView.h"

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "SamplingProfiler.h"
#include "absl/strings/str_format.h"

//----------------------------------------------------------------------------
CallStackDataView::CallStackDataView()
    : DataView(DataViewType::CALLSTACK), m_CallStack(nullptr) {}

//-----------------------------------------------------------------------------
void CallStackDataView::SetAsMainInstance() {
  GOrbitApp->RegisterCallStackDataView(this);
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& CallStackDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SELECTED] = {"Hooked", .0f, SortingOrder::Descending};
    columns[COLUMN_INDEX] = {"Index", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Function", .5f, SortingOrder::Ascending};
    columns[COLUMN_SIZE] = {"Size", .0f, SortingOrder::Ascending};
    columns[COLUMN_FILE] = {"File", .0f, SortingOrder::Ascending};
    columns[COLUMN_LINE] = {"Line", .0f, SortingOrder::Ascending};
    columns[COLUMN_MODULE] = {"Module", .0f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Sampled Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string CallStackDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= (int)GetNumElements()) {
    return "";
  }

  CallStackDataViewFrame frame = GetFrameFromRow(a_Row);
  Function* function = frame.function;
  Module* module = frame.module.get();

  switch (a_Column) {
    case COLUMN_SELECTED:
      return (function != nullptr && function->IsSelected()) ? "X" : "-";
    case COLUMN_INDEX:
      return absl::StrFormat("%d", a_Row);
    case COLUMN_NAME:
      return function != nullptr ? function->PrettyName() : frame.fallback_name;
    case COLUMN_SIZE:
      return function != nullptr ? absl::StrFormat("%lu", function->Size())
                                 : "";
    case COLUMN_FILE:
      return function != nullptr ? function->File() : "";
    case COLUMN_LINE:
      return function != nullptr ? absl::StrFormat("%d", function->Line()) : "";
    case COLUMN_MODULE:
      if (function != nullptr && function->GetPdb() != nullptr) {
        return function->GetPdb()->GetName();
      }
      if (module != nullptr) {
        return module->m_Name;
      }
      return "";
    case COLUMN_ADDRESS:
      return absl::StrFormat("%#llx", frame.address);
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
const std::string CallStackDataView::MENU_ACTION_MODULES_LOAD = "Load Symbols";
const std::string CallStackDataView::MENU_ACTION_SELECT = "Hook";
const std::string CallStackDataView::MENU_ACTION_UNSELECT = "Unhook";
const std::string CallStackDataView::MENU_ACTION_VIEW = "Visualize";
const std::string CallStackDataView::MENU_ACTION_DISASSEMBLY =
    "Go to Disassembly";

//-----------------------------------------------------------------------------
std::vector<std::string> CallStackDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_load = false;
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_view_disassembly = false;
  for (int index : a_SelectedIndices) {
    CallStackDataViewFrame frame = GetFrameFromRow(index);
    Function* function = frame.function;
    Module* module = frame.module.get();

    if (frame.function != nullptr) {
      enable_select |= !function->IsSelected();
      enable_unselect |= function->IsSelected();
      enable_view_disassembly = true;
    } else if (module != nullptr && module->m_FoundPdb &&
               !module->GetLoaded()) {
      enable_load = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_load) menu.emplace_back(MENU_ACTION_MODULES_LOAD);
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  if (enable_view_disassembly) {
    Append(menu, {MENU_ACTION_VIEW, MENU_ACTION_DISASSEMBLY});
  }
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnContextMenu(const std::string& a_Action,
                                      int a_MenuIndex,
                                      const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_MODULES_LOAD) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      std::shared_ptr<Module> module = frame.module;
      if (module != nullptr && module->m_FoundPdb && !module->GetLoaded()) {
        GOrbitApp->EnqueueModuleToLoad(module);
      }
      GOrbitApp->LoadModules();
    }

  } else if (a_Action == MENU_ACTION_SELECT) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      Function* function = frame.function;
      function->Select();
    }

  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      Function* function = frame.function;
      function->UnSelect();
    }

  } else if (a_Action == MENU_ACTION_VIEW) {
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      Function* function = frame.function;
      function->Print();
    }
    GOrbitApp->SendToUiNow("output");

  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    uint32_t pid = Capture::GTargetProcess->GetID();
    for (int i : a_ItemIndices) {
      CallStackDataViewFrame frame = GetFrameFromRow(i);
      Function* function = frame.function;
      function->GetDisassembly(pid);
    }

  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void CallStackDataView::DoFilter() {
  if (!m_CallStack) return;

  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = Tokenize(ToLower(m_Filter));

  for (int i = 0; i < (int)m_CallStack->m_Depth; ++i) {
    CallStackDataViewFrame frame = GetFrameFromIndex(i);
    Function* function = frame.function;
    std::string name = ToLower(function != nullptr ? function->PrettyName()
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

  m_Indices = indices;
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnDataChanged() {
  size_t numFunctions = m_CallStack ? m_CallStack->m_Depth : 0;
  m_Indices.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }

  DataView::OnDataChanged();
}

//-----------------------------------------------------------------------------
CallStackDataView::CallStackDataViewFrame CallStackDataView::GetFrameFromRow(
    int row) {
  return GetFrameFromIndex(m_Indices[row]);
}

//-----------------------------------------------------------------------------
CallStackDataView::CallStackDataViewFrame CallStackDataView::GetFrameFromIndex(
    int index_in_callstack) {
  if (m_CallStack == nullptr ||
      index_in_callstack >= static_cast<int>(m_CallStack->m_Depth)) {
    return CallStackDataViewFrame();
  }

  uint64_t address = m_CallStack->m_Data[index_in_callstack];
  Function* function = nullptr;
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
      fallback_name = Capture::GSamplingProfiler->GetSymbolFromAddress(address);
    }
    return CallStackDataViewFrame(address, fallback_name, module);
  }
}
