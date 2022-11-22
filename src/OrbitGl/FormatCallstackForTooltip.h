// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FORMAT_CALLSTACK_FOR_TOOLTIP_H_
#define ORBIT_GL_FORMAT_CALLSTACK_FOR_TOOLTIP_H_

#include <stddef.h>

#include <string>

#include "ClientData/CallstackInfo.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"

namespace orbit_gl {

constexpr const char* kUnwindErrorColorString = "#ffb000";

// Module and function name formatted to be displayed in a tooltip. Formatting contains simple HTML-
// escaping, name shortening and italic writing of unknown names.
struct FormattedModuleAndFunctionName {
  std::string module_name;
  std::string function_name;
};

[[nodiscard]] std::string FormatCallstackForTooltip(
    const orbit_client_data::CallstackInfo& callstack,
    const orbit_client_data::CaptureData& capture_data,
    const orbit_client_data::ModuleManager& module_manager, size_t max_line_length = 120,
    size_t max_lines = 20, size_t bottom_line_count = 5);

[[nodiscard]] FormattedModuleAndFunctionName FormatInnermostFrameOfCallstackForTooltip(
    const orbit_client_data::CallstackInfo& callstack,
    const orbit_client_data::CaptureData& capture_data,
    const orbit_client_data::ModuleManager& module_manager);

}  // namespace orbit_gl

#endif  // ORBIT_GL_FORMAT_CALLSTACK_FOR_TOOLTIP_H_