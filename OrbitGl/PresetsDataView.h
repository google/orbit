// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRESET_DATA_VIEW_H_
#define ORBIT_GL_PRESET_DATA_VIEW_H_

#include <memory>

#include "DataView.h"
#include "preset.pb.h"

class PresetsDataView : public DataView {
 public:
  PresetsDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_SESSION_NAME; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;
  std::string GetToolTip(int a_Row, int a_Column) override;
  std::string GetLabel() override { return "Presets"; }

  void OnDataChanged() override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;

  void SetPresets(
      const std::vector<std::shared_ptr<orbit_client_protos::PresetFile>>&
          presets);

 protected:
  void DoSort() override;
  void DoFilter() override;
  const std::shared_ptr<orbit_client_protos::PresetFile>& GetPreset(
      unsigned int a_Row) const;

  std::vector<std::shared_ptr<orbit_client_protos::PresetFile>> presets_;

  enum ColumnIndex { COLUMN_SESSION_NAME, COLUMN_PROCESS_NAME, COLUMN_NUM };

  static const std::string MENU_ACTION_LOAD;
  static const std::string MENU_ACTION_DELETE;
};

#endif  // ORBIT_GL_PRESET_DATA_VIEW_H_
