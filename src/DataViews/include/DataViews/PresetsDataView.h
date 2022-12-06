// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_PRESETS_DATA_VIEW_H_
#define DATA_VIEWS_PRESETS_DATA_VIEW_H_

#include <absl/types/span.h>
#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientProtos/preset.pb.h"
#include "DataViews/AppInterface.h"
#include "DataViews/DataView.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "PresetFile/PresetFile.h"

namespace orbit_data_views {
class PresetsDataView : public DataView {
 public:
  explicit PresetsDataView(AppInterface* app);

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return kColumnPresetName; }
  std::string GetValue(int row, int column) override;
  std::string GetToolTip(int row, int column) override;
  std::string GetLabel() override { return "Presets"; }

  void OnDataChanged() override;
  void OnDoubleClicked(int index) override;

  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int /*row*/, int /*column*/, unsigned char& /*red*/,
                       unsigned char& /*green*/, unsigned char& /*blue*/) override;

  void SetPresets(std::vector<orbit_preset_file::PresetFile> presets);

  void OnLoadPresetRequested(absl::Span<const int> selection) override;
  void OnDeletePresetRequested(absl::Span<const int> selection) override;
  void OnShowInExplorerRequested(absl::Span<const int> selection) override;
  void OnLoadPresetSuccessful(const std::filesystem::path& preset_file_path);

  static constexpr std::string_view kLoadedPresetPrefix{"* "};
  static constexpr std::string_view kNotLoadedPresetPrefix{"  "};
  static constexpr std::string_view kLoadedPresetTooltipSuffix{" (loaded)"};
  static constexpr std::string_view kNotLoadedPresetTooltipSuffix{""};

 protected:
  struct ModuleView {
    ModuleView(std::string name, uint64_t count)
        : module_name(std::move(name)), function_count(count){};
    std::string module_name;
    size_t function_count;
  };

  [[nodiscard]] ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                             absl::Span<const int> selected_indices) override;
  void DoSort() override;
  void DoFilter() override;
  [[nodiscard]] static std::string GetModulesList(absl::Span<const ModuleView> modules);
  [[nodiscard]] static std::string GetFunctionCountList(absl::Span<const ModuleView> modules);
  [[nodiscard]] static std::string GetModuleAndFunctionCountList(
      absl::Span<const ModuleView> modules);
  [[nodiscard]] const orbit_preset_file::PresetFile& GetPreset(unsigned int row) const;
  [[nodiscard]] const std::vector<ModuleView>& GetModules(uint32_t row) const;

  std::vector<orbit_preset_file::PresetFile> presets_;
  std::vector<std::vector<ModuleView>> modules_;

  enum ColumnIndex {
    kColumnLoadState,
    kColumnPresetName,
    kColumnModules,
    kColumnFunctionCount,
    kColumnDateModified,
    kNumColumns
  };

 private:
  [[nodiscard]] orbit_preset_file::PresetFile& GetMutablePreset(unsigned int row);

  std::shared_ptr<orbit_base::MainThreadExecutor> main_thread_executor_;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_PRESETS_DATA_VIEW_H_
