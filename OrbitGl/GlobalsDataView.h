//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitType.h"

class GlobalsDataView : public DataView {
 public:
  GlobalsDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_ADDRESS; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;

  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnDataChanged() override;

 protected:
  void DoSort() override;
  void DoFilter() override;
  void ParallelFilter();
  Variable& GetVariable(unsigned int a_Row) const;
  void AddToWatch(const std::vector<int>& a_Items);

  std::vector<std::string> m_FilterTokens;

  enum ColumnIndex {
    COLUMN_INDEX,
    COLUMN_NAME,
    COLUMN_TYPE,
    COLUMN_FILE,
    COLUMN_LINE,
    COLUMN_MODULE,
    COLUMN_ADDRESS,
    COLUMN_NUM
  };

  static const std::string MENU_ACTION_TYPES_MENU_WATCH;
};
