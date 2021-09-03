// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_INTROSPECTION_WINDOW_H_
#define ORBIT_GL_INTROSPECTION_WINDOW_H_

#include <stdint.h>

#include <memory>

#include "CaptureClient/ApiEventProcessor.h"
#include "CaptureWindow.h"
#include "ClientData/CaptureData.h"
#include "Introspection/Introspection.h"

class OrbitApp;

class IntrospectionWindow : public CaptureWindow {
 public:
  explicit IntrospectionWindow(OrbitApp* app);
  ~IntrospectionWindow() override;
  void ToggleRecording() override;
  void RenderImGuiDebugUI() override;

  void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) override;

  [[nodiscard]] bool IsIntrospecting() const;
  void StartIntrospection();
  void StopIntrospection();

 protected:
  void Draw() override;
  void DrawScreenSpace() override;
  void RenderText(float layer) override;
  bool ShouldSkipRendering() const override { return false; };

 private:
  [[nodiscard]] const char* GetHelpText() const override;
  [[nodiscard]] bool ShouldAutoZoom() const override;

  std::unique_ptr<orbit_introspection::IntrospectionListener> introspection_listener_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;

  std::unique_ptr<orbit_capture_client::CaptureListener> capture_listener_;
  orbit_capture_client::ApiEventProcessor api_event_processor_;
};

#endif  // ORBIT_GL_INTROSPECTION_WINDOW_H_
