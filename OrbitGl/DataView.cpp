//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "DataView.h"

#include <fstream>

#include "App.h"
#include "CallStackDataView.h"
#include "FunctionsDataView.h"
#include "GlobalsDataView.h"
#include "LiveFunctionsDataView.h"
#include "LogDataView.h"
#include "ModulesDataView.h"
#include "OrbitType.h"
#include "ProcessesDataView.h"
#include "SamplingReportDataView.h"
#include "SessionsDataView.h"
#include "TypesDataView.h"

//-----------------------------------------------------------------------------
void DataView::InitSortingOrders() {
  m_SortingOrders.clear();
  for (const auto& column : GetColumns()) {
    m_SortingOrders.push_back(column.initial_order);
  }

  m_SortingColumn = GetDefaultSortingColumn();
}

//-----------------------------------------------------------------------------
void DataView::OnSort(int column, std::optional<SortingOrder> new_order) {
  if (column < 0) {
    return;
  }

  if (!IsSortingAllowed()) {
    return;
  }

  if (m_SortingOrders.empty()) {
    InitSortingOrders();
  }

  m_SortingColumn = column;
  if (new_order.has_value()) {
    m_SortingOrders[column] = new_order.value();
  }

  DoSort();
}

//-----------------------------------------------------------------------------
void DataView::OnFilter(const std::string& filter) {
  m_Filter = filter;
  DoFilter();
}

//-----------------------------------------------------------------------------
void DataView::OnDataChanged() {
  OnSort(m_SortingColumn, std::optional<SortingOrder>{});
  OnFilter(m_Filter);
}

//-----------------------------------------------------------------------------
const std::string DataView::MENU_ACTION_COPY_SELECTION = "Copy Selection";
const std::string DataView::MENU_ACTION_EXPORT_TO_CSV = "Export to CSV";

//-----------------------------------------------------------------------------
std::vector<std::string> DataView::GetContextMenu(
    int /*a_ClickedIndex*/, const std::vector<int>& /*a_SelectedIndices*/) {
  static std::vector<std::string> menu = {MENU_ACTION_COPY_SELECTION,
                                          MENU_ACTION_EXPORT_TO_CSV};
  return menu;
}

//-----------------------------------------------------------------------------
void DataView::OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                             const std::vector<int>& a_ItemIndices) {
  UNUSED(a_MenuIndex);

  if (a_Action == MENU_ACTION_EXPORT_TO_CSV) {
    std::string save_file = GOrbitApp->GetSaveFile(".csv");
    if (!save_file.empty()) {
      ExportCSV(save_file);
    }
  } else if (a_Action == MENU_ACTION_COPY_SELECTION) {
    CopySelection(a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void DataView::ExportCSV(const std::string& a_FileName) {
  std::ofstream out(a_FileName);
  if (out.fail()) return;

  size_t numColumns = GetColumns().size();
  for (size_t i = 0; i < numColumns; ++i) {
    out << GetColumns()[i].header;
    if (i < numColumns - 1) out << ", ";
  }
  out << "\n";

  size_t numElements = GetNumElements();
  for (size_t i = 0; i < numElements; ++i) {
    for (size_t j = 0; j < numColumns; ++j) {
      out << GetValue(i, j);
      if (j < numColumns - 1) out << ", ";
    }
    out << "\n";
  }

  out.close();
}

//-----------------------------------------------------------------------------
void DataView::CopySelection(const std::vector<int>& selection) {
  std::string clipboard;
  size_t numColumns = GetColumns().size();
  for (size_t i = 0; i < numColumns; ++i) {
    clipboard += GetColumns()[i].header;
    if (i < numColumns - 1) clipboard += ", ";
  }
  clipboard += "\n";

  size_t numElements = GetNumElements();
  for (size_t i : selection) {
    if (i < numElements) {
      for (size_t j = 0; j < numColumns; ++j) {
        clipboard += GetValue(i, j);
        if (j < numColumns - 1) clipboard += ", ";
      }
      clipboard += "\n";
    }
  }

  GOrbitApp->SetClipboard(s2ws(clipboard));
}
