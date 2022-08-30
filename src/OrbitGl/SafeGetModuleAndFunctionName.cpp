// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SafeGetModuleAndFunctionName.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

#include <string>

#include "ClientData/CallstackInfo.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ShortenStringWithEllipsis.h"

namespace orbit_gl {

[[nodiscard]] std::string FormatCallstackForTooltip(
    const orbit_client_data::CallstackInfo& callstack,
    const orbit_client_data::CaptureData* capture_data,
    const orbit_client_data::ModuleManager* module_manager) {
  constexpr int kMaxLineLength = 120;
  // If the callstack has more than `kMaxLines` frames, we only show the `kBottomLineCount`
  // outermost frames and the `kMaxLines - kBottomLineCount` innermost frames, separated by
  // `kShortenedForReadabilityString`.
  constexpr int kMaxLines = 20;
  constexpr int kBottomLineCount = 5;
  static_assert(kBottomLineCount < kMaxLines);

  int callstack_size = static_cast<int>(callstack.frames().size());
  const int bottom_n = std::min(kBottomLineCount, callstack_size);
  const int top_n = std::min(kMaxLines, callstack_size) - bottom_n;

  constexpr int kShortenedForReadabilityFakeIndex = -1;
  constexpr const char* kShortenedForReadabilityString = "<i>... shortened for readability ...</i>";
  std::vector<int> frame_indices_to_display;
  for (int i = 0; i < top_n; ++i) {
    frame_indices_to_display.push_back(i);
  }
  if (callstack_size > kMaxLines) {
    frame_indices_to_display.push_back(kShortenedForReadabilityFakeIndex);
  }
  for (int i = callstack_size - bottom_n; i < callstack_size; ++i) {
    frame_indices_to_display.push_back(i);
  }

  std::string result;
  for (int frame_index : frame_indices_to_display) {
    if (frame_index == kShortenedForReadabilityFakeIndex) {
      result.append(kShortenedForReadabilityString).append("<br/>");
      continue;
    }

    static const std::string kModuleFunctionSeparator{" | "};
    const UnformattedModuleAndFunctionName module_and_function_name =
        SafeGetModuleAndFunctionName(callstack, frame_index, capture_data, module_manager);
    const std::string formatted_module_name = FormatModuleName(module_and_function_name);
    const std::string formatted_function_name = FormatFunctionName(
        module_and_function_name, kMaxLineLength - module_and_function_name.module_name.size() -
                                      kModuleFunctionSeparator.size());
    const std::string formatted_module_and_function_name =
        formatted_module_name + kModuleFunctionSeparator + formatted_function_name;
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

[[nodiscard]] UnformattedModuleAndFunctionName SafeGetModuleAndFunctionName(
    const orbit_client_data::CallstackInfo& callstack, size_t frame_index,
    const orbit_client_data::CaptureData* capture_data,
    const orbit_client_data::ModuleManager* module_manager) {
  ORBIT_CHECK(capture_data != nullptr);
  if (frame_index >= callstack.frames().size()) {
    static const UnformattedModuleAndFunctionName kFrameIndexTooLargeModuleAndFunctionName = [] {
      UnformattedModuleAndFunctionName module_and_function_name;
      module_and_function_name.module_name = orbit_client_data::kUnknownFunctionOrModuleName;
      module_and_function_name.module_is_unknown = true;
      module_and_function_name.function_name = orbit_client_data::kUnknownFunctionOrModuleName;
      module_and_function_name.function_is_unknown = true;
      return module_and_function_name;
    }();
    return kFrameIndexTooLargeModuleAndFunctionName;
  }

  const uint64_t address = callstack.frames()[frame_index];

  const auto& [module_path, unused_module_build_id] =
      orbit_client_data::FindModulePathAndBuildIdByAddress(*module_manager, *capture_data, address);
  const std::string unknown_module_path_or_module_name =
      (module_path == orbit_client_data::kUnknownFunctionOrModuleName)
          ? module_path
          : std::filesystem::path(module_path).filename().string();

  const std::string& function_name =
      orbit_client_data::GetFunctionNameByAddress(*module_manager, *capture_data, address);
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
    const UnformattedModuleAndFunctionName& module_and_function_name, int max_length) {
  const std::string& function_name = module_and_function_name.function_name;
  const std::string shortened_function_name =
      (max_length > 0) ? ShortenStringWithEllipsis(function_name, static_cast<size_t>(max_length))
                       : function_name;
  // Simple HTML escaping.
  const std::string escaped_function_name =
      absl::StrReplaceAll(shortened_function_name, {{"&", "&amp;"}, {"<", "&lt;"}, {">", "&gt;"}});
  return module_and_function_name.function_is_unknown
             ? absl::StrFormat("<i>%s</i>", escaped_function_name)
             : escaped_function_name;
}

}  // namespace orbit_gl