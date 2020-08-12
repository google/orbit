// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "Batcher.h"
#include "GlCanvas.h"
#include "GlSlider.h"
#include "TextBox.h"

struct ContextSwitch;

class CaptureWindow : public GlCanvas {
 public:
  CaptureWindow();
  ~CaptureWindow() override;

  void Initialize() override;
  void ZoomAll();
  void Zoom(int a_Delta);
  void Pan(float a_Ratio);

  void UpdateWheelMomentum(float a_DeltaTime) override;
  void MouseMoved(int a_X, int a_Y, bool a_Left, bool a_Right,
                  bool a_Middle) override;
  void LeftDoubleClick() override;
  void LeftDown(int a_X, int a_Y) override;
  void LeftUp() override;
  void Pick();
  void Pick(int a_X, int a_Y);
  void Pick(PickingId a_ID, int a_X, int a_Y);
  void Hover(int a_X, int a_Y);
  void FindCode(uint64_t address);
  void RightDown(int a_X, int a_Y) override;
  bool RightUp() override;
  void MiddleDown(int a_X, int a_Y) override;
  void MiddleUp(int a_X, int a_Y) override;
  void MouseWheelMoved(int a_X, int a_Y, int a_Delta, bool a_Ctrl) override;
  void MouseWheelMovedHorizontally(int a_X, int a_Y, int a_Delta,
                                   bool a_Ctrl) override;
  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                  bool a_Alt) override;
  void OnTimer() override;
  void Draw() override;
  void DrawScreenSpace() override;
  void RenderUI() override;
  void RenderText() override;
  void PreRender() override;
  void PostRender() override;
  void Resize(int a_Width, int a_Height) override;
  void RenderHelpUi();
  void RenderTimeBar();
  void ResetHoverTimer();
  void SelectTextBox(TextBox* text_box);
  void OnDrag(float a_Ratio);
  void OnVerticalDrag(float a_Ratio);
  void NeedsUpdate();
  void OnCaptureStarted();
  float GetTopBarTextY();
  std::vector<std::string> GetContextMenu() override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex) override;
  void UpdateVerticalSlider();
  void ToggleDrawHelp();

  Batcher& GetBatcherById(BatcherId batcher_id);

 private:
  [[nodiscard]] PickingMode GetPickingMode();

 private:
  TimeGraph time_graph_;
  OutputWindow m_StatsWindow;
  Timer m_HoverTimer;
  int m_HoverDelayMs;
  bool m_IsHovering;
  bool m_CanHover;
  bool m_DrawHelp;
  bool m_DrawFilter;
  bool m_FirstHelpDraw;
  bool m_DrawStats;
  std::shared_ptr<GlSlider> slider_;
  std::shared_ptr<GlSlider> vertical_slider_;

  static const std::string MENU_ACTION_GO_TO_CALLSTACK;
  static const std::string MENU_ACTION_GO_TO_SOURCE;
};
