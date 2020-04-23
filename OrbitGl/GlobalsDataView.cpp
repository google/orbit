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
GlobalsDataView::GlobalsDataView() : DataView(DataViewType::GLOBALS) {
  OnDataChanged();
  GOrbitApp->RegisterGlobalsDataView(this);
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& GlobalsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_INDEX] = {"Index", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Variable", .5f, SortingOrder::Ascending};
    columns[COLUMN_TYPE] = {"Type", .0f, SortingOrder::Ascending};
    columns[COLUMN_FILE] = {"File", .0f, SortingOrder::Ascending};
    columns[COLUMN_LINE] = {"Line", .0f, SortingOrder::Ascending};
    columns[COLUMN_MODULE] = {"Module", .0f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string GlobalsDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  const Variable& variable = GetVariable(a_Row);

  switch (a_Column) {
    case COLUMN_INDEX:
      return absl::StrFormat("%d", a_Row);
    case COLUMN_NAME:
      return variable.m_Name;
    case COLUMN_TYPE:
      return variable.m_Type;
    case COLUMN_FILE:
      return variable.m_File;
    case COLUMN_LINE:
      return absl::StrFormat("%i", variable.m_Line);
    case COLUMN_MODULE:
      return variable.m_Pdb->GetName();
    case COLUMN_ADDRESS:
      return absl::StrFormat("0x%llx", variable.m_Address);
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

//-----------------------------------------------------------------------------
void GlobalsDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  const std::vector<Variable*>& functions =
      Capture::GTargetProcess->GetGlobals();

  switch (m_SortingColumn) {
    case COLUMN_NAME:
      sorter = ORBIT_FUNC_SORT(m_Name);
      break;
    case COLUMN_TYPE:
      sorter = ORBIT_FUNC_SORT(m_Type);
      break;
    case COLUMN_FILE:
      sorter = ORBIT_FUNC_SORT(m_File);
      break;
    case COLUMN_LINE:
      sorter = ORBIT_FUNC_SORT(m_Line);
      break;
    case COLUMN_MODULE:
      sorter = ORBIT_FUNC_SORT(m_Pdb->GetName());
      break;
    case COLUMN_ADDRESS:
      sorter = ORBIT_FUNC_SORT(m_Address);
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
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
    AddToWatch(a_ItemIndices);
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void GlobalsDataView::AddToWatch(const std::vector<int>& a_Items) {
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
void GlobalsDataView::DoFilter() {
  m_FilterTokens = Tokenize(ToLower(m_Filter));

  // TODO: This only performs work on Windows. It is currently not an issue as
  //  globals are not supported elsewhere.
  ParallelFilter();

  OnSort(m_SortingColumn, {});
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

  DataView::OnDataChanged();
}

//-----------------------------------------------------------------------------
Variable& GlobalsDataView::GetVariable(unsigned int a_Row) const {
  std::vector<Variable*>& globals = Capture::GTargetProcess->GetGlobals();
  return *globals[m_Indices[a_Row]];
}
