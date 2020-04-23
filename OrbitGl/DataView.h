//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <OrbitBase/Logging.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "DataViewTypes.h"

class DataView {
 public:
  enum class SortingOrder {
    Ascending = 0,
    Descending = 1,
  };

  struct Column {
    Column() : Column{"", .0f, SortingOrder::Ascending} {}
    Column(std::string header, float ratio, SortingOrder initial_order)
        : header{std::move(header)},
          ratio{ratio},
          initial_order{initial_order} {}
    std::string header;
    float ratio;
    SortingOrder initial_order;
  };

  explicit DataView(DataViewType type)
      : m_UpdatePeriodMs(-1), m_SelectedIndex(-1), m_Type(type) {}

  virtual ~DataView();

  static std::unique_ptr<DataView> Create(DataViewType a_Type);

  virtual void SetAsMainInstance() {}
  virtual const std::vector<Column>& GetColumns() = 0;
  virtual bool IsSortingAllowed() { return true; }
  virtual int GetDefaultSortingColumn() { return 0; }
  virtual std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices);
  virtual size_t GetNumElements() { return m_Indices.size(); }
  virtual std::string GetValue(int /*a_Row*/, int /*a_Column*/) { return ""; }
  virtual std::string GetToolTip(int /*a_Row*/, int /*a_Column*/) { return ""; }

  void OnFilter(const std::string& filter);
  void OnSort(int column, std::optional<SortingOrder> new_order);
  virtual void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                             const std::vector<int>& a_ItemIndices);
  virtual void OnItemActivated() {}
  virtual void OnSelect(int /*a_Index*/) {}
  virtual int GetSelectedIndex() { return m_SelectedIndex; }
  virtual void OnDataChanged();
  virtual void OnTimer() {}
  virtual bool WantsDisplayColor() { return false; }
  virtual bool GetDisplayColor(int /*a_Row*/, int /*a_Column*/,
                               unsigned char& /*r*/, unsigned char& /*g*/,
                               unsigned char& /*b*/) {
    return false;
  }
  virtual std::string GetLabel() { return ""; }
  virtual void SetGlPanel(class GlPanel* /*a_GlPanel*/) {}
  virtual void LinkDataView(DataView* /*a_DataView*/) {}
  virtual bool ScrollToBottom() { return false; }
  virtual bool SkipTimer() { return false; }
  virtual void ExportCSV(const std::string& a_FileName);
  virtual void CopySelection(const std::vector<int>& selection);

  int GetUpdatePeriodMs() const { return m_UpdatePeriodMs; }
  DataViewType GetType() const { return m_Type; }

 protected:
  void InitSortingOrders();
  virtual void DoSort() {}
  virtual void DoFilter() {}

  std::vector<uint32_t> m_Indices;
  std::vector<SortingOrder> m_SortingOrders;
  int m_SortingColumn = 0;
  std::string m_Filter;
  int m_UpdatePeriodMs;
  int m_SelectedIndex;
  DataViewType m_Type;

  static const std::string MENU_ACTION_COPY_SELECTION;
  static const std::string MENU_ACTION_EXPORT_TO_CSV;
};
