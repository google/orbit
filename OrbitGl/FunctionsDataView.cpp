//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "FunctionsDataView.h"

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "Log.h"
#include "OrbitProcess.h"
#include "OrbitType.h"
#include "Pdb.h"
#include "RuleEditor.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
FunctionsDataView::FunctionsDataView() {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());
  GOrbitApp->RegisterFunctionsDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> FunctionsDataView::s_Headers;
std::vector<int> FunctionsDataView::s_HeaderMap;
std::vector<float> FunctionsDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> FunctionsDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void FunctionsDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back(L"Hooked");
    s_HeaderMap.push_back(Function::SELECTED);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Index");
    s_HeaderMap.push_back(Function::INDEX);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Function");
    s_HeaderMap.push_back(Function::NAME);
    s_HeaderRatios.push_back(0.5f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Size");
    s_HeaderMap.push_back(Function::SIZE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"File");
    s_HeaderMap.push_back(Function::FILE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Line");
    s_HeaderMap.push_back(Function::LINE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Module");
    s_HeaderMap.push_back(Function::MODULE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Address");
    s_HeaderMap.push_back(Function::ADDRESS);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Conv");
    s_HeaderMap.push_back(Function::CALL_CONV);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& FunctionsDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& FunctionsDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
FunctionsDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
int FunctionsDataView::GetDefaultSortingColumn() {
  return std::distance(
      s_HeaderMap.begin(),
      std::find(s_HeaderMap.begin(), s_HeaderMap.end(), Function::ADDRESS));
}

//-----------------------------------------------------------------------------
std::wstring FunctionsDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  if (a_Row >= (int)GetNumElements()) {
    return L"";
  }

  Function& function = GetFunction(a_Row);

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case Function::INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case Function::SELECTED:
      value = function.IsSelected() ? "X" : "-";
      break;
    case Function::NAME:
      value = function.PrettyName();
      break;
    case Function::ADDRESS:
      value = absl::StrFormat("0x%llx", function.GetVirtualAddress());
      break;
    case Function::FILE:
      value = function.File();
      break;
    case Function::MODULE:
      value = function.GetPdb() != nullptr ? function.GetPdb()->GetName() : "";
      break;
    case Function::LINE:
      value = absl::StrFormat("%i", function.Line());
      break;
    case Function::SIZE:
      value = absl::StrFormat("%lu", function.Size());
      break;
    case Function::CALL_CONV:
      value = function.GetCallingConventionString();
      break;
    default:
      break;
  }

  return s2ws(value);
}

//-----------------------------------------------------------------------------
#define ORBIT_FUNC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, \
                               ascending);                                 \
  }

