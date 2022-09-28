// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_API_SHIM_NAMESPACE_DISPATCHER_H_
#define WINDOWS_API_SHIM_NAMESPACE_DISPATCHER_H_

#include <absl/strings/str_replace.h>
#include <absl/strings/str_split.h>

#include <functional>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "WindowsApiShimUtils.h"
#include "win32/base.h"
#include "win32/manifest.h"

namespace orbit_windows_api_shim {

class WindowsApiHelper {
 public:
  static const WindowsApiHelper& Get() {
    static WindowsApiHelper api_helper;
    return api_helper;
  }

  [[nodiscard]] std::optional<std::string> GetNamespaceFromFunctionKey(
      const std::string& function_key) const {
    auto it = function_key_to_namespace_map_.find(function_key);
    if (it != function_key_to_namespace_map_.end()) return it->second;
    ORBIT_SHIM_ERROR("Could not find namespace associated with function key: %s", function_key);
    return std::nullopt;
  }

  [[nodiscsard]] std::optional<std::vector<std::string>> GetFunctionKeysFromNamespace(
      const std::string& name_space) const {
    auto it = namespace_to_functions_keys_map_.find(name_space);
    if (it != namespace_to_functions_keys_map_.end()) return it->second;
    ORBIT_SHIM_ERROR("Could not find function keys associated with namespace: %s", name_space);
    return std::nullopt;
  }

  [[nodiscard]] static std::optional<std::string> GetModuleFromFunctionKey(
      const std::string_view function_key) {
    std::vector<std::string> tokens = absl::StrSplit(function_key, "__");
    if (tokens.empty()) return std::nullopt;
    return tokens[0];
  }

  [[nodiscard]] static std::optional<std::string> GetFunctionFromFunctionKey(
      const std::string_view function_key) {
    std::vector<std::string> tokens = absl::StrSplit(function_key, "__");
    if (tokens.size() < 2) return std::nullopt;
    return tokens[1];
  }

  [[nodiscard]] const std::unordered_map<std::string, std::string>& GetFunctionKeyToNamespaceMap() {
    return function_key_to_namespace_map_;
  }

  [[nodiscard]] const std::unordered_map<std::string, std::vector<std::string>>&
  GetNamespaceToFunctionKeysMap() {
    return namespace_to_functions_keys_map_;
  }

 private:
  WindowsApiHelper() {
    for (int i = 0; i < kWindowsApiFunctions.size(); ++i) {
      const auto& api_function = kWindowsApiFunctions[i];
      if (api_function.function_key == nullptr || api_function.name_space == nullptr) {
        std::cout << "Found null function data" << std::endl;
        error_indices_.push_back(i);
      }
    }
    for (const WindowsApiFunction& api_function : kWindowsApiFunctions) {
      if (api_function.function_key == nullptr || api_function.name_space == nullptr) continue;
      function_key_to_namespace_map_.emplace(api_function.function_key, api_function.name_space);
      namespace_to_functions_keys_map_[api_function.name_space].push_back(
          api_function.function_key);
    }
  }

  std::unordered_map<std::string, std::string> function_key_to_namespace_map_;
  std::unordered_map<std::string, std::vector<std::string>> namespace_to_functions_keys_map_;
  std::vector<size_t> error_indices_;
};

bool FindTrampolineInfo(const char* function_key, TrampolineInfo& out_function_info);

}  // namespace orbit_windows_api_shim

#endif
