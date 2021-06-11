// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRESET_DATA_VIEW_H_
#define ORBIT_GL_PRESET_DATA_VIEW_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "DataView.h"
#include "MetricsUploader/MetricsUploader.h"
#include "PresetFile/PresetFile.h"
#include "preset.pb.h"

class OrbitApp;

class PresetsDataView : public DataView {
 public:
  explicit PresetsDataView(OrbitApp* app,
                           orbit_metrics_uploader::MetricsUploader* metrics_uploader);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnPresetName; }
  std::vector<std::string> GetContextMenu(int clicked_index,
                                          const std::vector<int>& selected_indices) override;
  std::string GetValue(int row, int column) override;
  std::string GetToolTip(int row, int column) override;
  std::string GetLabel() override { return "Presets"; }

  void OnDataChanged() override;
  void OnContextMenu(const std::string& action, int menu_index,
                     const std::vector<int>& item_indices) override;
  void OnDoubleClicked(int index) override;

  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int /*row*/, int /*column*/, unsigned char& /*red*/,
                       unsigned char& /*green*/, unsigned char& /*blue*/) override;

  void SetPresets(std::vector<orbit_preset_file::PresetFile> presets);

 protected:
  struct ModuleView {
    ModuleView(std::string name, uint32_t count)
        : module_name(std::move(name)), function_count(count){};
    std::string module_name;
    uint32_t function_count;
  };

  void DoSort() override;
  void DoFilter() override;
  [[nodiscard]] static std::string GetModulesList(const std::vector<ModuleView>& modules);
  [[nodiscard]] static std::string GetFunctionCountList(const std::vector<ModuleView>& modules);
  [[nodiscard]] const orbit_preset_file::PresetFile& GetPreset(unsigned int row) const;
  [[nodiscard]] const std::vector<ModuleView>& GetModules(uint32_t row) const;

  std::vector<orbit_preset_file::PresetFile> presets_;
  std::vector<std::vector<ModuleView>> modules_;

  enum ColumnIndex {
    kColumnLoadState,
    kColumnPresetName,
    kColumnModules,
    kColumnFunctionCount,
    kNumColumns
  };

  static const std::string kMenuActionLoad;
  static const std::string kMenuActionDelete;

 private:
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;

  // TODO(b/185090791): This is temporary and will be removed once this data view has been ported
  // and move to orbit_data_views.
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_PRESET_DATA_VIEW_H_
