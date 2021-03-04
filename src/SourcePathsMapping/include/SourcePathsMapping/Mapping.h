// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_PATHS_MAPPING_MAPPING_H_
#define SOURCE_PATHS_MAPPING_MAPPING_H_

#include <absl/strings/match.h>
#include <absl/types/span.h>

#include <filesystem>
#include <optional>
#include <tuple>

namespace orbit_source_paths_mapping {

// Mapping is a strong type for a pair of file paths. It says the `source_path` can be found locally
// at `target_path`. That means all absolute paths beginning with `source_path` can be mapped to
// `target_path`.
//
// This is important for mapping debug information. When Libraries or executable have been compiled
// on a different machine, the included source file paths need to be mapped.
struct Mapping {
  std::filesystem::path source_path;
  std::filesystem::path target_path;

  friend bool operator==(const Mapping& lhs, const Mapping& rhs) {
    return std::tie(lhs.source_path, lhs.target_path) == std::tie(rhs.source_path, rhs.target_path);
  }

  friend bool operator!=(const Mapping& lhs, const Mapping& rhs) { return !(lhs == rhs); }

  [[nodiscard]] bool IsValid() const { return !source_path.empty() && !target_path.empty(); }
};

template <typename Predicate>
[[nodiscard]] std::optional<std::filesystem::path> MapToFirstMatchingTarget(
    absl::Span<const Mapping> mappings, const std::filesystem::path& source_path,
    Predicate&& predicate) {
  for (const auto& mapping : mappings) {
    if (absl::StartsWith(source_path.string(), mapping.source_path.string())) {
      std::string target = mapping.target_path.string();
      target.append(
          std::string_view{source_path.string()}.substr(mapping.source_path.string().size()));

      std::filesystem::path target_path{target};
      if (predicate(target_path)) return target_path;
    }
  }

  return std::nullopt;
}
[[nodiscard]] std::optional<std::filesystem::path> MapToFirstExistingTarget(
    absl::Span<const Mapping> mappings, const std::filesystem::path& source_path);

// Tries to create a mapping by finding the longest common suffix between source and target. The
// remaining prefixes will construct the mapping.
[[nodiscard]] std::optional<Mapping> InferMappingFromExample(
    const std::filesystem::path& source_path, const std::filesystem::path& target_path);

}  // namespace orbit_source_paths_mapping

#endif  // SOURCE_PATHS_MAPPING_MAPPING_H_