// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_
#define ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_

#include "DataView.h"
#include "TimerChain.h"
#include "capture_data.pb.h"

class LiveFunctionsController;

class LiveFunctionsDataView : public DataView {
 public:
  LiveFunctionsDataView(LiveFunctionsController* live_functions);

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
  orbit_client_protos::FunctionInfo* GetFunction(unsigned int a_Row) const;
  std::pair<TextBox*, TextBox*> GetMinMax(
      const orbit_client_protos::FunctionInfo& function) const;

  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>> functions_;

  LiveFunctionsController* live_functions_;

  enum ColumnIndex {
    COLUMN_SELECTED,
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
  static const std::string MENU_ACTION_JUMP_TO_FIRST;
  static const std::string MENU_ACTION_JUMP_TO_LAST;
  static const std::string MENU_ACTION_JUMP_TO_MIN;
  static const std::string MENU_ACTION_JUMP_TO_MAX;
  static const std::string MENU_ACTION_DISASSEMBLY;
  static const std::string MENU_ACTION_ITERATE;
};

#endif  // ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_
