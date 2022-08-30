// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "ClientData/CallstackInfo.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"

namespace orbit_gl {

constexpr const char* kUnwindErrorColorString = "#ffb000";

struct UnformattedModuleAndFunctionName {
  // {module,function}_is_unknown doesn't imply that {module,function}_name is empty.
  // Rather, it indicates that the name might need to be formatted differently.
  std::string module_name;
  bool module_is_unknown;
  std::string function_name;
  bool function_is_unknown;
};

[[nodiscard]] UnformattedModuleAndFunctionName SafeGetModuleAndFunctionName(
    const orbit_client_data::CallstackInfo& callstack, size_t frame_index,
    const orbit_client_data::CaptureData* capture_data,
    const orbit_client_data::ModuleManager* module_manager);

[[nodiscard]] std::string FormatCallstackForTooltip(
    const orbit_client_data::CallstackInfo& callstack,
    const orbit_client_data::CaptureData* capture_data,
    const orbit_client_data::ModuleManager* module_manager);

[[nodiscard]] std::string FormatModuleName(
    const UnformattedModuleAndFunctionName& module_and_function_name);

[[nodiscard]] std::string FormatFunctionName(
    const UnformattedModuleAndFunctionName& module_and_function_name, int max_length);

}  // namespace orbit_gl