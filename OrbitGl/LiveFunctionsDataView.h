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
  int GetDefaultSortingColumn() override { return kColumnCount; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnSelect(int row) override;
  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnDataChanged() override;
  void OnTimer() override;
  int GetRowFromFunctionAddress(uint64_t function_address);

 protected:
  void DoFilter() override;
  void DoSort() override;
  [[nodiscard]] orbit_client_protos::FunctionInfo* GetSelectedFunction(unsigned int row);
  [[nodiscard]] std::pair<TextBox*, TextBox*> GetMinMax(
      const orbit_client_protos::FunctionInfo& function) const;

  std::vector<orbit_client_protos::FunctionInfo> functions_;

  LiveFunctionsController* live_functions_;

  enum ColumnIndex {
    kColumnSelected,
    kColumnName,
    kColumnCount,
    kColumnTimeTotal,
    kColumnTimeAvg,
    kColumnTimeMin,
    kColumnTimeMax,
    kColumnModule,
    kColumnAddress,
    kNumColumns
  };

  static const std::string kMenuActionSelect;
  static const std::string kMenuActionUnselect;
  static const std::string kMenuActionJumpToFirst;
  static const std::string kMenuActionJumpToLast;
  static const std::string kMenuActionJumpToMin;
  static const std::string kMenuActionJumpToMax;
  static const std::string kMenuActionDisassembly;
  static const std::string kMenuActionIterate;
  static const std::string kMenuActionEnableFrameTrack;
  static const std::string kMenuActionDisableFrameTrack;
};

#endif  // ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_