//-----------------------------------------------------------------------------
void FunctionsDataView::OnSort(int a_Column,
                               std::optional<SortingOrder> a_NewOrder) {
  if (!IsSortingAllowed()) {
    return;
  }

  const std::vector<Function*>& functions =
      Capture::GTargetProcess->GetFunctions();
  auto memberId = static_cast<Function::MemberID>(s_HeaderMap[a_Column]);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[a_Column] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[a_Column] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (memberId) {
    case Function::NAME:
      sorter = ORBIT_FUNC_SORT(PrettyName());
      break;
    case Function::ADDRESS:
      sorter = ORBIT_FUNC_SORT(Address());
      break;
    case Function::MODULE:
      sorter = ORBIT_FUNC_SORT(GetPdb()->GetName());
      break;
    case Function::FILE:
      sorter = ORBIT_FUNC_SORT(File());
      break;
    case Function::LINE:
      sorter = ORBIT_FUNC_SORT(Line());
      break;
    case Function::SIZE:
      sorter = ORBIT_FUNC_SORT(Size());
      break;
    case Function::SELECTED:
      sorter = ORBIT_FUNC_SORT(IsSelected());
      break;
    case Function::CALL_CONV:
      sorter = ORBIT_FUNC_SORT(CallingConvention());
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
const std::wstring FunctionsDataView::MENU_ACTION_SELECT = L"Hook";
const std::wstring FunctionsDataView::MENU_ACTION_UNSELECT = L"Unhook";
const std::wstring FunctionsDataView::MENU_ACTION_VIEW = L"Visualize";
const std::wstring FunctionsDataView::MENU_ACTION_DISASSEMBLY =
    L"Go to Disassembly";
const std::wstring FunctionsDataView::MENU_ACTION_CREATE_RULE = L"Create Rule";
const std::wstring FunctionsDataView::MENU_ACTION_SET_AS_FRAME =
    L"Set as Main Frame";

//-----------------------------------------------------------------------------
std::vector<std::wstring> FunctionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;
  for (int index : a_SelectedIndices) {
    const Function& function = GetFunction(index);
    enable_select |= !function.IsSelected();
    enable_unselect |= function.IsSelected();
  }

  bool enable_create_rule = a_SelectedIndices.size() == 1;

  std::vector<std::wstring> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  Append(menu, {MENU_ACTION_VIEW, MENU_ACTION_DISASSEMBLY});
  if (enable_create_rule) menu.emplace_back(MENU_ACTION_CREATE_RULE);
  // TODO: MENU_ACTION_SET_AS_FRAME is never shown, should it be removed?
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnContextMenu(const std::wstring& a_Action,
                                      int a_MenuIndex,
                                      const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SELECT) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      function.Select();
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (int i : a_ItemIndices) {
      Function& func = GetFunction(i);
      func.UnSelect();
    }
  } else if (a_Action == MENU_ACTION_VIEW) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      function.Print();
    }

    GOrbitApp->SendToUiNow(L"output");
  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    // TODO: does this action work or should we hide it?
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      function.GetDisassembly();
    }
  } else if (a_Action == MENU_ACTION_CREATE_RULE) {
    if (a_ItemIndices.size() == 1) {
      Function& function = GetFunction(0);
      GOrbitApp->LaunchRuleEditor(&function);
    }
  } else if (a_Action == MENU_ACTION_SET_AS_FRAME) {
    if (a_ItemIndices.size() == 1) {
      Function& function = GetFunction(0);
      function.SetAsMainFrameFunction();
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnFilter(const std::wstring& a_Filter) {
  m_FilterTokens = Tokenize(ToLower(a_Filter));

#ifdef WIN32
  ParallelFilter();
#else
  // TODO: port parallel filtering
  std::vector<uint32_t> indices;
  std::vector<std::wstring> tokens = Tokenize(ToLower(a_Filter));
  std::vector<Function*>& functions = Capture::GTargetProcess->GetFunctions();
  for (int i = 0; i < (int)functions.size(); ++i) {
    Function* function = functions[i];
    std::string name = function->Lower() + function->GetPdb()->GetName();

    bool match = true;

    for (std::wstring& filterToken : tokens) {
      if (name.find(ws2s(filterToken)) == std::string::npos) {
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
#endif
}

//-----------------------------------------------------------------------------
void FunctionsDataView::ParallelFilter() {
#ifdef _WIN32
  std::vector<Function*>& functions = Capture::GTargetProcess->GetFunctions();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  // int numWorkers = oqpi::thread::hardware_concurrency();
  std::vector<std::vector<int> > indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for(
      "FunctionsDataViewParallelFor", (int)functions.size(),
      [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
        std::vector<int>& result = indicesArray[a_BlockIndex];
        const std::wstring& name = s2ws(functions[a_ElementIndex]->Lower());
        const std::wstring& file = s2ws(functions[a_ElementIndex]->File());

        for (std::wstring& filterToken : m_FilterTokens) {
          if (name.find(filterToken) == std::wstring::npos &&
              file.find(filterToken) == std::wstring::npos) {
            return;
          }
        }

        result.push_back(a_ElementIndex);
      });

  std::set<int> indicesSet;
  for (std::vector<int>& results : indicesArray) {
    for (int index : results) {
      indicesSet.insert(index);
    }
  }

  m_Indices.clear();
  for (int i : indicesSet) {
    m_Indices.push_back(i);
  }
#endif
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnDataChanged() {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  size_t numFunctions = Capture::GTargetProcess->GetFunctions().size();
  m_Indices.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
Function& FunctionsDataView::GetFunction(unsigned int a_Row) {
  std::vector<Function*>& functions = Capture::GTargetProcess->GetFunctions();
  return *functions[m_Indices[a_Row]];
}
