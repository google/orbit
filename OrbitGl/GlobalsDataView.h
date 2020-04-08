//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitType.h"

class GlobalsDataView : public DataView {
 public:
  GlobalsDataView();

  const std::vector<std::wstring>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  const std::vector<SortingOrder>& GetColumnInitialOrders() override;
  std::vector<std::wstring> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::wstring GetValue(int a_Row, int a_Column) override;

  void OnFilter(const std::wstring& a_Filter) override;
  void ParallelFilter();
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnDataChanged() override;
  void OnAddToWatch(const std::vector<int>& a_Items);

 protected:
  Variable& GetVariable(unsigned int a_Row) const;
  std::vector<std::wstring> m_FilterTokens;

  static void InitColumnsIfNeeded();
  static std::vector<std::wstring> s_Headers;
  static std::vector<int> s_HeaderMap;
  static std::vector<float> s_HeaderRatios;
  static std::vector<SortingOrder> s_InitialOrders;
};
