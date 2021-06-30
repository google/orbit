// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_WINDOW_H_
#define ORBIT_GL_CAPTURE_WINDOW_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "Batcher.h"
#include "CaptureStats.h"
#include "GlCanvas.h"
#include "GlSlider.h"
#include "OrbitAccessibility/AccessibleWidgetBridge.h"
#include "PickingManager.h"
#include "TextBox.h"
#include "TimeGraph.h"

class OrbitApp;

class CaptureWindow : public GlCanvas {
 public:
  explicit CaptureWindow(OrbitApp* app);

  enum class ZoomDirection { kHorizontal, kVertical };

  void ZoomAll();
  void Zoom(ZoomDirection dir, int delta);
  void Pan(float ratio);

  void MouseMoved(int x, int y, bool left, bool right, bool middle) override;
  void LeftDown(int x, int y) override;
  void LeftUp() override;
  void RightDown(int x, int y) override;
  bool RightUp() override;
  void MouseWheelMoved(int x, int y, int delta, bool ctrl) override;
  void MouseWheelMovedHorizontally(int x, int y, int delta, bool ctrl) override;
  void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) override;

  void SetIsMouseOver(bool value) override;

  void PostRender() override;
  void RenderImGuiDebugUI() override;

  void RequestUpdatePrimitives();
  [[nodiscard]] bool IsRedrawNeeded() const override;

  void ToggleDrawHelp();
  void set_draw_help(bool draw_help);

  [[nodiscard]] TimeGraph* GetTimeGraph() { return time_graph_.get(); }
  void CreateTimeGraph(const orbit_client_model::CaptureData* capture_data);
  void ClearTimeGraph() { time_graph_.reset(nullptr); }

  Batcher& GetBatcherById(BatcherId batcher_id);

 protected:
  void Draw(bool viewport_was_dirty) override;

  virtual void DrawScreenSpace();
  virtual void RenderText(float layer);
  virtual bool ShouldSkipRendering() const;

  virtual void ToggleRecording();

  orbit_gl::GlSlider* FindSliderUnderMouseCursor(int x, int y);

  void RenderHelpUi();
  void RenderTimeBar();
  void RenderSelectionOverlay();
  void SelectTextBox(const TextBox* text_box);

  void UpdateHorizontalScroll(float ratio);
  void UpdateVerticalScroll(float ratio);
  void UpdateHorizontalZoom(float normalized_start, float normalized_end);
  void UpdateVerticalSliderFromWorld();
  void UpdateHorizontalSliderFromWorld();

  void ProcessSliderMouseMoveEvents(int x, int y);

  [[nodiscard]] virtual const char* GetHelpText() const;
  [[nodiscard]] virtual bool ShouldAutoZoom() const;
  void HandlePickedElement(PickingMode picking_mode, PickingId picking_id, int x, int y) override;

  std::unique_ptr<TimeGraph> time_graph_ = nullptr;
  bool draw_help_;
  std::shared_ptr<orbit_gl::GlSlider> slider_;
  std::shared_ptr<orbit_gl::GlSlider> vertical_slider_;
  orbit_gl::GlSlider* last_mouseover_slider_ = nullptr;

  uint64_t select_start_time_ = 0;
  uint64_t select_stop_time_ = 0;

  bool click_was_drag_ = false;
  bool background_clicked_ = false;

  OrbitApp* app_ = nullptr;
  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  CaptureStats selection_stats_;
};

#endif  // ORBIT_GL_CAPTURE_WINDOW_H_
