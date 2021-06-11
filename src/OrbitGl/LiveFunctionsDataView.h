// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_
#define ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "DataViews/DataView.h"
#include "MetricsUploader/MetricsUploader.h"
#include "capture.pb.h"
#include "capture_data.pb.h"

class LiveFunctionsController;
class OrbitApp;

class LiveFunctionsDataView : public orbit_data_views::DataView {
 public:
  explicit LiveFunctionsDataView(LiveFunctionsController* live_functions, OrbitApp* app,
                                 orbit_metrics_uploader::MetricsUploader* metrics_uploader);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnCount; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  // As we allow single selection on Live tab, this method returns either an empty vector or a
  // single-value vector.
  std::vector<int> GetVisibleSelectedIndices() override;
  void UpdateHighlightedFunctionId(const std::vector<int>& rows);
  void UpdateSelectedFunctionId();

  void OnSelect(const std::vector<int>& rows) override;
  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnDataChanged() override;
  void OnTimer() override;
  void OnRefresh(const std::vector<int>& visible_selected_indices,
                 const RefreshMode& mode) override;
  [[nodiscard]] bool ResetOnRefresh() const override { return false; }
  std::optional<int> GetRowFromFunctionId(uint64_t function_id);

 protected:
  void DoFilter() override;
  void DoSort() override;
  [[nodiscard]] uint64_t GetInstrumentedFunctionId(uint32_t row) const;
  [[nodiscard]] const orbit_client_protos::FunctionInfo& GetInstrumentedFunction(
      uint32_t row) const;
  [[nodiscard]] std::optional<orbit_client_protos::FunctionInfo>
  CreateFunctionInfoFromInstrumentedFunction(
      const orbit_grpc_protos::InstrumentedFunction& instrumented_function);

  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> functions_{};

  LiveFunctionsController* live_functions_;
  uint64_t selected_function_id_;

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
  static const std::string kMenuActionSourceCode;
  static const std::string kMenuActionIterate;
  static const std::string kMenuActionEnableFrameTrack;
  static const std::string kMenuActionDisableFrameTrack;

 private:
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;

  // TODO(b/185090791): This is temporary and will be removed once this data view has been ported
  // and move to orbit_data_views.
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_LIVE_FUNCTIONS_DATA_VIEW_H_
