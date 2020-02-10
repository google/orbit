//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlobalDataView.h"

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
  m_SortingToggles.resize(Variable::NUM_EXPOSED_MEMBERS, false);
  m_SortingToggles[Variable::SELECTED] = true;
  OnDataChanged();

  GOrbitApp->RegisterGlobalsDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<int> GlobalsDataView::s_HeaderMap;
std::vector<float> GlobalsDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& GlobalsDataView::GetColumnHeaders() {
  static std::vector<std::wstring> Columns;

  if (s_HeaderMap.size() == 0) {
    Columns.push_back(L"Index");
    s_HeaderMap.push_back(Variable::INDEX);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"Variable");
    s_HeaderMap.push_back(Variable::NAME);
    s_HeaderRatios.push_back(0.5f);
    Columns.push_back(L"Type");
    s_HeaderMap.push_back(Variable::TYPE);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"Address");
    s_HeaderMap.push_back(Variable::ADDRESS);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"File");
    s_HeaderMap.push_back(Variable::FILE);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"Line");
    s_HeaderMap.push_back(Variable::LINE);
    s_HeaderRatios.push_back(0);
    Columns.push_back(L"Module");
    s_HeaderMap.push_back(Variable::MODULE);
    s_HeaderRatios.push_back(0);
  }

  return Columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& GlobalsDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
std::wstring GlobalsDataView::GetValue(int a_Row, int a_Column) {
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
    /*case Variable::MODBASE:
        value = wxString::Format("0x%I64x", function.m_ModBase);  break;*/
    case Variable::ADDRESS:
      value = absl::StrFormat("0x%llx", variable.m_Address);
      break;
    case Variable::LINE:
      value = absl::StrFormat("%i", variable.m_Line);
      break;
    default:
      break;
      ;
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
void GlobalsDataView::OnSort(int a_Column, bool a_Toggle) {
  const std::vector<Variable*>& functions =
      Capture::GTargetProcess->GetGlobals();
  auto MemberID = Variable::MemberID(s_HeaderMap[a_Column]);

  if (a_Toggle) {
    m_SortingToggles[MemberID] = !m_SortingToggles[MemberID];
  }

  bool ascending = m_SortingToggles[MemberID];
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (MemberID) {
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
std::wstring TYPES_MENU_WATCH = L"Add to watch";

//-----------------------------------------------------------------------------
std::vector<std::wstring> GlobalsDataView::GetContextMenu(int a_Index) {
  std::vector<std::wstring> menu = {TYPES_MENU_WATCH};
  Append(menu, DataView::GetContextMenu(a_Index));
  return menu;
}

//-----------------------------------------------------------------------------
void GlobalsDataView::OnContextMenu(const std::wstring& a_Action,
                                    int a_MenuIndex,
                                    std::vector<int>& a_ItemIndices) {
  if (a_Action == TYPES_MENU_WATCH) {
    OnAddToWatch(a_ItemIndices);
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void GlobalsDataView::OnAddToWatch(std::vector<int>& a_Items) {
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
void GlobalsDataView::OnFilter(const std::wstring& a_Filter) {
  m_FilterTokens = Tokenize(ToLower(a_Filter));

  ParallelFilter();

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, false);
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

  oqpi_tk::parallel_for(
      "FunctionsDataViewParallelFor", (int)globals.size(),
      [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
        std::vector<int>& result = indicesArray[a_BlockIndex];
        const std::string& name = globals[a_ElementIndex]->FilterString();

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
void GlobalsDataView::OnDataChanged() {
  size_t numGlobals = Capture::GTargetProcess->GetGlobals().size();
  m_Indices.resize(numGlobals);
  for (uint32_t i = 0; i < numGlobals; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
Variable& GlobalsDataView::GetVariable(unsigned int a_Row) const {
  std::vector<Variable*>& globals = Capture::GTargetProcess->GetGlobals();
  return *globals[m_Indices[a_Row]];
}
