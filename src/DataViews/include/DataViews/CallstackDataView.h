// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_CALLSTACK_DATA_VIEW_H_
#define DATA_VIEWS_CALLSTACK_DATA_VIEW_H_

#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>
#include <stddef.h>
#include <stdint.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/CallstackInfo.h"
#include "ClientData/FunctionInfo.h"
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
  std::string GetValue(int row, int column) override;
  std::string GetToolTip(int row, int /*column*/) override;

  void OnDataChanged() override;
  void SetCallstack(const orbit_client_data::CallstackInfo& callstack) {
    callstack_ = callstack;
    OnDataChanged();
  }

  void ClearCallstack() {
    callstack_ = {};
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

  std::optional<orbit_client_data::CallstackInfo> callstack_;

  struct CallstackDataViewFrame {
    CallstackDataViewFrame(uint64_t address, const orbit_client_data::FunctionInfo* function,
                           const orbit_client_data::ModuleData* module)
        : address(address), function(function), module(module) {}
    CallstackDataViewFrame(uint64_t address, std::string fallback_name,
                           const orbit_client_data::ModuleData* module)
        : address(address), fallback_name(std::move(fallback_name)), module(module) {}

    uint64_t address = 0;
    const orbit_client_data::FunctionInfo* function = nullptr;
    std::string fallback_name;
    const orbit_client_data::ModuleData* module;
  };

  [[nodiscard]] CallstackDataViewFrame GetFrameFromRow(int row) const;
  [[nodiscard]] CallstackDataViewFrame GetFrameFromIndex(size_t index_in_callstack) const;

  enum ColumnIndex {
    kColumnSelected,
    kColumnName,
    kColumnSize,
    kColumnModule,
    kColumnAddress,
    kNumColumns
  };

  [[nodiscard]] ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                             absl::Span<const int> selected_indices) override;

 private:
  [[nodiscard]] const orbit_client_data::ModuleData* GetModuleDataFromRow(int row) const override {
    return GetFrameFromRow(row).module;
  }
  [[nodiscard]] const orbit_client_data::FunctionInfo* GetFunctionInfoFromRow(int row) override {
    return GetFrameFromRow(row).function;
  }

  absl::flat_hash_set<uint64_t> functions_to_highlight_;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_CALLSTACK_DATA_VIEW_H_
