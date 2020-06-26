// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <memory>

#include "DataView.h"

class Preset;

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

  void SetPresets(const std::vector<std::shared_ptr<Preset>>& presets);

 protected:
  void DoSort() override;
  void DoFilter() override;
  const std::shared_ptr<Preset>& GetPreset(unsigned int a_Row) const;

  std::vector<std::shared_ptr<Preset>> presets_;

  enum ColumnIndex { COLUMN_SESSION_NAME, COLUMN_PROCESS_NAME, COLUMN_NUM };

  static const std::string MENU_ACTION_LOAD;
  static const std::string MENU_ACTION_DELETE;
};
