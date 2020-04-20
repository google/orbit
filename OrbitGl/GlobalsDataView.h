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

  void OnFilter(const std::string& a_Filter) override;
  void ParallelFilter();
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnDataChanged() override;
  void OnAddToWatch(const std::vector<int>& a_Items);

 protected:
  Variable& GetVariable(unsigned int a_Row) const;
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
