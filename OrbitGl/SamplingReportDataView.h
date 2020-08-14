// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "CallStackDataView.h"
#include "DataView.h"
#include "SamplingProfiler.h"

class SamplingReportDataView : public DataView {
 public:
  SamplingReportDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnInclusive; }
  std::vector<std::string> GetContextMenu(
      int clicked_index, const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  const std::string& GetName() { return name_; }

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnSelect(int index) override;

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
  std::vector<orbit_client_protos::FunctionInfo*> GetFunctionsFromIndices(
      const std::vector<int>& indices);
  std::vector<std::shared_ptr<Module>> GetModulesFromIndices(
      const std::vector<int>& indices);

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
};
