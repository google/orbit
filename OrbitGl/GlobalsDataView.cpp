//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlobalsDataView.h"

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "Log.h"
#include "OrbitProcess.h"
#include "OrbitType.h"
#include "Pdb.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
GlobalsDataView::GlobalsDataView() {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());
  OnDataChanged();

  GOrbitApp->RegisterGlobalsDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<std::string> GlobalsDataView::s_Headers;
std::vector<int> GlobalsDataView::s_HeaderMap;
std::vector<float> GlobalsDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> GlobalsDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void GlobalsDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back("Index");
    s_HeaderMap.push_back(Variable::INDEX);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Variable");
    s_HeaderMap.push_back(Variable::NAME);
    s_HeaderRatios.push_back(0.5f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Type");
    s_HeaderMap.push_back(Variable::TYPE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Address");
    s_HeaderMap.push_back(Variable::ADDRESS);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("File");
    s_HeaderMap.push_back(Variable::FILE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Line");
    s_HeaderMap.push_back(Variable::LINE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Module");
    s_HeaderMap.push_back(Variable::MODULE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::string>& GlobalsDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& GlobalsDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
GlobalsDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
std::string GlobalsDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  const Variable& variable = GetVariable(a_Row);

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case Variable::INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case Variable::SELECTED:
      value = variable.m_Selected ? "*" : "";
      break;
    case Variable::NAME:
      value = variable.m_Name;
      break;
    case Variable::TYPE:
      value = variable.m_Type;
      break;
    case Variable::FILE:
      value = variable.m_File;
      break;
    case Variable::MODULE:
      value = variable.m_Pdb->GetName();
      break;
    case Variable::ADDRESS:
      value = absl::StrFormat("0x%llx", variable.m_Address);
      break;
    case Variable::LINE:
      value = absl::StrFormat("%i", variable.m_Line);
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
void GlobalsDataView::OnSort(int a_Column,
                             std::optional<SortingOrder> a_NewOrder) {
  const std::vector<Variable*>& functions =
      Capture::GTargetProcess->GetGlobals();
  auto memberId = static_cast<Variable::MemberID>(s_HeaderMap[a_Column]);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[a_Column] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[a_Column] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (memberId) {
    case Variable::NAME:
      sorter = ORBIT_FUNC_SORT(m_Name);
      break;
    case Variable::ADDRESS:
      sorter = ORBIT_FUNC_SORT(m_Address);
      break;
    case Variable::TYPE:
      sorter = ORBIT_FUNC_SORT(m_Type);
      break;
    case Variable::MODULE:
      sorter = ORBIT_FUNC_SORT(m_Pdb->GetName());
      break;
    case Variable::FILE:
      sorter = ORBIT_FUNC_SORT(m_File);
      break;
    case Variable::SELECTED:
      sorter = ORBIT_FUNC_SORT(m_Selected);
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
const std::string GlobalsDataView::MENU_ACTION_TYPES_MENU_WATCH =
    "Add to watch";

//-----------------------------------------------------------------------------
std::vector<std::string> GlobalsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  std::vector<std::string> menu = {MENU_ACTION_TYPES_MENU_WATCH};
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void GlobalsDataView::OnContextMenu(const std::string& a_Action,
                                    int a_MenuIndex,
                                    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_TYPES_MENU_WATCH) {
    OnAddToWatch(a_ItemIndices);
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void GlobalsDataView::OnAddToWatch(const std::vector<int>& a_Items) {
  for (auto& item : a_Items) {
    Variable& variable = GetVariable(item);
    variable.Populate();
    std::shared_ptr<Variable> var;

    Type* type = variable.GetType();
    if (type && type->HasMembers()) {
      var = type->GenerateVariable(variable.m_Address, &variable.m_Name);
      var->Print();
    } else {
      var = std::make_shared<Variable>(variable);
    }

    Capture::GTargetProcess->AddWatchedVariable(var);
    GOrbitApp->AddWatchedVariable(var.get());
  }
}

//-----------------------------------------------------------------------------
void GlobalsDataView::OnFilter(const std::string& a_Filter) {
  m_FilterTokens = Tokenize(ToLower(a_Filter));

  ParallelFilter();

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
void GlobalsDataView::ParallelFilter() {
#ifdef _WIN32
  const std::vector<Variable*>& globals = Capture::GTargetProcess->GetGlobals();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  std::vector<std::vector<int> > indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for("FunctionsDataViewParallelFor", (int)globals.size(),
                        [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
                          std::vector<int>& result = indicesArray[a_BlockIndex];
                          const std::string& name =
                              globals[a_ElementIndex]->FilterString();

                          for (std::string& filterToken : m_FilterTokens) {
                            if (name.find(filterToken) == std::string::npos) {
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
void GlobalsDataView::OnDataChanged() {
  size_t numGlobals = Capture::GTargetProcess->GetGlobals().size();
  m_Indices.resize(numGlobals);
  for (size_t i = 0; i < numGlobals; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
Variable& GlobalsDataView::GetVariable(unsigned int a_Row) const {
  std::vector<Variable*>& globals = Capture::GTargetProcess->GetGlobals();
  return *globals[m_Indices[a_Row]];
}
