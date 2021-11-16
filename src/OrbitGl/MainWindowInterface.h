// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MAIN_WINDOW_INTERFACE_H_
#define ORBIT_GL_MAIN_WINDOW_INTERFACE_H_

#include <stdint.h>

#include <filesystem>
#include <optional>
#include <string_view>

#include "CodeReport/CodeReport.h"
#include "CodeReport/DisassemblyReport.h"
#include "capture_data.pb.h"

namespace orbit_gl {

// This abstract base class is an attempt to simplify callbacks
// in OrbitApp and make it easier to refactor things in the future.
//
// OrbitMainWindow and Mocks can derive from this and offer a fixed interface
// to OrbitApp.
class MainWindowInterface {
 public:
  virtual void ShowTooltip(std::string_view message) = 0;
  virtual void ShowWarningWithDontShowAgainCheckboxIfNeeded(
      std::string_view title, std::string_view text,
      std::string_view dont_show_again_setting_key) = 0;

  virtual void ShowSourceCode(
      const std::filesystem::path& file_path, size_t line_number,
      std::optional<std::unique_ptr<orbit_code_report::CodeReport>> code_report) = 0;
  virtual void ShowDisassembly(const orbit_client_protos::FunctionInfo& function_info,
                               const std::string& assembly,
                               orbit_code_report::DisassemblyReport report) = 0;

  enum class CaptureLogSeverity { kInfo, kWarning, kSevereWarning, kError };
  virtual void AppendToCaptureLog(CaptureLogSeverity severity, absl::Duration capture_time,
                                  std::string_view message) = 0;

  virtual ~MainWindowInterface() = default;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MAIN_WINDOW_INTERFACE_H_