// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_CALLSTACK_DATA_VIEW_H_
#define DATA_VIEWS_CALLSTACK_DATA_VIEW_H_

#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/AppInterface.h"
#include "DataViews/DataView.h"

namespace orbit_data_views {

class CallstackDataView : public DataView {
 public:
  explicit CallstackDataView(AppInterface* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnAddress; }
  bool IsSortingAllowed() override { return false; }
  std::vector<std::vector<std::string>> GetContextMenuWithGrouping(
      int clicked_index, const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  std::string GetToolTip(int row, int /*column*/) override;

  void OnDataChanged() override;
  void SetCallstack(const orbit_client_protos::CallstackInfo& callstack) {
    callstack_ = callstack;
    OnDataChanged();
  }

  void ClearCallstack() {
    callstack_ = orbit_client_protos::CallstackInfo{};
    OnDataChanged();
  }

  void SetFunctionsToHighlight(const absl::flat_hash_set<uint64_t>& absolute_addresses);
  [[nodiscard]] bool WantsDisplayColor() override { return true; }
  [[nodiscard]] bool GetDisplayColor(int row, int /*column*/, unsigned char& red,
                                     unsigned char& green, unsigned char& blue) override;

  static const std::string kHighlightedFunctionString;
  static const std::string kHighlightedFunctionBlankString;

 protected:
  void DoFilter() override;

  orbit_client_protos::CallstackInfo callstack_;

  struct CallstackDataViewFrame {
    CallstackDataViewFrame(uint64_t address, const orbit_client_protos::FunctionInfo* function,
                           orbit_client_data::ModuleData* module)
        : address(address), function(function), module(module) {}
    CallstackDataViewFrame(uint64_t address, std::string fallback_name,
                           orbit_client_data::ModuleData* module)
        : address(address), fallback_name(std::move(fallback_name)), module(module) {}

    uint64_t address = 0;
    const orbit_client_protos::FunctionInfo* function = nullptr;
    std::string fallback_name;
    orbit_client_data::ModuleData* module;
  };

  CallstackDataViewFrame GetFrameFromRow(int row) const;
  CallstackDataViewFrame GetFrameFromIndex(int index_in_callstack) const;

  enum ColumnIndex {
    kColumnSelected,
    kColumnName,
    kColumnSize,
    kColumnModule,
    kColumnAddress,
    kNumColumns
  };

 private:
  [[nodiscard]] orbit_client_data::ModuleData* GetModuleDataFromRow(int row) const override {
    return GetFrameFromRow(row).module;
  }
  [[nodiscard]] const orbit_client_protos::FunctionInfo* GetFunctionInfoFromRow(int row) override {
    return GetFrameFromRow(row).function;
  }

  absl::flat_hash_set<uint64_t> functions_to_highlight_;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_CALLSTACK_DATA_VIEW_H_
