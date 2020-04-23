//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TypesDataView.h"

#include <algorithm>

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "OrbitDia.h"
#include "OrbitProcess.h"
#include "OrbitType.h"
#include "Pdb.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
TypesDataView::TypesDataView() : DataView(DataViewType::TYPES) {
  OnDataChanged();
  GOrbitApp->RegisterTypesDataView(this);
}

//-----------------------------------------------------------------------------
void TypesDataView::OnDataChanged() {
  int numTypes = (int)Capture::GTargetProcess->GetTypes().size();
  m_Indices.resize(numTypes);
  for (int i = 0; i < numTypes; ++i) {
    m_Indices[i] = i;
  }

  DataView::OnDataChanged();
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& TypesDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_INDEX] = {"Index", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Type", .5f, SortingOrder::Ascending};
    columns[COLUMN_LENGTH] = {"Length", .0f, SortingOrder::Ascending};
    columns[COLUMN_TYPE_ID] = {"Type Id", .0f, SortingOrder::Ascending};
    columns[COLUMN_TYPE_ID_UNMOD] = {"Unmodified Id", .0f,
                                     SortingOrder::Ascending};
    columns[COLUMN_NUM_VARIABLES] = {"Num Variables", .0f,
                                     SortingOrder::Ascending};
    columns[COLUMN_NUM_FUNCTIONS] = {"Num Functions", .0f,
                                     SortingOrder::Ascending};
    columns[COLUMN_NUM_BASE_CLASSES] = {"Num Base Classes", .0f,
                                        SortingOrder::Ascending};
    columns[COLUMN_BASE_OFFSET] = {"Base Offset", .0f, SortingOrder::Ascending};
    columns[COLUMN_MODULE] = {"Module", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string TypesDataView::GetValue(int a_Row, int a_Column) {
  Type& type = GetType(a_Row);

  switch (a_Column) {
    case COLUMN_INDEX:
      return absl::StrFormat("%d", a_Row);
    case COLUMN_NAME:
      return type.GetName();
    case COLUMN_LENGTH:
      return absl::StrFormat("%d", type.m_Length);
    case COLUMN_TYPE_ID:
      return absl::StrFormat("%lu", type.m_Id);
    case COLUMN_TYPE_ID_UNMOD:
      return absl::StrFormat("%lu", type.m_UnmodifiedId);
    case COLUMN_NUM_VARIABLES:
      return absl::StrFormat("%d", type.m_NumVariables);
    case COLUMN_NUM_FUNCTIONS:
      return absl::StrFormat("%d", type.m_NumFunctions);
    case COLUMN_NUM_BASE_CLASSES:
      return absl::StrFormat("%d", type.m_NumBaseClasses);
    case COLUMN_BASE_OFFSET:
      return absl::StrFormat("%d", type.m_BaseOffset);
    case COLUMN_MODULE:
      return type.m_Pdb->GetName();
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::DoFilter() {
  m_FilterTokens = Tokenize(ToLower(m_Filter));

  // TODO: This only performs work on Windows. It is currently not an issue as
  //  globals are not supported elsewhere.
  ParallelFilter();

  OnSort(m_SortingColumn, {});
}

//-----------------------------------------------------------------------------
void TypesDataView::ParallelFilter() {
#ifdef _WIN32
  std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  std::vector<std::vector<int> > indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for("TypesDataViewParallelFor", (int)types.size(),
                        [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
                          std::vector<int>& result = indicesArray[a_BlockIndex];
                          const std::string& name =
                              types[a_ElementIndex]->GetNameLower();

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
#define ORBIT_TYPE_SORT(Member)                                                \
  [&](int a, int b) {                                                          \
    return OrbitUtils::Compare(types[a]->Member, types[b]->Member, ascending); \
  }

//-----------------------------------------------------------------------------
void TypesDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  const std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();

  switch (m_SortingColumn) {
    case COLUMN_NAME:
      sorter = ORBIT_TYPE_SORT(m_Name);
      break;
    case COLUMN_LENGTH:
      sorter = ORBIT_TYPE_SORT(m_Length);
      break;
    case COLUMN_TYPE_ID:
      sorter = ORBIT_TYPE_SORT(m_Id);
      break;
    case COLUMN_TYPE_ID_UNMOD:
      sorter = ORBIT_TYPE_SORT(m_UnmodifiedId);
      break;
    case COLUMN_NUM_VARIABLES:
      sorter = ORBIT_TYPE_SORT(m_NumVariables);
      break;
    case COLUMN_NUM_FUNCTIONS:
      sorter = ORBIT_TYPE_SORT(m_NumFunctions);
      break;
    case COLUMN_NUM_BASE_CLASSES:
      sorter = ORBIT_TYPE_SORT(m_NumBaseClasses);
      break;
    case COLUMN_BASE_OFFSET:
      sorter = ORBIT_TYPE_SORT(m_BaseOffset);
      break;
    case COLUMN_MODULE:
      sorter = ORBIT_TYPE_SORT(m_Pdb->GetName());
      break;
    default:
      break;
  }

  if (sorter) {
    std::sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
const std::string TypesDataView::MENU_ACTION_SUMMARY = "Summary";
const std::string TypesDataView::MENU_ACTION_DETAILS = "Details";

//-----------------------------------------------------------------------------
std::vector<std::string> TypesDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  std::vector<std::string> menu = {MENU_ACTION_SUMMARY, MENU_ACTION_DETAILS};
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void TypesDataView::OnProp(const std::vector<int>& a_Items) {
  for (auto& item : a_Items) {
    Type& type = GetType(item);
    std::shared_ptr<Variable> var = type.GetTemplateVariable();
    var->Print();
    GOrbitApp->SendToUiNow("output");
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::OnView(const std::vector<int>& a_Items) {
  for (auto& item : a_Items) {
    Type& type = GetType(item);
    std::shared_ptr<Variable> var = type.GetTemplateVariable();
    var->PrintDetails();
#ifdef _WIN32
    std::shared_ptr<OrbitDiaSymbol> diaSymbol = type.GetDiaSymbol();
    OrbitDia::DiaDump(diaSymbol ? diaSymbol->m_Symbol : nullptr);
#endif
    GOrbitApp->SendToUiNow("output");
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::OnClip(const std::vector<int>& a_Items) {
  UNUSED(a_Items);
  GOrbitApp->SendToUiAsync("output");
}

//-----------------------------------------------------------------------------
void TypesDataView::OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                                  const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SUMMARY) {
    OnProp(a_ItemIndices);
  } else if (a_Action == MENU_ACTION_DETAILS) {
    OnView(a_ItemIndices);
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
Type& TypesDataView::GetType(unsigned int a_Row) const {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());
  std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();
  return *types[m_Indices[a_Row]];
}
