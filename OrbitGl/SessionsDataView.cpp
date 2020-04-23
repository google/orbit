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
SessionsDataView::SessionsDataView() : DataView(DataViewType::SESSIONS) {
  GOrbitApp->RegisterSessionsDataView(this);
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& SessionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SESSION_NAME] = {"Session", .5f, SortingOrder::Ascending};
    columns[COLUMN_PROCESS_NAME] = {"Process", .5f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string SessionsDataView::GetValue(int row, int col) {
  const std::shared_ptr<Session>& session = GetSession(row);

  switch (col) {
    case COLUMN_SESSION_NAME:
      return Path::GetFileName(session->m_FileName);
    case COLUMN_PROCESS_NAME:
      return Path::GetFileName(session->m_ProcessFullPath);
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
std::string SessionsDataView::GetToolTip(int a_Row, int /*a_Column*/) {
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
void SessionsDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (m_SortingColumn) {
    case COLUMN_SESSION_NAME:
      sorter = ORBIT_SESSION_SORT(m_FileName);
      break;
    case COLUMN_PROCESS_NAME:
      sorter = ORBIT_SESSION_SORT(m_ProcessFullPath);
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
const std::string SessionsDataView::MENU_ACTION_SESSIONS_LOAD = "Load Session";

//-----------------------------------------------------------------------------
std::vector<std::string> SessionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  std::vector<std::string> menu = {MENU_ACTION_SESSIONS_LOAD};
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnContextMenu(const std::string& a_Action,
                                     int a_MenuIndex,
                                     const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SESSIONS_LOAD) {
    for (int index : a_ItemIndices) {
      const std::shared_ptr<Session>& session = GetSession(index);
      GOrbitApp->LoadSession(session);
    }

    GOrbitApp->LoadModules();
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void SessionsDataView::DoFilter() {
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = Tokenize(ToLower(m_Filter));

  for (size_t i = 0; i < m_Sessions.size(); ++i) {
    const Session& session = *m_Sessions[i];
    std::string name = Path::GetFileName(ToLower(session.m_FileName));
    std::string path = ToLower(session.m_ProcessFullPath);

    bool match = true;

    for (std::string& filterToken : tokens) {
      if (!(name.find(filterToken) != std::string::npos ||
            path.find(filterToken) != std::string::npos)) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  m_Indices = indices;

  OnSort(m_SortingColumn, {});
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnDataChanged() {
  m_Indices.resize(m_Sessions.size());
  for (size_t i = 0; i < m_Sessions.size(); ++i) {
    m_Indices[i] = i;
  }

  DataView::OnDataChanged();
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
