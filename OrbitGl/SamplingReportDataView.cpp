//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SamplingReportDataView.h"

#include <memory>
#include <set>

#include "App.h"
#include "CallStackDataView.h"
#include "Capture.h"
#include "Core.h"
#include "OrbitModule.h"
#include "OrbitType.h"
#include "SamplingReport.h"

//-----------------------------------------------------------------------------
SamplingReportDataView::SamplingReportDataView()
    : m_CallstackDataView(nullptr) {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> SamplingReportDataView::s_Headers;
std::vector<int> SamplingReportDataView::s_HeaderMap;
std::vector<float> SamplingReportDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> SamplingReportDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void SamplingReportDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back(L"Hooked");
    s_HeaderMap.push_back(SamplingColumn::Toggle);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Index");
    s_HeaderMap.push_back(SamplingColumn::Index);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Name");
    s_HeaderMap.push_back(SamplingColumn::FunctionName);
    s_HeaderRatios.push_back(0.6f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Exclusive");
    s_HeaderMap.push_back(SamplingColumn::Exclusive);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Inclusive");
    s_HeaderMap.push_back(SamplingColumn::Inclusive);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Module");
    s_HeaderMap.push_back(SamplingColumn::ModuleName);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"File");
    s_HeaderMap.push_back(SamplingColumn::SourceFile);
    s_HeaderRatios.push_back(0.2f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Line");
    s_HeaderMap.push_back(SamplingColumn::SourceLine);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Address");
    s_HeaderMap.push_back(SamplingColumn::Address);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& SamplingReportDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& SamplingReportDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
SamplingReportDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
int SamplingReportDataView::GetDefaultSortingColumn() {
  return std::distance(s_HeaderMap.begin(),
                       std::find(s_HeaderMap.begin(), s_HeaderMap.end(),
                                 SamplingColumn::Inclusive));
}

//-----------------------------------------------------------------------------
std::wstring SamplingReportDataView::GetValue(int a_Row, int a_Column) {
  SampledFunction& func = GetSampledFunction(a_Row);

  std::wstring value;

  switch (s_HeaderMap[a_Column]) {
    case SamplingColumn::Toggle:
      value = func.GetSelected() ? L"X" : L"-";
      break;
    case SamplingColumn::Index:
      value = Format(L"%d", a_Row);
      break;
    case SamplingColumn::FunctionName:
      value = func.m_Name;
      break;
    case SamplingColumn::Exclusive:
      value = Format(L"%.2f", func.m_Exclusive);
      break;
    case SamplingColumn::Inclusive:
      value = Format(L"%.2f", func.m_Inclusive);
      break;
    case SamplingColumn::ModuleName:
      value = func.m_Module;
      break;
    case SamplingColumn::SourceFile:
      value = func.m_File;
      break;
    case SamplingColumn::SourceLine:
      value = func.m_Line > 0 ? Format(L"%d", func.m_Line) : L"";
      break;
    case SamplingColumn::Address:
      value = Format(L"0x%llx", func.m_Address);
      break;
    default:
      break;
  }

  return value;
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member)                                          \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(functions[a].Member, functions[b].Member, \
                               ascending);                               \
  }

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnSort(int a_Column,
                                    std::optional<SortingOrder> a_NewOrder) {
  std::vector<SampledFunction>& functions = m_Functions;
  auto column = static_cast<SamplingColumn>(s_HeaderMap[a_Column]);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[column] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[column] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (column) {
    case SamplingColumn::Toggle:
      sorter = ORBIT_PROC_SORT(GetSelected());
      break;
    case SamplingColumn::FunctionName:
      sorter = ORBIT_PROC_SORT(m_Name);
      break;
    case SamplingColumn::Exclusive:
      sorter = ORBIT_PROC_SORT(m_Exclusive);
      break;
    case SamplingColumn::Inclusive:
      sorter = ORBIT_PROC_SORT(m_Inclusive);
      break;
    case SamplingColumn::ModuleName:
      sorter = ORBIT_PROC_SORT(m_Module);
      break;
    case SamplingColumn::SourceFile:
      sorter = ORBIT_PROC_SORT(m_File);
      break;
    case SamplingColumn::SourceLine:
      sorter = ORBIT_PROC_SORT(m_Line);
      break;
    case SamplingColumn::Address:
      sorter = ORBIT_PROC_SORT(m_Address);
      break;
    default:
      break;
  }

  if (sorter) {
    std::sort(m_Indices.begin(), m_Indices.end(), sorter);
  }

  m_LastSortedColumn = a_Column;
}

//-----------------------------------------------------------------------------
std::vector<Function*> SamplingReportDataView::GetFunctionsFromIndices(
    const std::vector<int>& a_Indices) {
  std::set<Function*> functions_set;
  if (Capture::GTargetProcess != nullptr) {
    for (int index : a_Indices) {
      SampledFunction& sampled_function = GetSampledFunction(index);
      if (sampled_function.m_Function == nullptr) {
        sampled_function.m_Function =
            Capture::GTargetProcess->GetFunctionFromAddress(
                sampled_function.m_Address, false);
      }

      Function* function = sampled_function.m_Function;
      if (function != nullptr) {
        functions_set.insert(function);
      }
    }
  }

  return std::vector<Function*>(functions_set.begin(), functions_set.end());
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<Module>>
SamplingReportDataView::GetModulesFromIndices(
    const std::vector<int>& a_Indices) {
  std::vector<std::shared_ptr<Module>> modules;
  if (Capture::GTargetProcess != nullptr) {
    std::set<std::wstring> module_names;
    for (int index : a_Indices) {
      SampledFunction& sampled_function = GetSampledFunction(index);
      module_names.emplace(sampled_function.m_Module);
    }

    auto& module_map = Capture::GTargetProcess->GetNameToModulesMap();
    for (const std::wstring& module_name : module_names) {
      auto module_it = module_map.find(ws2s(ToLower(module_name)));
      if (module_it != module_map.end()) {
        modules.push_back(module_it->second);
      }
    }
  }
  return modules;
}

//-----------------------------------------------------------------------------
const std::wstring SamplingReportDataView::MENU_ACTION_SELECT = L"Hook";
const std::wstring SamplingReportDataView::MENU_ACTION_UNSELECT = L"Unhook";
const std::wstring SamplingReportDataView::MENU_ACTION_MODULES_LOAD =
    L"Load Symbols";
const std::wstring SamplingReportDataView::MENU_ACTION_DISASSEMBLY =
    L"Go to Disassembly";

//-----------------------------------------------------------------------------
std::vector<std::wstring> SamplingReportDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;
  for (const Function* function : GetFunctionsFromIndices(a_SelectedIndices)) {
    enable_select |= !function->IsSelected();
    enable_unselect |= function->IsSelected();
  }

  bool enable_load = false;
  for (const auto& module : GetModulesFromIndices(a_SelectedIndices)) {
    if (module->m_FoundPdb && !module->GetLoaded()) {
      enable_load = true;
    }
  }

  std::vector<std::wstring> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  if (enable_load) menu.emplace_back(MENU_ACTION_MODULES_LOAD);
  Append(menu, {MENU_ACTION_DISASSEMBLY});
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnContextMenu(
    const std::wstring& a_Action, int a_MenuIndex,
    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SELECT) {
    for (Function* function : GetFunctionsFromIndices(a_ItemIndices)) {
      function->Select();
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (Function* function : GetFunctionsFromIndices(a_ItemIndices)) {
      function->UnSelect();
    }
  } else if (a_Action == MENU_ACTION_MODULES_LOAD) {
    for (const auto& module : GetModulesFromIndices(a_ItemIndices)) {
      if (module->m_FoundPdb && !module->GetLoaded()) {
        GOrbitApp->EnqueueModuleToLoad(module);
      }
      GOrbitApp->LoadModules();
    }
  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    // TODO: does this action work or should we hide it?
    for (int i : a_ItemIndices) {
      SampledFunction& sampled_function = GetSampledFunction(i);
      GOrbitApp->GetDisassembly(sampled_function.m_Address, 200, 400);
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnSelect(int a_Index) {
  SampledFunction& func = GetSampledFunction(a_Index);
  m_SamplingReport->OnSelectAddress(func.m_Address, m_TID);
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::LinkDataView(DataView* a_DataView) {
  if (a_DataView->GetType() == CALLSTACK) {
    m_CallstackDataView = (CallStackDataView*)a_DataView;
    m_SamplingReport->SetCallstackDataView(m_CallstackDataView);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::SetSampledFunctions(
    const std::vector<SampledFunction>& a_Functions) {
  m_Functions = a_Functions;

  size_t numFunctions = m_Functions.size();
  m_Indices.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::SetThreadID(ThreadID a_TID) {
  m_TID = a_TID;
  if (a_TID == 0) {
    m_Name = L"All";
  } else {
    m_Name = Format(L"%d", m_TID);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnFilter(const std::wstring& a_Filter) {
  std::vector<uint32_t> indices;

  std::vector<std::wstring> tokens = Tokenize(ToLower(a_Filter));

  for (size_t i = 0; i < m_Functions.size(); ++i) {
    SampledFunction& func = m_Functions[i];
    std::wstring name = ToLower(func.m_Name);
    std::wstring module = ToLower(func.m_Module);

    bool match = true;

    for (std::wstring& filterToken : tokens) {
      if (!(name.find(filterToken) != std::wstring::npos ||
            module.find(filterToken) != std::wstring::npos)) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  m_Indices = indices;

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
const SampledFunction& SamplingReportDataView::GetSampledFunction(
    unsigned int a_Row) const {
  return m_Functions[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
SampledFunction& SamplingReportDataView::GetSampledFunction(
    unsigned int a_Row) {
  return m_Functions[m_Indices[a_Row]];
}
