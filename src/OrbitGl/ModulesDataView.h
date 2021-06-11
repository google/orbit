// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MODULES_DATA_VIEW_H_
#define ORBIT_GL_MODULES_DATA_VIEW_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "DataView.h"

class OrbitApp;

class ModulesDataView : public DataView {
 public:
  explicit ModulesDataView(OrbitApp* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnFileSize; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnDoubleClicked(int index) override;
  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int row, int column, unsigned char& red, unsigned char& green,
                       unsigned char& blue) override;
  std::string GetLabel() override { return "Modules"; }
  [[nodiscard]] bool HasRefreshButton() const override { return true; }
  void OnRefreshButtonClicked() override;
  void UpdateModules(const orbit_client_data::ProcessData* process);

 protected:
  void DoSort() override;
  void DoFilter() override;

 private:
  [[nodiscard]] orbit_client_data::ModuleData* GetModule(uint32_t row) const {
    return start_address_to_module_.at(indices_[row]);
  }

  absl::flat_hash_map<uint64_t, orbit_client_data::ModuleInMemory>
      start_address_to_module_in_memory_;
  absl::flat_hash_map<uint64_t, orbit_client_data::ModuleData*> start_address_to_module_;

  enum ColumnIndex {
    kColumnName,
    kColumnPath,
    kColumnAddressRange,
    kColumnFileSize,
    kColumnLoaded,
    kNumColumns
  };

  static const std::string kMenuActionLoadSymbols;
  static const std::string kMenuActionVerifyFramePointers;

  // TODO(b/185090791): This is temporary and will be removed once this data view has been ported
  // and move to orbit_data_views.
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_MODULES_DATA_VIEW_H_
