// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_WINDOW_H_
#define ORBIT_GL_CAPTURE_WINDOW_H_

#include <absl/container/btree_map.h>
#include <stdint.h>

#include <QPainter>
#include <memory>
#include <string>
#include <vector>

#include "CaptureClient/AppInterface.h"
#include "ClientData/CaptureData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/Batcher.h"
#include "OrbitGl/CaptureStats.h"
#include "OrbitGl/CaptureWindowDebugInterface.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/SimpleTimings.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "absl/container/btree_map.h"

class OrbitApp;

class CaptureWindow : public GlCanvas, public orbit_gl::CaptureWindowDebugInterface {
 public:
  explicit CaptureWindow(OrbitApp* app,
                         orbit_capture_client::CaptureControlInterface* capture_control,
                         TimeGraphLayout* time_graph_layout);

  void PreRender() override;

  void ZoomAll();
  void ZoomHorizontally(int delta, int mouse_x);
  void Pan(float ratio);

  void MouseMoved(int x, int y, bool left, bool right, bool middle) override;
  void LeftDown(int x, int y) override;
  void LeftUp() override;
  void RightDown(int x, int y) override;
  void RightUp() override;
  void MouseWheelMoved(int x, int y, int delta, bool ctrl) override;
  void MouseWheelMovedHorizontally(int x, int y, int delta, bool ctrl) override;
  void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) override;

  void SetIsMouseOver(bool value) override;

  void PostRender(QPainter* painter) override;

  void RequestUpdatePrimitives();
  [[nodiscard]] bool IsRedrawNeeded() const override;

  void set_draw_help(bool draw_help);
  [[nodiscard]] bool get_draw_help() const { return draw_help_; }

  [[nodiscard]] TimeGraph* GetTimeGraph() { return time_graph_.get(); }
  void CreateTimeGraph(orbit_client_data::CaptureData* capture_data);
  void ClearTimeGraph() { time_graph_.reset(nullptr); }

  [[nodiscard]] std::string GetCaptureInfo() const override;
  [[nodiscard]] std::string GetPerformanceInfo() const override;
  [[nodiscard]] std::string GetSelectionSummary() const override;

 protected:
  void Draw(QPainter* painter) override;

  void RenderAllLayers(QPainter* painter);

  virtual void RenderText(QPainter* painter, float layer);
  virtual bool ShouldSkipRendering() const;

  virtual void ToggleRecording();

  void RenderHelpUi();
  void RenderSelectionOverlay();
  void SelectTimer(const orbit_client_protos::TimerInfo* timer_info);

  [[nodiscard]] virtual const char* GetHelpText() const;
  [[nodiscard]] virtual bool ShouldAutoZoom() const;
  void HandlePickedElement(PickingMode picking_mode, PickingId picking_id, int x, int y) override;
  orbit_gl::Batcher& GetBatcherById(BatcherId batcher_id);

  std::unique_ptr<TimeGraph> time_graph_ = nullptr;
  bool draw_help_;

  uint64_t select_start_time_ = 0;
  uint64_t select_stop_time_ = 0;

  uint64_t last_frame_start_time_ = 0;

  bool click_was_drag_ = false;
  bool background_clicked_ = false;

  OrbitApp* app_ = nullptr;
  orbit_capture_client::CaptureControlInterface* capture_client_app_ = nullptr;

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  CaptureStats selection_stats_;

  absl::btree_map<std::string, std::unique_ptr<orbit_gl::SimpleTimings>> scoped_frame_times_;

 private:
  TimeGraphLayout* time_graph_layout_ = nullptr;
};

#endif  // ORBIT_GL_CAPTURE_WINDOW_H_
