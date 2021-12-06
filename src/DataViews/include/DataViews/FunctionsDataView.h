// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_FUNCTIONS_DATA_VIEW_H_
#define DATA_VIEWS_FUNCTIONS_DATA_VIEW_H_

#include <string>
#include <vector>

#include "ClientProtos/capture_data.pb.h"
#include "DataViews/AppInterface.h"
#include "DataViews/DataView.h"
#include "OrbitBase/ThreadPool.h"

namespace orbit_data_views {
class FunctionsDataView : public DataView {
 public:
  explicit FunctionsDataView(AppInterface* app, orbit_base::ThreadPool* thread_pool);

  static const std::string kUnselectedFunctionString;
  static const std::string kSelectedFunctionString;
  static const std::string kFrameTrackString;
  static std::string BuildSelectedColumnsString(AppInterface* app,
                                                const orbit_client_protos::FunctionInfo& function);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnAddressInModule; }
  std::vector<std::vector<std::string>> GetContextMenuWithGrouping(
      int clicked_index, const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  std::string GetLabel() override { return "Functions"; }

  void AddFunctions(std::vector<const orbit_client_protos::FunctionInfo*> functions);
  void ClearFunctions();

 protected:
  void DoSort() override;
  void DoFilter() override;

  std::vector<std::string> filter_tokens_;

  enum ColumnIndex {
    kColumnSelected,
    kColumnName,
    kColumnSize,
    kColumnModule,
    kColumnAddressInModule,
    kNumColumns
  };

 private:
  [[nodiscard]] const orbit_client_protos::FunctionInfo* GetFunctionInfoFromRow(int row) override {
    return functions_[indices_[row]];
  }

  static bool ShouldShowSelectedFunctionIcon(AppInterface* app,
                                             const orbit_client_protos::FunctionInfo& function);
  static bool ShouldShowFrameTrackIcon(AppInterface* app,
                                       const orbit_client_protos::FunctionInfo& function);
  std::vector<const orbit_client_protos::FunctionInfo*> functions_;

  orbit_base::ThreadPool* thread_pool_;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_FUNCTIONS_DATA_VIEW_H_
