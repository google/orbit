//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SessionsDataView.h"

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "ModulesDataView.h"
#include "OrbitSession.h"
#include "OrbitType.h"
#include "Pdb.h"

//-----------------------------------------------------------------------------
SessionsDataView::SessionsDataView() {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());
  GOrbitApp->RegisterSessionsDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> SessionsDataView::s_Headers;
std::vector<float> SessionsDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> SessionsDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void SessionsDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back(L"Session");
    s_HeaderRatios.push_back(0.5);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back(L"Process");
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    // s_Headers.emplace_back(L"LastUsed");
    // s_HeaderRatios.push_back(0);
    // s_InitialOrders.push_back(DescendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& SessionsDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& SessionsDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
SessionsDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
std::wstring SessionsDataView::GetValue(int row, int col) {
  std::string value;

  const std::shared_ptr<Session>& session = GetSession(row);

  switch (col) {
    case SDV_SessionName:
      value = Path::GetFileName(session->m_FileName);
      break;
    case SDV_ProcessName:
      value = Path::GetFileName(session->m_ProcessFullPath);
      break;
      /*case SDV_LastUsed:
          value = "LastUsed"; break;*/
      break;
    default:
      break;
  }

  return s2ws(value);
}

//-----------------------------------------------------------------------------
std::wstring SessionsDataView::GetToolTip(int a_Row, int /*a_Column*/) {
  const Session& session = *GetSession(a_Row);
  return s2ws(session.m_FileName);
}

//-----------------------------------------------------------------------------
#define ORBIT_SESSION_SORT(Member)                                           \
  [&](int a, int b) {                                                        \
    return OrbitUtils::Compare(m_Sessions[a]->Member, m_Sessions[b]->Member, \
                               ascending);                                   \
  }

//-----------------------------------------------------------------------------
void SessionsDataView::OnSort(int a_Column,
                              std::optional<SortingOrder> a_NewOrder) {
  auto pdvColumn = static_cast<SdvColumn>(a_Column);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[pdvColumn] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[pdvColumn] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (pdvColumn) {
    case SDV_SessionName:
      sorter = ORBIT_SESSION_SORT(m_FileName);
      break;
    case SDV_ProcessName:
      sorter = ORBIT_SESSION_SORT(m_ProcessFullPath);
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
const std::wstring SessionsDataView::MENU_ACTION_SESSIONS_LOAD =
    L"Load Session";

//-----------------------------------------------------------------------------
std::vector<std::wstring> SessionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  std::vector<std::wstring> menu = {MENU_ACTION_SESSIONS_LOAD};
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnContextMenu(const std::wstring& a_Action,
                                     int a_MenuIndex,
                                     const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SESSIONS_LOAD) {
    for (int index : a_ItemIndices) {
      const std::shared_ptr<Session>& session = GetSession(index);
      if (GOrbitApp->SelectProcess(
              Path::GetFileName(session->m_ProcessFullPath))) {
        Capture::LoadSession(session);
      }
    }

    GOrbitApp->LoadModules();
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnFilter(const std::wstring& a_Filter) {
  std::vector<uint32_t> indices;

  std::vector<std::wstring> tokens = Tokenize(ToLower(a_Filter));

  for (size_t i = 0; i < m_Sessions.size(); ++i) {
    const Session& session = *m_Sessions[i];
    std::wstring name = s2ws(Path::GetFileName(ToLower(session.m_FileName)));
    std::wstring path = s2ws(ToLower(session.m_ProcessFullPath));

    bool match = true;

    for (std::wstring& filterToken : tokens) {
      if (!(name.find(filterToken) != std::wstring::npos ||
            path.find(filterToken) != std::wstring::npos)) {
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
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnDataChanged() {
  m_Indices.resize(m_Sessions.size());
  for (size_t i = 0; i < m_Sessions.size(); ++i) {
    m_Indices[i] = i;
  }

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
void SessionsDataView::SetSessions(
    const std::vector<std::shared_ptr<Session> >& a_Sessions) {
  m_Sessions = a_Sessions;
  OnDataChanged();
}

//-----------------------------------------------------------------------------
const std::shared_ptr<Session>& SessionsDataView::GetSession(
    unsigned int a_Row) const {
  return m_Sessions[m_Indices[a_Row]];
}
