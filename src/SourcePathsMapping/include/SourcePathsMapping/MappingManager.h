// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_PATHS_MAPPING_MAPPING_MANAGER_H_
#define SOURCE_PATHS_MAPPING_MAPPING_MANAGER_H_

#include <optional>
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

 private:
  void SaveMappings();
  void LoadMappings();

  std::vector<Mapping> mappings_;
};

}  // namespace orbit_source_paths_mapping

#endif  // SOURCE_PATHS_MAPPING_MAPPING_MANAGER_H_