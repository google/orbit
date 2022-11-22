// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SourcePathsMapping/MappingManager.h"

#include <stddef.h>

#include <QSettings>
#include <QString>
#include <QVariant>
#include <algorithm>

#include "OrbitBase/Logging.h"

constexpr const char* kMappingSettingsKey = "source_path_mappings";
constexpr const char* kSourcePathKey = "source_path";
constexpr const char* kTargetPathKey = "target_path";

namespace orbit_source_paths_mapping {

MappingManager::MappingManager() { LoadMappings(); }

void MappingManager::LoadMappings() {
  mappings_.clear();

  QSettings settings{};
  const int size = settings.beginReadArray(kMappingSettingsKey);
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    Mapping new_mapping{};
    new_mapping.source_path = settings.value(kSourcePathKey).toString().toStdString();
    new_mapping.target_path = settings.value(kTargetPathKey).toString().toStdString();
    if (new_mapping.source_path.empty() || new_mapping.target_path.empty()) continue;
    mappings_.emplace_back(std::move(new_mapping));
  }
  settings.endArray();
}

void MappingManager::SaveMappings() {
  QSettings settings{};
  settings.beginWriteArray(kMappingSettingsKey, static_cast<int>(mappings_.size()));
  for (size_t i = 0; i < mappings_.size(); ++i) {
    settings.setArrayIndex(static_cast<int>(i));
    Mapping& current_mapping = mappings_[i];
    settings.setValue(kSourcePathKey, QString::fromStdString(current_mapping.source_path.string()));
    settings.setValue(kTargetPathKey, QString::fromStdString(current_mapping.target_path.string()));
  }
  settings.endArray();
}

void MappingManager::SetMappings(std::vector<Mapping> mappings) {
  mappings_ = std::move(mappings);
  SaveMappings();
}

void MappingManager::AppendMapping(Mapping mapping) {
  mappings_.emplace_back(std::move(mapping));
  SaveMappings();
}

void InferAndAppendSourcePathsMapping(const std::filesystem::path& source_path,
                                      const std::filesystem::path& target_path) {
  std::optional<orbit_source_paths_mapping::Mapping> maybe_mapping =
      orbit_source_paths_mapping::InferMappingFromExample(source_path, target_path);
  if (!maybe_mapping.has_value()) {
    ORBIT_ERROR("Unable to infer a mapping from \"%s\" to \"%s\"", source_path.string(),
                target_path.string());
    return;
  }

  orbit_source_paths_mapping::MappingManager mapping_manager{};
  mapping_manager.AppendMapping(maybe_mapping.value());
}

}  // namespace orbit_source_paths_mapping