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
  InitSortingOrders();
  GOrbitApp->RegisterFunctionsDataView(this);
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& FunctionsDataView::GetColumns() {
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
    columns[COLUMN_ADDRESS] = {"Address", .0f, SortingOrder::Ascending};
    columns[COLUMN_CALL_CONV] = {"Call conv", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string FunctionsDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  if (a_Row >= (int)GetNumElements()) {
    return "";
  }

  Function* function = GetFunction(a_Row);
  if (function == nullptr) {
    return "";
  }

  std::string value;

  switch (a_Column) {
    case COLUMN_SELECTED:
      value = function->IsSelected() ? "X" : "-";
      break;
    case COLUMN_INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case COLUMN_NAME:
      value = function->PrettyName();
      break;
    case COLUMN_SIZE:
      value = absl::StrFormat("%lu", function->Size());
      break;
    case COLUMN_FILE:
      value = function->File();
      break;
    case COLUMN_LINE:
      value = absl::StrFormat("%i", function->Line());
      break;
    case COLUMN_MODULE:
      value =
          function->GetPdb() != nullptr ? function->GetPdb()->GetName() : "";
      break;
    case COLUMN_ADDRESS:
      value = absl::StrFormat("0x%llx", function->GetVirtualAddress());
      break;
    case COLUMN_CALL_CONV:
      value = function->GetCallingConventionString();
      break;
    default:
      break;
  }

  return value;
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

  const std::vector<std::shared_ptr<Function>>& functions =
      Capture::GTargetProcess->GetFunctions();

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
    case COLUMN_SIZE:
      sorter = ORBIT_FUNC_SORT(Size());
      break;
    case COLUMN_FILE:
      sorter = ORBIT_FUNC_SORT(File());
      break;
    case COLUMN_LINE:
      sorter = ORBIT_FUNC_SORT(Line());
      break;
    case COLUMN_MODULE:
      sorter = ORBIT_FUNC_SORT(GetPdb()->GetName());
      break;
    case COLUMN_ADDRESS:
      sorter = ORBIT_FUNC_SORT(Address());
      break;
    case COLUMN_CALL_CONV:
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
const std::string FunctionsDataView::MENU_ACTION_SELECT = "Hook";
const std::string FunctionsDataView::MENU_ACTION_UNSELECT = "Unhook";
const std::string FunctionsDataView::MENU_ACTION_VIEW = "Visualize";
const std::string FunctionsDataView::MENU_ACTION_DISASSEMBLY =
    "Go to Disassembly";
const std::string FunctionsDataView::MENU_ACTION_CREATE_RULE = "Create Rule";
const std::string FunctionsDataView::MENU_ACTION_SET_AS_FRAME =
    "Set as Main Frame";

//-----------------------------------------------------------------------------
std::vector<std::string> FunctionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_view_disassembly = false;
  for (int index : a_SelectedIndices) {
    const Function* function = GetFunction(index);
    if (function == nullptr) continue;
    enable_select |= !function->IsSelected();
    enable_unselect |= function->IsSelected();
    enable_view_disassembly = true;
  }

  bool enable_create_rule = false;
  if (a_SelectedIndices.size() == 1 &&
      GetFunction(a_SelectedIndices[0]) != nullptr) {
    enable_create_rule = true;
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  if (enable_view_disassembly) {
    Append(menu, {MENU_ACTION_VIEW, MENU_ACTION_DISASSEMBLY});
  }
  if (enable_create_rule) menu.emplace_back(MENU_ACTION_CREATE_RULE);
  // TODO: MENU_ACTION_SET_AS_FRAME is never shown, should it be removed?
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnContextMenu(const std::string& a_Action,
                                      int a_MenuIndex,
                                      const std::vector<int>& a_ItemIndices) {
  std::vector<Function*> functions;
  for (int i : a_ItemIndices) {
    Function* function = GetFunction(i);
    if (function != nullptr) {
      functions.push_back(function);
    }
  }

  if (a_Action == MENU_ACTION_SELECT) {
    for (Function* function : functions) {
      function->Select();
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (Function* function : functions) {
      function->UnSelect();
    }
  } else if (a_Action == MENU_ACTION_VIEW) {
    for (Function* function : functions) {
      function->Print();
    }

    GOrbitApp->SendToUiNow("output");
  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    uint32_t pid = Capture::GTargetProcess->GetID();
    for (Function* function : functions) {
      function->GetDisassembly(pid);
    }
  } else if (a_Action == MENU_ACTION_CREATE_RULE) {
    if (functions.size() != 1) {
      return;
    }
    GOrbitApp->LaunchRuleEditor(functions[0]);
  } else if (a_Action == MENU_ACTION_SET_AS_FRAME) {
    if (functions.size() != 1) {
      return;
    }
    functions[0]->SetAsMainFrameFunction();
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnFilter(const std::string& a_Filter) {
  m_FilterTokens = Tokenize(ToLower(a_Filter));

#ifdef WIN32
  ParallelFilter();
#else
  // TODO: port parallel filtering
  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = Tokenize(ToLower(a_Filter));
  const std::vector<std::shared_ptr<Function>>& functions =
      Capture::GTargetProcess->GetFunctions();
  for (int i = 0; i < (int)functions.size(); ++i) {
    auto& function = functions[i];
    std::string name = function->Lower() + function->GetPdb()->GetName();

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

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
#endif
}

//-----------------------------------------------------------------------------
void FunctionsDataView::ParallelFilter() {
#ifdef _WIN32
  const std::vector<std::shared_ptr<Function>>& functions =
      Capture::GTargetProcess->GetFunctions();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  // int numWorkers = oqpi::thread::hardware_concurrency();
  std::vector<std::vector<int>> indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for(
      "FunctionsDataViewParallelFor", (int)functions.size(),
      [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
        std::vector<int>& result = indicesArray[a_BlockIndex];
        const std::string& name = functions[a_ElementIndex]->Lower();
        const std::string& file = functions[a_ElementIndex]->File();

        for (std::string& filterToken : m_FilterTokens) {
          if (name.find(filterToken) == std::string::npos &&
              file.find(filterToken) == std::string::npos) {
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
Function* FunctionsDataView::GetFunction(int a_Row) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());
  const std::vector<std::shared_ptr<Function>>& functions =
      Capture::GTargetProcess->GetFunctions();
  return functions[m_Indices[a_Row]].get();
}
