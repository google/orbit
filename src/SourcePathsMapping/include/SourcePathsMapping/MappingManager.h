// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_PATHS_MAPPING_MAPPING_MANAGER_H_
#define SOURCE_PATHS_MAPPING_MAPPING_MANAGER_H_

#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

#include "SourcePathsMapping/Mapping.h"

namespace orbit_source_paths_mapping {

// Maintains a list of source paths mappings and takes care of persistence using QSettings.
class MappingManager {
 public:
  explicit MappingManager();
  [[nodiscard]] const std::vector<Mapping>& GetMappings() const { return mappings_; }

  void SetMappings(std::vector<Mapping> mappings);
  void AppendMapping(Mapping mapping);

  template <typename Predicate>
  [[nodiscard]] std::optional<std::filesystem::path> MapToFirstMatchingTarget(
      const std::filesystem::path& source_path, Predicate&& predicate) const {
    return orbit_source_paths_mapping::MapToFirstMatchingTarget(mappings_, source_path,
                                                                std::forward<Predicate>(predicate));
  }

  [[nodiscard]] std::optional<std::filesystem::path> MapToFirstExistingTarget(
      const std::filesystem::path& source_path) const {
    return orbit_source_paths_mapping::MapToFirstExistingTarget(mappings_, source_path);
  }

 private:
  void SaveMappings();
  void LoadMappings();

  std::vector<Mapping> mappings_;
};

// This is a convenience function. It will try to infer a mapping and if that succeeds it will
// append the mapping to the current list of mappings using a local instance of MappingManager.
void InferAndAppendSourcePathsMapping(const std::filesystem::path& source_path,
                                      const std::filesystem::path& target_path);

}  // namespace orbit_source_paths_mapping

#endif  // SOURCE_PATHS_MAPPING_MAPPING_MANAGER_H_