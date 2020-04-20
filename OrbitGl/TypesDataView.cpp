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
TypesDataView::TypesDataView() {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());
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
}

//-----------------------------------------------------------------------------
std::vector<std::string> TypesDataView::s_Headers;
std::vector<int> TypesDataView::s_HeaderMap;
std::vector<float> TypesDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> TypesDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void TypesDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back("Index");
    s_HeaderMap.push_back(Type::INDEX);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Type");
    s_HeaderMap.push_back(Type::NAME);
    s_HeaderRatios.push_back(0.5f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Length");
    s_HeaderMap.push_back(Type::LENGTH);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("TypeId");
    s_HeaderMap.push_back(Type::TYPE_ID);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("UnModifiedId");
    s_HeaderMap.push_back(Type::TYPE_ID_UNMODIFIED);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("NumVariables");
    s_HeaderMap.push_back(Type::NUM_VARIABLES);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("NumFunctions");
    s_HeaderMap.push_back(Type::NUM_FUNCTIONS);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("NumBaseClasses");
    s_HeaderMap.push_back(Type::NUM_BASE_CLASSES);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("BaseOffset");
    s_HeaderMap.push_back(Type::BASE_OFFSET);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Module");
    s_HeaderMap.push_back(Type::MODULE);
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::string>& TypesDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& TypesDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
TypesDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
std::string TypesDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  Type& type = GetType(a_Row);

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case Type::INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case Type::SELECTED:
      value = type.m_Selected ? "X" : "-";
      break;
    case Type::NAME:
      value = type.GetName();
      break;
    case Type::LENGTH:
      value = absl::StrFormat("%d", type.m_Length);
      break;
    case Type::TYPE_ID:
      value = absl::StrFormat("%lu", type.m_Id);
      break;
    case Type::TYPE_ID_UNMODIFIED:
      value = absl::StrFormat("%lu", type.m_UnmodifiedId);
      break;
    case Type::NUM_VARIABLES:
      value = absl::StrFormat("%d", type.m_NumVariables);
      break;
    case Type::NUM_FUNCTIONS:
      value = absl::StrFormat("%d", type.m_NumFunctions);
      break;
    case Type::NUM_BASE_CLASSES:
      value = absl::StrFormat("%d", type.m_NumBaseClasses);
      break;
    case Type::BASE_OFFSET:
      value = absl::StrFormat("%d", type.m_BaseOffset);
      break;
    case Type::MODULE:
      value = type.m_Pdb->GetName();
      break;
    default:
      break;
  }

  return value;
}

//-----------------------------------------------------------------------------
void TypesDataView::OnFilter(const std::string& a_Filter) {
  ParallelFilter(a_Filter);

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::ParallelFilter(const std::string& a_Filter) {
#ifdef _WIN32
  m_FilterTokens = Tokenize(ToLower(a_Filter));
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
#else
  UNUSED(a_Filter);
#endif
}

//-----------------------------------------------------------------------------
#define ORBIT_TYPE_SORT(Member)                                                \
  [&](int a, int b) {                                                          \
    return OrbitUtils::Compare(types[a]->Member, types[b]->Member, ascending); \
  }

//-----------------------------------------------------------------------------
void TypesDataView::OnSort(int a_Column,
                           std::optional<SortingOrder> a_NewOrder) {
  const std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();
  auto MemberID = static_cast<Type::MemberID>(s_HeaderMap[a_Column]);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[a_Column] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[a_Column] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (MemberID) {
    case Type::NAME:
      sorter = ORBIT_TYPE_SORT(m_Name);
      break;
    case Type::LENGTH:
      sorter = ORBIT_TYPE_SORT(m_Length);
      break;
    case Type::TYPE_ID:
      sorter = ORBIT_TYPE_SORT(m_Id);
      break;
    case Type::TYPE_ID_UNMODIFIED:
      sorter = ORBIT_TYPE_SORT(m_UnmodifiedId);
      break;
    case Type::NUM_VARIABLES:
      sorter = ORBIT_TYPE_SORT(m_NumVariables);
      break;
    case Type::NUM_FUNCTIONS:
      sorter = ORBIT_TYPE_SORT(m_NumFunctions);
      break;
    case Type::NUM_BASE_CLASSES:
      sorter = ORBIT_TYPE_SORT(m_NumBaseClasses);
      break;
    case Type::BASE_OFFSET:
      sorter = ORBIT_TYPE_SORT(m_BaseOffset);
      break;
    case Type::MODULE:
      sorter = ORBIT_TYPE_SORT(m_Pdb->GetName());
      break;
    case Type::SELECTED:
      sorter = ORBIT_TYPE_SORT(m_Selected);
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
  std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();
  return *types[m_Indices[a_Row]];
}
