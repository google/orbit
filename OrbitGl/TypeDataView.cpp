//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TypeDataView.h"

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
  m_SortingToggles.resize(Type::NUM_EXPOSED_MEMBERS, false);
  m_SortingToggles[Type::SELECTED] = true;
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
std::vector<int> TypesDataView::s_HeaderMap;
std::vector<float> TypesDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& TypesDataView::GetColumnHeaders() {
  static std::vector<std::wstring> Columns;

  if (s_HeaderMap.size() == 0) {
    Columns.push_back(L"Index");
    s_HeaderMap.push_back(Type::INDEX);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"Type");
    s_HeaderMap.push_back(Type::NAME);
    s_HeaderRatios.push_back(0.5f);
    Columns.push_back(L"Length");
    s_HeaderMap.push_back(Type::LENGTH);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"TypeId");
    s_HeaderMap.push_back(Type::TYPE_ID);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"UnModifiedId");
    s_HeaderMap.push_back(Type::TYPE_ID_UNMODIFIED);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"NumVariables");
    s_HeaderMap.push_back(Type::NUM_VARIABLES);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"NumFunctions");
    s_HeaderMap.push_back(Type::NUM_FUNCTIONS);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"NumBaseClasses");
    s_HeaderMap.push_back(Type::NUM_BASE_CLASSES);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"BaseOffset");
    s_HeaderMap.push_back(Type::BASE_OFFSET);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"Module");
    s_HeaderMap.push_back(Type::MODULE);
    s_HeaderRatios.push_back(0);
  }

  return Columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& TypesDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
std::wstring TypesDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  Type& type = GetType(a_Row);

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case Type::INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case Type::SELECTED:
      value = type.m_Selected;
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

  return s2ws(value);
}

//-----------------------------------------------------------------------------
void TypesDataView::OnFilter(const std::wstring& a_Filter) {
  ParallelFilter(a_Filter);

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, false);
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::ParallelFilter(const std::wstring& a_Filter) {
#ifdef _WIN32
  m_FilterTokens = Tokenize(ToLower(a_Filter));
  std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  std::vector<std::vector<int> > indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for(
      "TypesDataViewParallelFor", (int)types.size(),
      [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
        std::vector<int>& result = indicesArray[a_BlockIndex];
        const std::string& name = types[a_ElementIndex]->GetNameLower();

        for (std::wstring& filterToken : m_FilterTokens) {
          if (name.find(ws2s(filterToken)) == std::string::npos) {
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
void TypesDataView::OnSort(int a_Column, bool a_Toggle) {
  const std::vector<Type*>& types = Capture::GTargetProcess->GetTypes();
  auto MemberID = Type::MemberID(s_HeaderMap[a_Column]);

  if (a_Toggle) {
    m_SortingToggles[MemberID] = !m_SortingToggles[MemberID];
  }

  bool ascending = m_SortingToggles[MemberID];

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
std::wstring TYPES_SUMMARY = L"Summary";
std::wstring TYPES_DETAILS = L"Details";

//-----------------------------------------------------------------------------
std::vector<std::wstring> TypesDataView::GetContextMenu(int a_Index) {
  std::vector<std::wstring> menu = {TYPES_SUMMARY, TYPES_DETAILS};
  Append(menu, DataView::GetContextMenu(a_Index));
  return menu;
}

//-----------------------------------------------------------------------------
void TypesDataView::OnProp(std::vector<int>& a_Items) {
  for (auto& item : a_Items) {
    Type& type = GetType(item);
    std::shared_ptr<Variable> var = type.GetTemplateVariable();
    var->Print();
    GOrbitApp->SendToUiNow(L"output");
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::OnView(std::vector<int>& a_Items) {
  for (auto& item : a_Items) {
    Type& type = GetType(item);
    std::shared_ptr<Variable> var = type.GetTemplateVariable();
    var->PrintDetails();
#ifdef _WIN32
    std::shared_ptr<OrbitDiaSymbol> diaSymbol = type.GetDiaSymbol();
    OrbitDia::DiaDump(diaSymbol ? diaSymbol->m_Symbol : nullptr);
#endif
    GOrbitApp->SendToUiNow(L"output");
  }
}

//-----------------------------------------------------------------------------
void TypesDataView::OnClip(std::vector<int>& a_Items) {
  UNUSED(a_Items);
  GOrbitApp->SendToUiAsync(L"output");
}

//-----------------------------------------------------------------------------
void TypesDataView::OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                                  std::vector<int>& a_ItemIndices) {
  if (a_Action == TYPES_SUMMARY) {
    OnProp(a_ItemIndices);
  } else if (a_Action == TYPES_DETAILS) {
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
