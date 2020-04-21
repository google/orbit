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
LiveFunctionsDataView::LiveFunctionsDataView()
    : DataView(DataViewType::LIVE_FUNCTIONS) {
  InitSortingOrders();
  GOrbitApp->RegisterLiveFunctionsDataView(this);
  m_UpdatePeriodMs = 300;
  m_LastSortedColumn = 3; /*Count*/
  OnDataChanged();
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& LiveFunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SELECTED] = {"Hooked", .0f, SortingOrder::Descending};
    columns[COLUMN_INDEX] = {"Index", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Function", .5f, SortingOrder::Ascending};
    columns[COLUMN_COUNT] = {"Count", .0f, SortingOrder::Descending};
    columns[COLUMN_TIME_TOTAL] = {"Total", .0f, SortingOrder::Descending};
    columns[COLUMN_TIME_AVG] = {"Avg", .0f, SortingOrder::Descending};
    columns[COLUMN_TIME_MIN] = {"Min", .0f, SortingOrder::Descending};
    columns[COLUMN_TIME_MAX] = {"Max", .0f, SortingOrder::Descending};
    columns[COLUMN_MODULE] = {"Module", .0f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string LiveFunctionsDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= (int)GetNumElements()) {
    return "";
  }

  Function& function = GetFunction(a_Row);
  const FunctionStats& stats = function.Stats();

  switch (a_Column) {
    case COLUMN_SELECTED:
      return function.IsSelected() ? "X" : "-";
    case COLUMN_INDEX:
      return absl::StrFormat("%d", a_Row);
    case COLUMN_NAME:
      return function.PrettyName();
    case COLUMN_COUNT:
      return absl::StrFormat("%lu", stats.m_Count);
    case COLUMN_TIME_TOTAL:
      return GetPrettyTime(stats.m_TotalTimeMs);
    case COLUMN_TIME_AVG:
      return GetPrettyTime(stats.m_AverageTimeMs);
    case COLUMN_TIME_MIN:
      return GetPrettyTime(stats.m_MinMs);
    case COLUMN_TIME_MAX:
      return GetPrettyTime(stats.m_MaxMs);
    case COLUMN_MODULE:
      return function.GetPdb() != nullptr ? function.GetPdb()->GetName() : "";
    case COLUMN_ADDRESS:
      return absl::StrFormat("0x%llx", function.GetVirtualAddress());
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
#define ORBIT_FUNC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, \
                               ascending);                                 \
  }
#define ORBIT_STAT_SORT(Member)                                          \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(functions[a]->Stats().Member,             \
                               functions[b]->Stats().Member, ascending); \
  }

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnSort(int a_Column,
                                   std::optional<SortingOrder> a_NewOrder) {
  const std::vector<Function*>& functions = m_Functions;

  if (a_NewOrder.has_value()) {
    m_SortingOrders[a_Column] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[a_Column] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (a_Column) {
    case COLUMN_SELECTED:
      sorter = ORBIT_FUNC_SORT(IsSelected());
      break;
    case COLUMN_NAME:
      sorter = ORBIT_FUNC_SORT(PrettyName());
      break;
    case COLUMN_COUNT:
      ascending = false;
      sorter = ORBIT_STAT_SORT(m_Count);
      break;
    case COLUMN_TIME_TOTAL:
      sorter = ORBIT_STAT_SORT(m_TotalTimeMs);
      break;
    case COLUMN_TIME_AVG:
      sorter = ORBIT_STAT_SORT(m_AverageTimeMs);
      break;
    case COLUMN_TIME_MIN:
      sorter = ORBIT_STAT_SORT(m_MinMs);
      break;
    case COLUMN_TIME_MAX:
      sorter = ORBIT_STAT_SORT(m_MaxMs);
      break;
    case COLUMN_MODULE:
      sorter = ORBIT_FUNC_SORT(GetPdb()->GetName());
      break;
    case COLUMN_ADDRESS:
      sorter = ORBIT_FUNC_SORT(Address());
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
const std::string LiveFunctionsDataView::MENU_ACTION_SELECT = "Hook";
const std::string LiveFunctionsDataView::MENU_ACTION_UNSELECT = "Unhook";

//-----------------------------------------------------------------------------
std::vector<std::string> LiveFunctionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;
  for (int index : a_SelectedIndices) {
    const Function& function = GetFunction(index);
    enable_select |= !function.IsSelected();
    enable_unselect |= function.IsSelected();
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnContextMenu(
    const std::string& a_Action, int a_MenuIndex,
    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SELECT) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      function.Select();
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      function.UnSelect();
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnFilter(const std::string& a_Filter) {
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = Tokenize(ToLower(a_Filter));

  for (size_t i = 0; i < m_Functions.size(); ++i) {
    const Function* function = m_Functions[i];
    if (function != nullptr) {
      std::string name = ToLower(function->PrettyName());

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
  for (size_t i = 0; i < numFunctions; ++i) {
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
