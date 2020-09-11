// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MODULES_DATA_VIEW_H_
#define ORBIT_GL_MODULES_DATA_VIEW_H_

#include "DataView.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ProcessData.h"

class ModulesDataView : public DataView {
 public:
  ModulesDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnFileSize; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int row, int column, unsigned char& red, unsigned char& green,
                       unsigned char& blue) override;
  std::string GetLabel() override { return "Modules"; }
  bool HasRefreshButton() const override { return true; }
  void OnRefreshButtonClicked() override;

  void SetProcess(const ProcessData* process);

 protected:
  void DoSort() override;
  void DoFilter() override;

 private:
  [[nodiscard]] const ModuleData* GetModule(uint32_t row) const { return modules_[indices_[row]]; }

  std::vector<ModuleData*> modules_;
  absl::flat_hash_map<const ModuleData*, const MemorySpace*> module_memory_;

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
};

#endif  // ORBIT_GL_MODULES_DATA_VIEW_H_
