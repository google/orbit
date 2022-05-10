// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_LIVE_FUNCTIONS_DATA_VIEW_H_
#define DATA_VIEWS_LIVE_FUNCTIONS_DATA_VIEW_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "DataViews/AppInterface.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataView.h"
#include "DataViews/LiveFunctionsInterface.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_data_views {

class LiveFunctionsDataView : public DataView {
 public:
  explicit LiveFunctionsDataView(LiveFunctionsInterface* live_functions, AppInterface* app,
                                 orbit_metrics_uploader::MetricsUploader* metrics_uploader);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnCount; }
  std::string GetValue(int row, int column) override;
  // As we allow single selection on Live tab, this method returns either an empty vector or a
  // single-value vector.
  std::vector<int> GetVisibleSelectedIndices() override;
  void UpdateHighlightedFunctionId(const std::vector<int>& rows);
  void UpdateSelectedFunctionId();

  void OnSelect(const std::vector<int>& rows) override;
  void OnDataChanged() override;
  void OnTimer() override;
  void OnRefresh(const std::vector<int>& visible_selected_indices,
                 const RefreshMode& mode) override;
  [[nodiscard]] bool ResetOnRefresh() const override { return false; }
  std::optional<int> GetRowFromScopeId(uint64_t scope_id);
  void AddFunction(uint64_t scope_id, orbit_client_data::FunctionInfo function_info);

  void OnIteratorRequested(const std::vector<int>& selection) override;
  void OnJumpToRequested(const std::string& action, const std::vector<int>& selection) override;
  // Export all events (including the function name, thread name and id, start timestamp, end
  // timestamp, and duration) associated with the selected rows in to a CSV file.
  void OnExportEventsToCsvRequested(const std::vector<int>& selection) override;

  void UpdateHistogramWithScopeIds(const std::vector<uint64_t>& scope_ids);

  std::string GetToolTip(int /*row*/, int column) override;

 protected:
  [[nodiscard]] ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                             const std::vector<int>& selected_indices) override;
  void DoFilter() override;
  void DoSort() override;
  [[nodiscard]] std::optional<orbit_client_data::FunctionInfo>
  CreateFunctionInfoFromInstrumentedFunction(
      const orbit_grpc_protos::InstrumentedFunction& instrumented_function);

  // Maps scope_ids corresponding to dynamically instrumented functions to FunctionInfo instances
  absl::flat_hash_map<uint64_t, orbit_client_data::FunctionInfo> scope_id_to_function_info_{};
  // TODO(b/191333567) This is populated in OnDataChanged(), which causes an overhead upon capture
  // load/finalization this may be optimized via populating it function-wise on user's demand

  LiveFunctionsInterface* live_functions_;
  uint64_t selected_scope_id_;

  enum ColumnIndex {
    kColumnType,
    kColumnName,
    kColumnCount,
    kColumnTimeTotal,
    kColumnTimeAvg,
    kColumnTimeMin,
    kColumnTimeMax,
    kColumnStdDev,
    kColumnModule,
    kColumnAddress,
    kNumColumns
  };

 private:
  // The row does not necessarily refer to a dynamically instrumented function. If it does, a
  // pointer to the corresponding FunctionInfo is returned. `nullptr` is returned otherwise.
  [[nodiscard]] const orbit_client_data::FunctionInfo* GetFunctionInfoFromRow(int row) override;

  [[nodiscard]] ErrorMessageOr<void> WriteEventsToCsv(const std::vector<int>& selection,
                                                      const std::string& file_path) const;

  void UpdateHistogramWithIndices(const std::vector<int>& visible_selected_indices);

  template <typename ValueGetterType>
  [[nodiscard]] std::function<bool(uint64_t, uint64_t)> MakeSorter(ValueGetterType getter,
                                                                   bool ascending) {
    return [getter, ascending](uint64_t id_a, uint64_t id_b) {
      return orbit_data_views_internal::CompareAscendingOrDescending(getter(id_a), getter(id_b),
                                                                     ascending);
    };
  }

  template <typename ValueGetterType, typename ValueType>
  [[nodiscard]] std::function<bool(uint64_t, uint64_t)> MakeFunctionSorter(
      ValueGetterType getter, bool ascending, ValueType default_value) {
    return MakeSorter(
        [this, getter, default_value](uint64_t id) {
          const auto it = scope_id_to_function_info_.find(id);
          return it == scope_id_to_function_info_.end() ? default_value : getter(it->second);
        },
        ascending);
  }

  [[nodiscard]] const orbit_client_data::ScopeInfo& GetScopeInfo(uint64_t scope_id) const;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_LIVE_FUNCTIONS_DATA_VIEW_H_
