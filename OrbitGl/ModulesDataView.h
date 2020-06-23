// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MODULES_DATA_VIEW_H_
#define ORBIT_GL_MODULES_DATA_VIEW_H_

#include "DataView.h"
#include "ModuleData.h"
#include "OrbitType.h"
#include "ProcessUtils.h"

class ModulesDataView : public DataView {
 public:
  ModulesDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_PDB_SIZE; }
  std::vector<std::string> GetContextMenu(
      int clicked_index, const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;

  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int row, int column, unsigned char& red,
                       unsigned char& green, unsigned char& blue) override;
  std::string GetLabel() override { return "Modules"; }

  void SetModules(uint32_t process_id, const std::vector<ModuleData*>& modules);

 protected:
  void DoSort() override;
  void DoFilter() override;

 private:
  const ModuleData* GetModule(uint32_t row) const;

  uint32_t process_id_;
  std::vector<ModuleData*> modules_;

  enum ColumnIndex {
    COLUMN_INDEX,
    COLUMN_NAME,
    COLUMN_PATH,
    COLUMN_ADDRESS_RANGE,
    COLUMN_HAS_PDB,
    COLUMN_PDB_SIZE,
    COLUMN_LOADED,
    COLUMN_NUM
  };

  static const std::string MENU_ACTION_MODULES_LOAD;
  static const std::string MENU_ACTION_MODULES_VERIFY;
};

#endif  // ORBIT_GL_MODULES_DATA_VIEW_H_
