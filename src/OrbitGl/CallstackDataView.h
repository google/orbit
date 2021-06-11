// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CALLSTACK_DATA_VIEW_H_
#define ORBIT_GL_CALLSTACK_DATA_VIEW_H_

#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "DataView.h"
#include "capture_data.pb.h"

class CallstackDataView : public DataView {
 public:
  explicit CallstackDataView(OrbitApp* app);

  void SetAsMainInstance() override;
  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnAddress; }
  bool IsSortingAllowed() override { return false; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  std::string GetToolTip(int row, int /*column*/) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
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

  CallstackDataViewFrame GetFrameFromRow(int row);
  CallstackDataViewFrame GetFrameFromIndex(int index_in_callstack);

  enum ColumnIndex {
    kColumnSelected,
    kColumnName,
    kColumnSize,
    kColumnModule,
    kColumnAddress,
    kNumColumns
  };

  static const std::string kMenuActionLoadSymbols;
  static const std::string kMenuActionSelect;
  static const std::string kMenuActionUnselect;
  static const std::string kMenuActionDisassembly;
  static const std::string kHighlightedFunctionString;
  static const std::string kHighlightedFunctionBlankString;
  static const std::string kMenuActionSourceCode;

 private:
  absl::flat_hash_set<uint64_t> functions_to_highlight_;

  // TODO(b/185090791): This is temporary and will be removed once this data view has been ported
  // and move to orbit_data_views.
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_CALLSTACK_DATA_VIEW_H_
