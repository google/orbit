// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ClientData/CallstackType.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientModel/SamplingDataPostProcessor.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/AppInterface.h"
#include "DataViews/CallstackDataView.h"
#include "DataViews/DataView.h"
#include "DataViews/SamplingReportInterface.h"
#include "OrbitBase/Result.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "absl/container/flat_hash_set.h"

class SamplingReport;

namespace orbit_data_views {

class SamplingReportDataView : public DataView {
 public:
  explicit SamplingReportDataView(AppInterface* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnInclusive; }
  std::string GetValue(int row, int column) override;
  std::string GetValueForCopy(int row, int column) override;
  std::string GetToolTip(int /*row*/, int /*column*/) override;
  const std::string& GetName() { return name_; }

  void OnSelect(absl::Span<const int> indices) override;
  void OnRefresh(absl::Span<const int> visible_selected_indices, const RefreshMode& mode) override;

  void LinkDataView(DataView* data_view) override;
  void SetSamplingReport(SamplingReportInterface* sampling_report) {
    sampling_report_ = sampling_report;
  }
  void SetSampledFunctions(absl::Span<const orbit_client_data::SampledFunction> functions);
  void SetThreadID(orbit_client_data::ThreadID tid);
  void SetStackEventsCount(uint32_t stack_events_count);
  [[nodiscard]] orbit_client_data::ThreadID GetThreadID() const { return tid_; }

  void OnExportEventsToCsvRequested(absl::Span<const int> selection) override;

 protected:
  [[nodiscard]] ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                             absl::Span<const int> selected_indices) override;
  void DoSort() override;
  void DoFilter() override;
  [[nodiscard]] const orbit_client_data::SampledFunction& GetSampledFunction(
      unsigned int row) const;
  orbit_client_data::SampledFunction& GetSampledFunction(unsigned int row);
  [[nodiscard]] std::optional<orbit_symbol_provider::ModuleIdentifier> GetModuleIdentifierFromRow(
      int row) const;

 private:
  [[nodiscard]] orbit_client_data::ModuleData* GetModuleDataFromRow(int row) const override;
  [[nodiscard]] const orbit_client_data::FunctionInfo* GetFunctionInfoFromRow(int row) override;

  void UpdateSelectedIndicesAndFunctionIds(absl::Span<const int> selected_indices);
  void RestoreSelectedIndicesAfterFunctionsChanged();
  // The callstack view will be updated according to the visible selected addresses and thread id.
  void UpdateVisibleSelectedAddressesAndTid(absl::Span<const int> visible_selected_indices);

  [[nodiscard]] std::string BuildPercentageString(float percentage) const;

  [[nodiscard]] std::string BuildToolTipInclusive(
      const orbit_client_data::SampledFunction& function) const;
  [[nodiscard]] std::string BuildToolTipExclusive(
      const orbit_client_data::SampledFunction& function) const;
  [[nodiscard]] std::string BuildToolTipUnwindErrors(
      const orbit_client_data::SampledFunction& function) const;

  ErrorMessageOr<void> WriteStackEventsToCsv(std::string_view file_path);

  std::vector<orbit_client_data::SampledFunction> functions_;
  // We need to keep user's selected function ids such that if functions_ changes, the
  // selected_indices_ can be updated according to the selected function ids.
  absl::flat_hash_set<uint64_t> selected_function_ids_;
  orbit_client_data::ThreadID tid_ = -1;
  uint32_t stack_events_count_ = 0;
  std::string name_;
  SamplingReportInterface* sampling_report_ = nullptr;

  enum ColumnIndex {
    kColumnSelected,
    kColumnFunctionName,
    kColumnInclusive,  // Default sorting column.
    kColumnExclusive,
    kColumnModuleName,
    kColumnAddress,
    kColumnUnwindErrors,
    kNumColumns
  };
};

}  // namespace orbit_data_views