//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SessionsDataView.h"

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "ModuleDataView.h"
#include "OrbitSession.h"
#include "OrbitType.h"
#include "Pdb.h"

//-----------------------------------------------------------------------------
SessionsDataView::SessionsDataView() {
  m_SortingToggles.resize(SDV_NumColumns, false);
  GOrbitApp->RegisterSessionsDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<float> SessionsDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& SessionsDataView::GetColumnHeaders() {
  static std::vector<std::wstring> Columns;
  if (Columns.size() == 0) {
    Columns.push_back(L"Session");
    s_HeaderRatios.push_back(0.5);
    Columns.push_back(L"Process");
    s_HeaderRatios.push_back(0);
    // Columns.push_back( L"LastUsed" ); s_HeaderRatios.push_back( 0 );
  };

  return Columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& SessionsDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
std::wstring SessionsDataView::GetValue(int row, int col) {
  std::string value;

  const std::shared_ptr<Session>& session = GetSession(row);

  switch (col) {
    case SDV_SessionName:
      value = Path::GetFileName(ws2s(session->m_FileName));
      break;
    case SDV_ProcessName:
      value = Path::GetFileName(ws2s(session->m_ProcessFullPath));
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
std::wstring SessionsDataView::GetToolTip(int a_Row, int a_Column) {
  const Session& session = *GetSession(a_Row);
  return session.m_FileName;
}

//-----------------------------------------------------------------------------
#define ORBIT_SESSION_SORT(Member)                                           \
  [&](int a, int b) {                                                        \
    return OrbitUtils::Compare(m_Sessions[a]->Member, m_Sessions[b]->Member, \
                               ascending);                                   \
  }

//-----------------------------------------------------------------------------
void SessionsDataView::OnSort(int a_Column, bool a_Toggle) {
  SdvColumn pdvColumn = SdvColumn(a_Column);

  if (a_Toggle) {
    m_SortingToggles[pdvColumn] = !m_SortingToggles[pdvColumn];
  }

  bool ascending = m_SortingToggles[pdvColumn];
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
std::wstring SESSIONS_LOAD = L"Load Session";

//-----------------------------------------------------------------------------
std::vector<std::wstring> SessionsDataView::GetContextMenu(int a_Index) {
  std::vector<std::wstring> menu = {SESSIONS_LOAD};
  Append(menu, DataView::GetContextMenu(a_Index));
  return menu;
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnContextMenu(const std::wstring& a_Action,
                                     int a_MenuIndex,
                                     std::vector<int>& a_ItemIndices) {
  if (a_Action == SESSIONS_LOAD) {
    for (int index : a_ItemIndices) {
      const std::shared_ptr<Session>& session = GetSession(index);
      if (GOrbitApp->SelectProcess(
              s2ws(Path::GetFileName(ws2s(session->m_ProcessFullPath))))) {
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

  for (uint32_t i = 0; i < m_Sessions.size(); ++i) {
    const Session& session = *m_Sessions[i];
    std::wstring name =
        s2ws(Path::GetFileName(ToLower(ws2s(session.m_FileName))));
    std::wstring path = ToLower(session.m_ProcessFullPath);

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
    OnSort(m_LastSortedColumn, false);
  }
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnDataChanged() {
  m_Indices.resize(m_Sessions.size());
  for (uint32_t i = 0; i < m_Sessions.size(); ++i) {
    m_Indices[i] = i;
  }

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, false);
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
