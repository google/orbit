//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "LiveFunctionsDataView.h"

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "FunctionStats.h"
#include "Log.h"
#include "OrbitType.h"
#include "Pdb.h"

//-----------------------------------------------------------------------------
namespace LiveFunction {
enum Columns {
  SELECTED,
  NAME,
  COUNT,
  TIME_TOTAL,
  TIME_AVG,
  TIME_MIN,
  TIME_MAX,
  ADDRESS,
  MODULE,
  INDEX,
  NUM_EXPOSED_MEMBERS
};
}

//-----------------------------------------------------------------------------
LiveFunctionsDataView::LiveFunctionsDataView() {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());
  GOrbitApp->RegisterLiveFunctionsDataView(this);
  m_UpdatePeriodMs = 300;
  m_LastSortedColumn = 3; /*Count*/
  GetColumnHeaders();
  OnDataChanged();
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> LiveFunctionsDataView::s_Headers;
std::vector<int> LiveFunctionsDataView::s_HeaderMap;
std::vector<float> LiveFunctionsDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> LiveFunctionsDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back(L"Hooked");
    s_HeaderMap.push_back(LiveFunction::SELECTED);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Index");
    s_HeaderMap.push_back(LiveFunction::INDEX);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Function");
    s_HeaderMap.push_back(LiveFunction::NAME);
    s_HeaderRatios.push_back(0.5f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Count");
    s_HeaderMap.push_back(LiveFunction::COUNT);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Total");
    s_HeaderMap.push_back(LiveFunction::TIME_TOTAL);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Avg");
    s_HeaderMap.push_back(LiveFunction::TIME_AVG);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Min");
    s_HeaderMap.push_back(LiveFunction::TIME_MIN);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Max");
    s_HeaderMap.push_back(LiveFunction::TIME_MAX);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back(L"Module");
    s_HeaderMap.push_back(LiveFunction::MODULE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Address");
    s_HeaderMap.push_back(LiveFunction::ADDRESS);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& LiveFunctionsDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& LiveFunctionsDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
LiveFunctionsDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
int LiveFunctionsDataView::GetDefaultSortingColumn() {
  return std::distance(
      s_HeaderMap.begin(),
      std::find(s_HeaderMap.begin(), s_HeaderMap.end(), LiveFunction::COUNT));
}

//-----------------------------------------------------------------------------
std::wstring LiveFunctionsDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= (int)GetNumElements()) {
    return L"";
  }

  Function& function = GetFunction(a_Row);
  const FunctionStats* stats = function.Stats();

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case LiveFunction::SELECTED:
      value = function.IsSelected() ? "X" : "-";
      break;
    case LiveFunction::INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case LiveFunction::NAME:
      value = function.PrettyName();
      break;
    case LiveFunction::COUNT:
      value = absl::StrFormat("%lu", stats->m_Count);
      break;
    case LiveFunction::TIME_TOTAL:
      value = GetPrettyTime(stats->m_TotalTimeMs);
      break;
    case LiveFunction::TIME_AVG:
      value = GetPrettyTime(stats->m_AverageTimeMs);
      break;
    case LiveFunction::TIME_MIN:
      value = GetPrettyTime(stats->m_MinMs);
      break;
    case LiveFunction::TIME_MAX:
      value = GetPrettyTime(stats->m_MaxMs);
      break;
    case LiveFunction::ADDRESS:
      value = absl::StrFormat("0x%llx", function.GetVirtualAddress());
      break;
    case LiveFunction::MODULE:
      value = function.GetPdb() != nullptr ? function.GetPdb()->GetName() : "";
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
#define ORBIT_STAT_SORT(Member)                                           \
  [&](int a, int b) {                                                     \
    return OrbitUtils::Compare(functions[a]->Stats()->Member,             \
                               functions[b]->Stats()->Member, ascending); \
  }

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnSort(int a_Column,
                                   std::optional<SortingOrder> a_NewOrder) {
  const std::vector<Function*>& functions = m_Functions;
  auto memberId = static_cast<LiveFunction::Columns>(s_HeaderMap[a_Column]);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[memberId] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[memberId] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (memberId) {
    case LiveFunction::NAME:
      sorter = ORBIT_FUNC_SORT(PrettyName());
      break;
    case LiveFunction::COUNT:
      ascending = false;
      sorter = ORBIT_STAT_SORT(m_Count);
      break;
    case LiveFunction::TIME_TOTAL:
      sorter = ORBIT_STAT_SORT(m_TotalTimeMs);
      break;
    case LiveFunction::TIME_AVG:
      sorter = ORBIT_STAT_SORT(m_AverageTimeMs);
      break;
    case LiveFunction::TIME_MIN:
      sorter = ORBIT_STAT_SORT(m_MinMs);
      break;
    case LiveFunction::TIME_MAX:
      sorter = ORBIT_STAT_SORT(m_MaxMs);
      break;
    case LiveFunction::ADDRESS:
      sorter = ORBIT_FUNC_SORT(Address());
      break;
    case LiveFunction::MODULE:
      sorter = ORBIT_FUNC_SORT(GetPdb()->GetName());
      break;
    case LiveFunction::SELECTED:
      sorter = ORBIT_FUNC_SORT(IsSelected());
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
std::wstring TOGGLE_SELECT = L"Toggle Hook";

//-----------------------------------------------------------------------------
std::vector<std::wstring> LiveFunctionsDataView::GetContextMenu(int a_Index) {
  std::vector<std::wstring> menu = {TOGGLE_SELECT};
  Append(menu, DataView::GetContextMenu(a_Index));
  return menu;
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnContextMenu(const std::wstring& a_Action,
                                          int a_MenuIndex,
                                          std::vector<int>& a_ItemIndices) {
  if (a_Action == TOGGLE_SELECT) {
    for (int i : a_ItemIndices) {
      Function& func = GetFunction(i);
      func.ToggleSelect();
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnFilter(const std::wstring& a_Filter) {
  std::vector<uint32_t> indices;

  std::vector<std::wstring> tokens = Tokenize(ToLower(a_Filter));

  for (uint32_t i = 0; i < (uint32_t)m_Functions.size(); ++i) {
    const Function* function = m_Functions[i];
    if (function) {
      std::wstring name = ToLower(s2ws(function->PrettyName()));

      bool match = true;

      for (std::wstring& filterToken : tokens) {
        if (name.find(filterToken) == std::wstring::npos) {
          match = false;
          break;
        }
      }

      if (match) {
        indices.push_back(i);
      }
    }
  }

  m_Indices = indices;

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }

  // Filter drawn textboxes
  Capture::GVisibleFunctionsMap.clear();
  for (uint32_t i = 0; i < (uint32_t)m_Indices.size(); ++i) {
    Function& func = GetFunction(i);
    Capture::GVisibleFunctionsMap[func.GetVirtualAddress()] = &func;
  }

  GOrbitApp->NeedsRedraw();
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnDataChanged() {
  size_t numFunctions = Capture::GFunctionCountMap.size();
  m_Indices.resize(numFunctions);
  for (uint32_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }

  m_Functions.clear();
  for (auto& pair : Capture::GFunctionCountMap) {
    const ULONG64& address = pair.first;
    Function* func = Capture::GSelectedFunctionsMap[address];
    m_Functions.push_back(func);
  }

  OnFilter(m_Filter);
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnTimer() {
  if (Capture::IsCapturing()) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
Function& LiveFunctionsDataView::GetFunction(unsigned int a_Row) const {
  assert(a_Row < m_Functions.size());
  assert(m_Functions[m_Indices[a_Row]]);
  return *m_Functions[m_Indices[a_Row]];
}
