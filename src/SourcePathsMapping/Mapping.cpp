// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SourcePathsMapping/Mapping.h"

#include <chrono>
#include <filesystem>
#include <optional>
#include <system_error>

#include "OrbitBase/Logging.h"

namespace fs = std::filesystem;

namespace orbit_source_paths_mapping {

static bool IsRegularFile(const std::filesystem::path& target_path) {
  std::error_code error{};
  if (fs::is_regular_file(target_path, error)) return true;
  if (error.value() != 0) {
    ORBIT_ERROR("Failed to 'stat' the file \"%s\": %s", target_path.string(), error.message());
  }
  return false;
}

std::optional<fs::path> MapToFirstExistingTarget(absl::Span<const Mapping> mappings,
                                                 const fs::path& source_path) {
  return MapToFirstMatchingTarget(mappings, source_path, &IsRegularFile);
}

std::optional<Mapping> InferMappingFromExample(const fs::path& source_path,
                                               const fs::path& target_path) {
  if (source_path.filename() != target_path.filename()) return std::nullopt;
  if (source_path == target_path) return std::nullopt;

  fs::path source = source_path;
  fs::path target = target_path;

  while (source.has_filename() && target.has_filename() && source.filename() == target.filename()) {
    source = source.parent_path();
    target = target.parent_path();
  }

  return Mapping{source, target};
}

}  // namespace orbit_source_paths_mapping