// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "CaptureWindow.h"
#include "OrbitBase/Tracing.h"

class OrbitApp;

class IntrospectionWindow : public CaptureWindow {
 public:
  explicit IntrospectionWindow(uint32_t font_size, OrbitApp* app);
  ~IntrospectionWindow() override;
  void ToggleRecording() override;

  [[nodiscard]] bool IsIntrospecting() const;
  void StartIntrospection();
  void StopIntrospection();

 protected:
  [[nodiscard]] const char* GetHelpText() const override;
  [[nodiscard]] bool ShouldAutoZoom() const override;

 private:
  std::unique_ptr<orbit_base::TracingListener> introspection_listener_;
};
