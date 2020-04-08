//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SamplingReportDataView.h"

#include <memory>

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
  SampledFunction& func = GetFunction(a_Row);

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
std::wstring SELECT = L"Hook";
std::wstring UNSELECT = L"Unhook";
std::wstring MODULES_LOAD = L"Load Symbols";
std::wstring MODULES_DIS = L"Go To Disassembly";

//-----------------------------------------------------------------------------
std::vector<std::wstring> SamplingReportDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  std::vector<std::wstring> menu = {SELECT, UNSELECT, MODULES_LOAD,
                                    MODULES_DIS};
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnContextMenu(
    const std::wstring& a_Action, int a_MenuIndex,
    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MODULES_LOAD) {
    if (Capture::GTargetProcess) {
      std::set<std::wstring> moduleNames;

      for (size_t i = 0; i < a_ItemIndices.size(); ++i) {
        SampledFunction& sampledFunc = GetFunction(a_ItemIndices[i]);
        moduleNames.insert(sampledFunc.m_Module);
      }

      auto& moduleMap = Capture::GTargetProcess->GetNameToModulesMap();
      for (const std::wstring& moduleName : moduleNames) {
        std::shared_ptr<Module> module = moduleMap[ws2s(ToLower(moduleName))];
        if (module && module->m_FoundPdb && !module->GetLoaded()) {
          GOrbitApp->EnqueueModuleToLoad(module);
        }
      }

      GOrbitApp->LoadModules();
    }
  } else if (a_Action == UNSELECT || a_Action == SELECT) {
    bool unhook = a_Action == UNSELECT;

    if (Capture::GTargetProcess) {
      for (size_t i = 0; i < a_ItemIndices.size(); ++i) {
        Function* func = nullptr;
        SampledFunction& sampledFunc = GetFunction(a_ItemIndices[i]);
        if (sampledFunc.m_Function == nullptr) {
          func = Capture::GTargetProcess->GetFunctionFromAddress(
              sampledFunc.m_Address, false);
          sampledFunc.m_Function = func;
        } else {
          func = sampledFunc.m_Function;
        }

        if (func) {
          unhook ? func->UnSelect() : func->Select();
        }
      }
    }
  } else if (a_Action == MODULES_DIS) {
    for (size_t i = 0; i < a_ItemIndices.size(); ++i) {
      SampledFunction& sampledFunc = GetFunction(a_ItemIndices[i]);
      GOrbitApp->GetDisassembly(sampledFunc.m_Address, 200, 400);
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnSelect(int a_Index) {
  SampledFunction& func = GetFunction(a_Index);
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
  for (uint32_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::SetThreadID(ThreadID a_TID) {
  m_TID = a_TID;
  m_Name = Format(L"%d", m_TID);

  if (a_TID == 0) {
    m_Name = L"All";
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnFilter(const std::wstring& a_Filter) {
  std::vector<uint32_t> indices;

  std::vector<std::wstring> tokens = Tokenize(ToLower(a_Filter));

  for (uint32_t i = 0; i < m_Functions.size(); ++i) {
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
const SampledFunction& SamplingReportDataView::GetFunction(
    unsigned int a_Row) const {
  return m_Functions[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
SampledFunction& SamplingReportDataView::GetFunction(unsigned int a_Row) {
  return m_Functions[m_Indices[a_Row]];
}
