// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "CaptureWindow.h"
#include "OrbitBase/Tracing.h"

class IntrospectionWindow : public CaptureWindow {
 public:
  explicit IntrospectionWindow(uint32_t font_size);
  ~IntrospectionWindow() override;
  void ToggleCapture() override;

  [[nodiscard]] bool IsIntrospecting();
  void StartIntrospection();
  void StopIntrospection();

 protected:
  [[nodiscard]] const char* GetHelpText() override;
  [[nodiscard]] bool ShouldAutoZoom() override;

 private:
  std::unique_ptr<orbit::tracing::Listener> introspection_listener_;
};
