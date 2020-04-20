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
DataView::~DataView() {
  if (GOrbitApp) {
    GOrbitApp->Unregister(this);
  }
}

//-----------------------------------------------------------------------------
DataView* DataView::Create(DataViewType a_Type) {
  DataView* model = nullptr;
  switch (a_Type) {
    case DataViewType::FUNCTIONS:
      model = new FunctionsDataView();
      break;
    case DataViewType::TYPES:
      model = new TypesDataView();
      break;
    case DataViewType::LIVEFUNCTIONS:
      model = new LiveFunctionsDataView();
      break;
    case DataViewType::CALLSTACK:
      model = new CallStackDataView();
      break;
    case DataViewType::GLOBALS:
      model = new GlobalsDataView();
      break;
    case DataViewType::MODULES:
      model = new ModulesDataView();
      break;
    case DataViewType::SAMPLING:
      model = new SamplingReportDataView();
      break;
    case DataViewType::PROCESSES:
      model = new ProcessesDataView();
      break;
    case DataViewType::SESSIONS:
      model = new SessionsDataView();
      break;
    case DataViewType::LOG:
      model = new LogDataView();
      break;
    default:
      break;
  }

  model->m_Type = a_Type;
  return model;
}

//-----------------------------------------------------------------------------
void DataView::InitSortingOrders() {
  m_SortingOrders.clear();
  for (const auto& column : GetColumns()) {
    m_SortingOrders.push_back(column.initial_order);
  }
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
    ExportCSV(ws2s(GOrbitApp->GetSaveFile(L".csv")));
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
      out << GetValue((int)i, (int)j);
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
        clipboard += GetValue((int)i, (int)j);
        if (j < numColumns - 1) clipboard += ", ";
      }
      clipboard += "\n";
    }
  }

  GOrbitApp->SetClipboard(s2ws(clipboard));
}
