//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitType.h"

//-----------------------------------------------------------------------------
class LiveFunctionsDataView : public DataView {
 public:
  LiveFunctionsDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_COUNT; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;

  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnDataChanged() override;
  void OnTimer() override;

 protected:
  void DoFilter() override;
  void DoSort() override;
  Function& GetFunction(unsigned int a_Row) const;

  std::vector<Function*> m_Functions;

  enum ColumnIndex {
    COLUMN_SELECTED,
    COLUMN_INDEX,
    COLUMN_NAME,
    COLUMN_COUNT,
    COLUMN_TIME_TOTAL,
    COLUMN_TIME_AVG,
    COLUMN_TIME_MIN,
    COLUMN_TIME_MAX,
    COLUMN_MODULE,
    COLUMN_ADDRESS,
    COLUMN_NUM
  };

  static const std::string MENU_ACTION_SELECT;
  static const std::string MENU_ACTION_UNSELECT;
};
