// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "Batcher.h"
#include "GlCanvas.h"
#include "GlSlider.h"
#include "TextBox.h"

class CaptureWindow : public GlCanvas {
 public:
  explicit CaptureWindow(uint32_t font_size);
  ~CaptureWindow() override;

  void Initialize() override;
  void ZoomAll();
  void Zoom(int delta);
  void Pan(float ratio);

  void UpdateWheelMomentum(float delta_time) override;
  void MouseMoved(int x, int y, bool left, bool right, bool middle) override;
  void LeftDoubleClick() override;
  void LeftDown(int x, int y) override;
  void LeftUp() override;
  void Pick();
  void Pick(int x, int y);
  void Pick(PickingId picking_id, int x, int y);
  void Hover(int x, int y) override;
  void RightDown(int x, int y) override;
  bool RightUp() override;
  void MiddleDown(int x, int y) override;
  void MiddleUp(int x, int y) override;
  void MouseWheelMoved(int x, int y, int delta, bool ctrl) override;
  void MouseWheelMovedHorizontally(int x, int y, int delta, bool ctrl) override;
  void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) override;
  void OnTimer() override;
  void Draw() override;
  void DrawScreenSpace() override;
  void RenderImGui() override;
  void RenderText(float layer) override;
  void PreRender() override;
  void PostRender() override;
  void Resize(int width, int height) override;
  void RenderHelpUi();
  void RenderTimeBar();
  void RenderSelectionOverlay();
  void SelectTextBox(const TextBox* text_box);

  void UpdateHorizontalScroll(float ratio);
  void UpdateVerticalScroll(float ratio);
  void UpdateHorizontalZoom(float normalized_start, float normalized_end);
  void UpdateVerticalSliderFromWorld();
  void UpdateHorizontalSliderFromWorld();
  void UpdateWorldTopLeftY(float val) override;

  void NeedsUpdate();
  void OnCaptureStarted();
  std::vector<std::string> GetContextMenu() override;
  void OnContextMenu(const std::string& action, int menu_index) override;
  virtual void ToggleCapture();
  void ToggleDrawHelp();
  void set_draw_help(bool draw_help);
  [[nodiscard]] TimeGraph* GetTimeGraph() { return &time_graph_; }

  Batcher& GetBatcherById(BatcherId batcher_id);

 protected:
  [[nodiscard]] virtual const char* GetHelpText();
  [[nodiscard]] virtual bool ShouldAutoZoom();

 protected:
  uint32_t font_size_;
  TimeGraph time_graph_;
  bool draw_help_;
  bool draw_filter_;
  std::shared_ptr<GlSlider> slider_;
  std::shared_ptr<GlSlider> vertical_slider_;

  bool click_was_drag_ = false;
  bool background_clicked_ = false;
};
