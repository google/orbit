//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>

#include "DataView.h"

class Session;

class SessionsDataView : public DataView {
 public:
  SessionsDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_SESSION_NAME; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;
  std::string GetToolTip(int a_Row, int a_Column) override;
  std::string GetLabel() override { return "Sessions"; }

  void OnDataChanged() override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;

  void SetSessions(const std::vector<std::shared_ptr<Session> >& a_Sessions);

 protected:
  void DoSort() override;
  void DoFilter() override;
  const std::shared_ptr<Session>& GetSession(unsigned int a_Row) const;

  std::vector<std::shared_ptr<Session> > m_Sessions;

  enum ColumnIndex { COLUMN_SESSION_NAME, COLUMN_PROCESS_NAME, COLUMN_NUM };

  static const std::string MENU_ACTION_SESSIONS_LOAD;
};
