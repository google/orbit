// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LiveFunctionsDataView.h"

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "FunctionStats.h"
#include "FunctionUtils.h"
#include "LiveFunctions.h"
#include "Log.h"
#include "OrbitFunction.h"
#include "Pdb.h"
#include "TextBox.h"
#include "TimeGraph.h"
#include "TimerChain.h"

//-----------------------------------------------------------------------------
LiveFunctionsDataView::LiveFunctionsDataView(LiveFunctions* live_functions)
    : DataView(DataViewType::LIVE_FUNCTIONS), live_functions_(live_functions) {
  m_UpdatePeriodMs = 300;
  OnDataChanged();
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& LiveFunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SELECTED] = {"Hooked", .0f, SortingOrder::Descending};
    columns[COLUMN_NAME] = {"Function", .4f, SortingOrder::Ascending};
    columns[COLUMN_COUNT] = {"Count", .0f, SortingOrder::Descending};
    columns[COLUMN_TIME_TOTAL] = {"Total", .075f, SortingOrder::Descending};
    columns[COLUMN_TIME_AVG] = {"Avg", .075f, SortingOrder::Descending};
    columns[COLUMN_TIME_MIN] = {"Min", .075f, SortingOrder::Descending};
    columns[COLUMN_TIME_MAX] = {"Max", .075f, SortingOrder::Descending};
    columns[COLUMN_MODULE] = {"Module", .1f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string LiveFunctionsDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  Function& function = GetFunction(a_Row);
  const std::shared_ptr<FunctionStats> stats = function.stats();

  switch (a_Column) {
    case COLUMN_SELECTED:
      return FunctionUtils::IsSelected(function) ? "X" : "-";
    case COLUMN_NAME:
      return FunctionUtils::GetDisplayName(function);
    case COLUMN_COUNT:
      return absl::StrFormat("%lu", stats->m_Count);
    case COLUMN_TIME_TOTAL:
      return GetPrettyTime(stats->m_TotalTimeMs);
    case COLUMN_TIME_AVG:
      return GetPrettyTime(stats->m_AverageTimeMs);
    case COLUMN_TIME_MIN:
      return GetPrettyTime(stats->m_MinMs);
    case COLUMN_TIME_MAX:
      return GetPrettyTime(stats->m_MaxMs);
    case COLUMN_MODULE:
      return function.loaded_module_path();
    case COLUMN_ADDRESS:
      return absl::StrFormat("0x%llx", FunctionUtils::GetAbsoluteAddress(function));
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
#define ORBIT_FUNC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, \
                               ascending);                                 \
  }
#define ORBIT_STAT_SORT(Member)                                             \
  [&](int a, int b) {                                                       \
    return OrbitUtils::Compare(functions[a]->stats()->Member,             \
                               functions[b]->stats()->Member, ascending); \
  }
#define ORBIT_CUSTOM_FUNC_SORT(Func)                                     \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(Func(*functions[a]), Func(*functions[b]), \
                               ascending);                               \
  }

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  const std::vector<Function*>& functions = m_Functions;

  switch (m_SortingColumn) {
    case COLUMN_SELECTED:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::IsSelected);
      break;
    case COLUMN_NAME:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::GetDisplayName);
      break;
    case COLUMN_COUNT:
      sorter = ORBIT_STAT_SORT(m_Count);
      break;
    case COLUMN_TIME_TOTAL:
      sorter = ORBIT_STAT_SORT(m_TotalTimeMs);
      break;
    case COLUMN_TIME_AVG:
      sorter = ORBIT_STAT_SORT(m_AverageTimeMs);
      break;
    case COLUMN_TIME_MIN:
      sorter = ORBIT_STAT_SORT(m_MinMs);
      break;
    case COLUMN_TIME_MAX:
      sorter = ORBIT_STAT_SORT(m_MaxMs);
      break;
    case COLUMN_MODULE:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::GetLoadedModuleName);
      break;
    case COLUMN_ADDRESS:
      sorter = ORBIT_FUNC_SORT(address());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
const std::string LiveFunctionsDataView::MENU_ACTION_SELECT = "Hook";
const std::string LiveFunctionsDataView::MENU_ACTION_UNSELECT = "Unhook";
const std::string LiveFunctionsDataView::MENU_ACTION_JUMP_TO_FIRST =
    "Jump to first";
const std::string LiveFunctionsDataView::MENU_ACTION_JUMP_TO_LAST =
    "Jump to last";
const std::string LiveFunctionsDataView::MENU_ACTION_JUMP_TO_MIN =
    "Jump to min";
const std::string LiveFunctionsDataView::MENU_ACTION_JUMP_TO_MAX =
    "Jump to max";
const std::string LiveFunctionsDataView::MENU_ACTION_DISASSEMBLY =
    "Go to Disassembly";
const std::string LiveFunctionsDataView::MENU_ACTION_ITERATE =
    "Add iterator(s)";

