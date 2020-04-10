//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitType.h"

class FunctionsDataView : public DataView {
 public:
  FunctionsDataView();

  const std::vector<std::string>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  const std::vector<SortingOrder>& GetColumnInitialOrders() override;
  int GetDefaultSortingColumn() override;
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;
  void OnFilter(const std::string& a_Filter) override;
  void ParallelFilter();
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnDataChanged() override;

 protected:
  virtual Function& GetFunction(unsigned int a_Row);

  std::vector<std::string> m_FilterTokens;

  static void InitColumnsIfNeeded();
  static std::vector<std::string> s_Headers;
  static std::vector<int> s_HeaderMap;
  static std::vector<float> s_HeaderRatios;
  static std::vector<SortingOrder> s_InitialOrders;

  static const std::string MENU_ACTION_SELECT;
  static const std::string MENU_ACTION_UNSELECT;
  static const std::string MENU_ACTION_VIEW;
  static const std::string MENU_ACTION_DISASSEMBLY;
  static const std::string MENU_ACTION_CREATE_RULE;
  static const std::string MENU_ACTION_SET_AS_FRAME;
};
