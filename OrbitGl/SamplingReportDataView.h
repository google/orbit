// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "CallStackDataView.h"
#include "DataView.h"
#include "OrbitClientModel/SamplingDataPostProcessor.h"
#include "absl/container/flat_hash_set.h"

class OrbitApp;

class SamplingReportDataView : public DataView {
 public:
  explicit SamplingReportDataView(OrbitApp* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnInclusive; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  const std::string& GetName() { return name_; }

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnSelect(std::optional<int> index) override;

  void LinkDataView(DataView* data_view) override;
  void SetSamplingReport(class SamplingReport* sampling_report) {
    sampling_report_ = sampling_report;
  }
  void SetSampledFunctions(const std::vector<SampledFunction>& functions);
  void SetThreadID(ThreadID tid);
  ThreadID GetThreadID() const { return tid_; }

 protected:
  void DoSort() override;
  void DoFilter() override;
  const SampledFunction& GetSampledFunction(unsigned int row) const;
  SampledFunction& GetSampledFunction(unsigned int row);
  absl::flat_hash_set<const orbit_client_protos::FunctionInfo*> GetFunctionsFromIndices(
      const std::vector<int>& indices);
  [[nodiscard]] absl::flat_hash_set<std::string> GetModulePathsFromIndices(
      const std::vector<int>& indices) const;

 private:
  std::vector<SampledFunction> functions_;
  ThreadID tid_ = -1;
  std::string name_;
  CallStackDataView* callstack_data_view_;
  SamplingReport* sampling_report_ = nullptr;

  enum ColumnIndex {
    kColumnSelected,
    kColumnFunctionName,
    kColumnExclusive,
    kColumnInclusive,
    kColumnModuleName,
    kColumnFile,
    kColumnLine,
    kColumnAddress,
    kNumColumns
  };

  static const std::string kMenuActionSelect;
  static const std::string kMenuActionUnselect;
  static const std::string kMenuActionLoadSymbols;
  static const std::string kMenuActionDisassembly;

  OrbitApp* app_ = nullptr;
};
