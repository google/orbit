// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_INTROSPECTION_WINDOW_H_
#define ORBIT_GL_INTROSPECTION_WINDOW_H_

#include <stdint.h>

#include <memory>

#include "CaptureWindow.h"
#include "OrbitBase/Tracing.h"

class OrbitApp;

class IntrospectionWindow : public CaptureWindow {
 public:
  explicit IntrospectionWindow(OrbitApp* app);
  ~IntrospectionWindow() override;
  void ToggleRecording() override;
  void RenderImGuiDebugUI() override;

  [[nodiscard]] bool IsIntrospecting() const;
  void StartIntrospection();
  void StopIntrospection();

 protected:
  void Draw(bool viewport_was_dirty) override;
  void DrawScreenSpace() override;
  void RenderText(float layer) override;
  bool ShouldSkipRendering() const override { return false; };

 private:
  [[nodiscard]] const char* GetHelpText() const override;
  [[nodiscard]] bool ShouldAutoZoom() const override;

  std::unique_ptr<orbit_base::TracingListener> introspection_listener_;
};

#endif  // ORBIT_GL_INTROSPECTION_WINDOW_H_