//-----------------------------------------------------------------------------
std::vector<std::string> LiveFunctionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_disassembly = !a_SelectedIndices.empty();
  for (int index : a_SelectedIndices) {
    const Function& function = GetFunction(index);
    enable_select |= !FunctionUtils::IsSelected(function);
    enable_unselect |= FunctionUtils::IsSelected(function);
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  if (enable_disassembly) menu.emplace_back(MENU_ACTION_DISASSEMBLY);
  menu.emplace_back(MENU_ACTION_ITERATE);

  // For now, these actions only make sense when one function is selected,
  // so we don't show them otherwise.
  if (a_SelectedIndices.size() == 1) {
    menu.insert(menu.end(), {MENU_ACTION_JUMP_TO_FIRST,
                             MENU_ACTION_JUMP_TO_LAST, MENU_ACTION_JUMP_TO_MIN,
                             MENU_ACTION_JUMP_TO_MAX});
  }
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnContextMenu(
    const std::string& a_Action, int a_MenuIndex,
    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SELECT) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      FunctionUtils::Select(&function);
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      FunctionUtils::UnSelect(&function);
    }
  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    int32_t pid = Capture::GTargetProcess->GetID();
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      GOrbitApp->Disassemble(pid, function);
    }
  } else if (a_Action == MENU_ACTION_JUMP_TO_FIRST) {
    CHECK(a_ItemIndices.size() == 1);
    auto function_address =
        FunctionUtils::GetAbsoluteAddress(GetFunction(a_ItemIndices[0]));
    auto first_box = GCurrentTimeGraph->FindNextFunctionCall(
        function_address, std::numeric_limits<TickType>::lowest());
    if (first_box) {
      GCurrentTimeGraph->SelectAndZoom(first_box);
    }
  } else if (a_Action == MENU_ACTION_JUMP_TO_LAST) {
    CHECK(a_ItemIndices.size() == 1);
    auto function_address =
        FunctionUtils::GetAbsoluteAddress(GetFunction(a_ItemIndices[0]));
    auto last_box = GCurrentTimeGraph->FindPreviousFunctionCall(
        function_address, std::numeric_limits<TickType>::max());
    if (last_box) {
      GCurrentTimeGraph->SelectAndZoom(last_box);
    }
  } else if (a_Action == MENU_ACTION_JUMP_TO_MIN) {
    CHECK(a_ItemIndices.size() == 1);
    Function& function = GetFunction(a_ItemIndices[0]);
    auto [min_box, _] = GetMinMax(function);
    if (min_box) {
      GCurrentTimeGraph->SelectAndZoom(min_box);
    }
  } else if (a_Action == MENU_ACTION_JUMP_TO_MAX) {
    CHECK(a_ItemIndices.size() == 1);
    Function& function = GetFunction(a_ItemIndices[0]);
    auto [_, max_box] = GetMinMax(function);
    if (max_box) {
      GCurrentTimeGraph->SelectAndZoom(max_box);
    }
  } else if (a_Action == MENU_ACTION_ITERATE) {
    for (int i : a_ItemIndices) {
      Function& function = GetFunction(i);
      if (function.stats()->m_Count > 0) {
        live_functions_->AddIterator(&function);
      } else {
        // TODO: Display error message to user?
      }
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::DoFilter() {
  GOrbitApp->FilterFunctions(m_Filter);
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(ToLower(m_Filter), ' ');

  for (size_t i = 0; i < m_Functions.size(); ++i) {
    const Function* function = m_Functions[i];
    if (function != nullptr) {
      std::string name = ToLower(FunctionUtils::GetDisplayName(*function));

      bool match = true;

      for (std::string& filterToken : tokens) {
        if (name.find(filterToken) == std::string::npos) {
          match = false;
          break;
        }
      }

      if (match) {
        indices.push_back(i);
      }
    }
  }

  m_Indices = indices;

  OnSort(m_SortingColumn, {});

  // Filter drawn textboxes
  Capture::GVisibleFunctionsMap.clear();
  for (size_t i = 0; i < m_Indices.size(); ++i) {
    Function& func = GetFunction(i);
    Capture::GVisibleFunctionsMap[FunctionUtils::GetAbsoluteAddress(func)] = &func;
  }

  GOrbitApp->NeedsRedraw();
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnDataChanged() {
  size_t numFunctions = Capture::GFunctionCountMap.size();
  m_Indices.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }

  m_Functions.clear();
  for (auto& pair : Capture::GFunctionCountMap) {
    const ULONG64& address = pair.first;
    Function* func = Capture::GSelectedFunctionsMap[address];
    m_Functions.push_back(func);
  }

  DataView::OnDataChanged();
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnTimer() {
  if (Capture::IsCapturing()) {
    OnSort(m_SortingColumn, {});
  }
}

//-----------------------------------------------------------------------------
Function& LiveFunctionsDataView::GetFunction(unsigned int a_Row) const {
  CHECK(a_Row < m_Functions.size());
  CHECK(m_Functions[m_Indices[a_Row]]);
  return *m_Functions[m_Indices[a_Row]];
}

std::pair<TextBox*, TextBox*> LiveFunctionsDataView::GetMinMax(
    Function& function) const {
  auto function_address = FunctionUtils::GetAbsoluteAddress(function);
  TextBox *min_box = nullptr, *max_box = nullptr;
  std::vector<std::shared_ptr<TimerChain>> chains =
      GCurrentTimeGraph->GetAllThreadTrackTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
      TimerBlock& block = *it;
      for (size_t i = 0; i < block.size(); i++) {
        TextBox& box = block[i];
        if (box.GetTimer().m_FunctionAddress == function_address) {
          if (!min_box || box.GetTimer().ElapsedMicros() <
                              min_box->GetTimer().ElapsedMicros())
            min_box = &box;
          if (!max_box || box.GetTimer().ElapsedMicros() >
                              max_box->GetTimer().ElapsedMicros())
            max_box = &box;
        }
      }
    }
  }
  return std::make_pair(min_box, max_box);
}