// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_INTROSPECTION_WINDOW_H_
#define ORBIT_GL_INTROSPECTION_WINDOW_H_

#include <stdint.h>

#include <QPainter>
#include <memory>

#include "CaptureClient/ApiEventProcessor.h"
#include "CaptureClient/AppInterface.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/CaptureData.h"
#include "Introspection/Introspection.h"
#include "OrbitGl/CaptureWindow.h"
#include "OrbitGl/TimeGraphLayout.h"

class OrbitApp;

class IntrospectionWindow : public CaptureWindow {
 public:
  explicit IntrospectionWindow(OrbitApp* app,
                               orbit_capture_client::CaptureControlInterface* capture_control,
                               TimeGraphLayout* time_graph_layout);
  ~IntrospectionWindow() override;
  void ToggleRecording() override;

  void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) override;

  [[nodiscard]] bool IsIntrospecting() const;
  void StartIntrospection();
  void StopIntrospection();

 protected:
  void Draw(QPainter* painter) override;
  void RenderText(QPainter* painter, float layer) override;
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
