// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FUNCTIONS_DATA_VIEW_H_
#define ORBIT_GL_FUNCTIONS_DATA_VIEW_H_

#include "DataView.h"
#include "capture_data.pb.h"

class FunctionsDataView : public DataView {
 public:
  FunctionsDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnAddress; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnDataChanged() override;

 protected:
  void DoSort() override;
  void DoFilter() override;
  void ParallelFilter();
  orbit_client_protos::FunctionInfo& GetFunction(int row) const;

  std::vector<std::string> m_FilterTokens;

  enum ColumnIndex {
    kColumnSelected,
    kColumnName,
    kColumnSize,
    kColumnFile,
    kColumnLine,
    kColumnModule,
    kColumnAddress,
    kNumColumns
  };

  static const std::string kMenuActionSelect;
  static const std::string kMenuActionUnselect;
  static const std::string kMenuActionDisassembly;
};

#endif  // ORBIT_GL_FUNCTIONS_DATA_VIEW_H_
