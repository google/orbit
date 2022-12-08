// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_LIVE_FUNCTIONS_DATA_VIEW_H_
#define DATA_VIEWS_LIVE_FUNCTIONS_DATA_VIEW_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/ScopeStatsCollection.h"
#include "DataViews/AppInterface.h"
#include "DataViews/CompareAscendingOrDescending.h"
#include "DataViews/DataView.h"
#include "DataViews/LiveFunctionsInterface.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Typedef.h"

namespace orbit_data_views {

class LiveFunctionsDataView : public DataView {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  explicit LiveFunctionsDataView(LiveFunctionsInterface* live_functions, AppInterface* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnCount; }
  std::string GetValue(int row, int column) override;
  // As we allow single selection on Live tab, this method returns either an empty vector or a
  // single-value vector.
  std::vector<int> GetVisibleSelectedIndices() override;
  void UpdateHighlightedFunctionId(absl::Span<const int> rows);
  void UpdateSelectedFunctionId();

  void OnSelect(absl::Span<const int> rows) override;
  void OnDataChanged() override;
  void OnTimer() override;
  void OnRefresh(absl::Span<const int> visible_selected_indices, const RefreshMode& mode) override;
  [[nodiscard]] bool ResetOnRefresh() const override { return false; }
  std::optional<int> GetRowFromScopeId(ScopeId scope_id);

  void OnIteratorRequested(absl::Span<const int> selection) override;
  void OnJumpToRequested(std::string_view action, absl::Span<const int> selection) override;
  // Export all events (including the function name, thread name and id, start timestamp, end
  // timestamp, and duration) associated with the selected rows in to a CSV file.
  void OnExportEventsToCsvRequested(absl::Span<const int> selection) override;

  void UpdateHistogramWithScopeIds(absl::Span<const ScopeId> scope_ids);

  void SetScopeStatsCollection(
      std::shared_ptr<const orbit_client_data::ScopeStatsCollectionInterface>
          scope_stats_collection);

  std::string GetToolTip(int /*row*/, int column) override;

  void AddScope(ScopeId scope_id) {
    ORBIT_CHECK(app_->HasCaptureData());
    const ScopeId max_scope_id = app_->GetCaptureData().GetMaxId();
    ORBIT_CHECK(scope_id <= max_scope_id);
    indices_.push_back(*scope_id);
  }

 protected:
  [[nodiscard]] ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                             absl::Span<const int> selected_indices) override;
  void DoFilter() override;
  void DoSort() override;
  [[nodiscard]] ScopeId GetScopeId(uint32_t row) const;
  [[nodiscard]] std::optional<orbit_client_data::FunctionInfo>
  CreateFunctionInfoFromInstrumentedFunction(
      const orbit_grpc_protos::InstrumentedFunction& instrumented_function);

  // Maps scope_ids corresponding to dynamically instrumented functions to FunctionInfo instances
  absl::flat_hash_map<ScopeId, orbit_client_data::FunctionInfo> scope_id_to_function_info_{};
  // TODO(b/191333567) This is populated in OnDataChanged(), which causes an overhead upon capture
  // load/finalization this may be optimized via populating it function-wise on user's demand

  LiveFunctionsInterface* live_functions_;
  std::optional<ScopeId> selected_scope_id_;

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

  [[nodiscard]] ErrorMessageOr<void> WriteEventsToCsv(absl::Span<const int> selection,
                                                      std::string_view file_path) const;

  void UpdateHistogramWithIndices(absl::Span<const int> visible_selected_indices);

  template <typename ValueGetterType>
  [[nodiscard]] std::function<bool(ScopeId, ScopeId)> MakeSorter(ValueGetterType getter,
                                                                 bool ascending) {
    return [getter, ascending](ScopeId id_a, ScopeId id_b) {
      return orbit_data_views_internal::CompareAscendingOrDescending(getter(id_a), getter(id_b),
                                                                     ascending);
    };
  }

  template <typename ValueGetterType, typename ValueType>
  [[nodiscard]] std::function<bool(ScopeId, ScopeId)> MakeFunctionSorter(ValueGetterType getter,
                                                                         bool ascending,
                                                                         ValueType default_value) {
    return MakeSorter(
        [this, getter, default_value](ScopeId id) {
          const auto* info = app_->GetCaptureData().GetFunctionInfoByScopeId(id);
          return info == nullptr ? default_value : getter(*info);
        },
        ascending);
  }

  template <typename ScopeIdSorter>
  [[nodiscard]] std::function<bool(uint64_t, uint64_t)> MakeIndexSorter(ScopeIdSorter sorter) {
    return [sorter](uint64_t index_a, uint64_t index_b) {
      return sorter(ScopeId(index_a), ScopeId(index_b));
    };
  }

  [[nodiscard]] std::vector<ScopeId> FetchMissingScopeIds() const;

  [[nodiscard]] const orbit_client_data::ScopeInfo& GetScopeInfo(ScopeId scope_id) const;

  std::shared_ptr<const orbit_client_data::ScopeStatsCollectionInterface> scope_stats_collection_ =
      std::make_shared<orbit_client_data::ScopeStatsCollection>();
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_LIVE_FUNCTIONS_DATA_VIEW_H_
