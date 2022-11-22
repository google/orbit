// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_SOURCE_PATHS_MAPPING_DIALOG_H_
#define CONFIG_WIDGETS_SOURCE_PATHS_MAPPING_DIALOG_H_

#include <QDialog>
#include <QItemSelection>
#include <QObject>
#include <QString>
#include <QWidget>
#include <memory>
#include <utility>
#include <vector>

#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingItemModel.h"

namespace Ui {
class SourcePathsMappingDialog;  // IWYU pragma: keep
}

namespace orbit_config_widgets {

class SourcePathsMappingDialog : public QDialog {
  Q_OBJECT
 public:
  explicit SourcePathsMappingDialog(QWidget* parent = nullptr);
  ~SourcePathsMappingDialog() override;

  void SetMappings(std::vector<orbit_source_paths_mapping::Mapping> new_mappings) {
    model_.SetMappings(std::move(new_mappings));
  }
  [[nodiscard]] const std::vector<orbit_source_paths_mapping::Mapping>& GetMappings() const {
    return model_.GetMappings();
  }

  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnSourcePathChanged(const QString& new_source);
  void OnTargetPathChanged(const QString& new_target);
  void OnRemoveSelectedMapping();

 private:
  std::unique_ptr<Ui::SourcePathsMappingDialog> ui_;
  orbit_source_paths_mapping::MappingItemModel model_;
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_SOURCE_PATHS_MAPPING_DIALOG_H_