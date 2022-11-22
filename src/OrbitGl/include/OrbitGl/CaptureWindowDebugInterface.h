// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_WINDOW_DEBUG_INTERACE_H_
#define ORBIT_GL_CAPTURE_WINDOW_DEBUG_INTERACE_H_

#include <string>

namespace orbit_gl {
// Exposes some debug information from a CaptureWindow. All pieces are targeting developers of
// Orbit and are not meant for the user of the tool.
class CaptureWindowDebugInterface {
 public:
  [[nodiscard]] virtual std::string GetCaptureInfo() const = 0;
  [[nodiscard]] virtual std::string GetPerformanceInfo() const = 0;
  [[nodiscard]] virtual std::string GetSelectionSummary() const = 0;

  virtual ~CaptureWindowDebugInterface() = default;
};
}  // namespace orbit_gl

#endif  // ORBIT_GL_CAPTURE_WINDOW_DEBUG_INTERFACE_H_
