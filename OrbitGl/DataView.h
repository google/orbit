//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "DataViewTypes.h"

//-----------------------------------------------------------------------------
class DataView {
 public:
  enum SortingOrder {
    AscendingOrder = 0,
    DescendingOrder = 1,
  };

  DataView()
      : m_LastSortedColumn(-1),
        m_UpdatePeriodMs(-1),
        m_SelectedIndex(-1),
        m_Type(INVALID) {}

  virtual ~DataView();

  static DataView* Create(DataViewType a_Type);

  virtual void SetAsMainInstance() {}
  virtual const std::vector<std::wstring>& GetColumnHeaders();
  virtual const std::vector<float>& GetColumnHeadersRatios();
  virtual bool IsSortingAllowed() { return true; }
  virtual const std::vector<SortingOrder>& GetColumnInitialOrders();
  virtual int GetDefaultSortingColumn() { return 0; }
  virtual std::vector<std::wstring> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices);
  virtual size_t GetNumElements() { return m_Indices.size(); }
  virtual std::wstring GetValue(int /*a_Row*/, int /*a_Column*/) { return L""; }
  virtual std::wstring GetToolTip(int /*a_Row*/, int /*a_Column*/) {
    return L"";
  }
  virtual void SetFilter(const std::wstring& a_Filter) {
    m_Filter = a_Filter;
    OnFilter(a_Filter);
  }
  virtual void OnFilter(const std::wstring& /*a_Filter*/) {}
  virtual void OnSort(int /*a_Column*/,
                      std::optional<SortingOrder> /*a_NewOrder*/) {}
  virtual void OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                             const std::vector<int>& a_ItemIndices);
  virtual void OnItemActivated() {}
  virtual void OnSelect(int /*a_Index*/) {}
  virtual int GetSelectedIndex() { return m_SelectedIndex; }
  virtual void OnDataChanged() {}
  virtual void OnTimer() {}
  virtual bool WantsDisplayColor() { return false; }
  virtual bool GetDisplayColor(int /*a_Row*/, int /*a_Column*/,
                               unsigned char& /*r*/, unsigned char& /*g*/,
                               unsigned char& /*b*/) {
    return false;
  }
  virtual const std::wstring& GetName() {
    static std::wstring s(L"noname");
    return s;
  }
  virtual std::wstring GetLabel() { return L""; }
  virtual void SetGlPanel(class GlPanel* /*a_GlPanel*/) {}
  virtual void LinkDataView(DataView* /*a_DataView*/) {}
  virtual bool ScrollToBottom() { return false; }
  virtual bool SkipTimer() { return false; }
  virtual void ExportCSV(const std::wstring& a_FileName);
  virtual void CopySelection(const std::vector<int>& selection);

  int GetUpdatePeriodMs() const { return m_UpdatePeriodMs; }
  DataViewType GetType() const { return m_Type; }

 protected:
  std::vector<uint32_t> m_Indices;
  std::vector<SortingOrder> m_SortingOrders;
  int m_LastSortedColumn;
  std::wstring m_Filter;
  int m_UpdatePeriodMs;
  int m_SelectedIndex;
  DataViewType m_Type;

  static const std::wstring MENU_ACTION_COPY_SELECTION;
  static const std::wstring MENU_ACTION_EXPORT_TO_CSV;
};
