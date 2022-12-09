// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/FormatCallstackForTooltip.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include "ClientData/CallstackInfo.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/ShortenStringWithEllipsis.h"

namespace orbit_gl {

namespace {

struct UnformattedModuleAndFunctionName {
  // {module,function}_is_unknown doesn't imply that {module,function}_name is empty.
  // Rather, it indicates that the name might need to be formatted differently.
  std::string module_name{};
  bool module_is_unknown{};
  std::string function_name{};
  bool function_is_unknown{};
};

[[nodiscard]] UnformattedModuleAndFunctionName SafeGetModuleAndFunctionName(
    const orbit_client_data::CallstackInfo& callstack, size_t frame_index,
    const orbit_client_data::CaptureData& capture_data,
    const orbit_client_data::ModuleManager& module_manager) {
  if (frame_index >= callstack.frames().size()) {
    static const UnformattedModuleAndFunctionName frame_index_too_large_module_and_function_name =
        [] {
          UnformattedModuleAndFunctionName module_and_function_name;
          module_and_function_name.module_name = orbit_client_data::kUnknownFunctionOrModuleName;
          module_and_function_name.module_is_unknown = true;
          module_and_function_name.function_name = orbit_client_data::kUnknownFunctionOrModuleName;
          module_and_function_name.function_is_unknown = true;
          return module_and_function_name;
        }();
    return frame_index_too_large_module_and_function_name;
  }

  const uint64_t address = callstack.frames()[frame_index];

  const auto& [module_path, unused_module_build_id] =
      orbit_client_data::FindModulePathAndBuildIdByAddress(module_manager, capture_data, address);
  const std::string unknown_module_path_or_module_name =
      (module_path == orbit_client_data::kUnknownFunctionOrModuleName)
          ? module_path
          : std::filesystem::path(module_path).filename().string();

  const std::string& function_name =
      orbit_client_data::GetFunctionNameByAddress(module_manager, capture_data, address);
  const std::string function_name_or_unknown_with_address =
      (function_name == orbit_client_data::kUnknownFunctionOrModuleName)
          ? absl::StrFormat("[unknown@%#x]", address)
          : function_name;

  UnformattedModuleAndFunctionName module_and_function_name;
  module_and_function_name.module_name = unknown_module_path_or_module_name;
  module_and_function_name.module_is_unknown =
      (module_path == orbit_client_data::kUnknownFunctionOrModuleName);
  module_and_function_name.function_name = function_name_or_unknown_with_address;
  module_and_function_name.function_is_unknown =
      (function_name == orbit_client_data::kUnknownFunctionOrModuleName);
  return module_and_function_name;
}

[[nodiscard]] std::string FormatModuleName(
    const UnformattedModuleAndFunctionName& module_and_function_name) {
  return module_and_function_name.module_is_unknown
             ? absl::StrFormat("<i>%s</i>", module_and_function_name.module_name)
             : module_and_function_name.module_name;
}

[[nodiscard]] std::string FormatFunctionName(
    const UnformattedModuleAndFunctionName& module_and_function_name, size_t max_length) {
  const std::string& function_name = module_and_function_name.function_name;
  const std::string shortened_function_name =
      (max_length != std::numeric_limits<size_t>::max())
          ? ShortenStringWithEllipsis(function_name, max_length)
          : function_name;
  // Simple HTML escaping.
  const std::string escaped_function_name =
      absl::StrReplaceAll(shortened_function_name, {{"&", "&amp;"}, {"<", "&lt;"}, {">", "&gt;"}});
  return module_and_function_name.function_is_unknown
             ? absl::StrFormat("<i>%s</i>", escaped_function_name)
             : escaped_function_name;
}

}  // namespace

std::string FormatCallstackForTooltip(const orbit_client_data::CallstackInfo& callstack,
                                      const orbit_client_data::CaptureData& capture_data,
                                      const orbit_client_data::ModuleManager& module_manager,
                                      size_t max_line_length, size_t max_lines,
                                      size_t bottom_line_count) {
  // If the callstack has more than `max_lines` frames, we only show the `bottom_line_count`
  // outermost frames and the `max_lines - bottom_line_count` innermost frames, separated by
  // `kShortenedForReadabilityString`.
  ORBIT_CHECK(bottom_line_count < max_lines);

  size_t callstack_size = callstack.frames().size();
  const size_t bottom_n = std::min(bottom_line_count, callstack_size);
  const size_t top_n = std::min(max_lines, callstack_size) - bottom_n;

  constexpr size_t kShortenedForReadabilityFakeIndex = std::numeric_limits<size_t>::max();
  constexpr const char* kShortenedForReadabilityString = "<i>... shortened for readability ...</i>";
  std::vector<size_t> frame_indices_to_display;
  frame_indices_to_display.reserve(top_n);
  for (size_t i = 0; i < top_n; ++i) {
    frame_indices_to_display.push_back(i);
  }
  if (callstack_size > max_lines) {
    frame_indices_to_display.push_back(kShortenedForReadabilityFakeIndex);
  }
  for (size_t i = callstack_size - bottom_n; i < callstack_size; ++i) {
    frame_indices_to_display.push_back(i);
  }

  std::string result;
  for (size_t frame_index : frame_indices_to_display) {
    if (frame_index == kShortenedForReadabilityFakeIndex) {
      result.append(kShortenedForReadabilityString).append("<br/>");
      continue;
    }

    static const std::string module_function_separator{" | "};
    const UnformattedModuleAndFunctionName module_and_function_name =
        SafeGetModuleAndFunctionName(callstack, frame_index, capture_data, module_manager);
    const std::string formatted_module_name = FormatModuleName(module_and_function_name);
    const std::string formatted_function_name = FormatFunctionName(
        module_and_function_name, max_line_length - module_and_function_name.module_name.size() -
                                      module_function_separator.size());
    const std::string formatted_module_and_function_name =
        absl::StrCat(formatted_module_name, module_function_separator, formatted_function_name);
    // The first frame is always correct.
    if (callstack.IsUnwindingError() && frame_index > 0) {
      result += absl::StrFormat("<span style=\"color:%s;\">%s</span><br/>", kUnwindErrorColorString,
                                formatted_module_and_function_name);
    } else {
      result.append(formatted_module_and_function_name).append("<br/>");
    }
  }

  return result;
}

FormattedModuleAndFunctionName FormatInnermostFrameOfCallstackForTooltip(
    const orbit_client_data::CallstackInfo& callstack,
    const orbit_client_data::CaptureData& capture_data,
    const orbit_client_data::ModuleManager& module_manager) {
  UnformattedModuleAndFunctionName innermost_module_and_function_name =
      SafeGetModuleAndFunctionName(callstack, 0, capture_data, module_manager);
  std::string innermost_formatted_function_name =
      FormatFunctionName(innermost_module_and_function_name, std::numeric_limits<size_t>::max());
  std::string innermost_formatted_module_name =
      FormatModuleName(innermost_module_and_function_name);

  return {innermost_formatted_module_name, innermost_formatted_function_name};
}

}  // namespace orbit_gl