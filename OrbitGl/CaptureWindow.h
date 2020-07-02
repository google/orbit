// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "Batcher.h"
#include "GlCanvas.h"
#include "GlSlider.h"

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
  void Pick(PickingID a_ID, int a_X, int a_Y);
  void Hover(int a_X, int a_Y);
  void FindCode(DWORD64 address);
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
  void DrawStatus();
  void RenderUI() override;
  void RenderText() override;
  void PreRender() override;
  void PostRender() override;
  void Resize(int a_Width, int a_Height) override;
  void RenderHelpUi();
  void RenderToolbars();
  void RenderMemTracker();
  void RenderTimeBar();
  void ResetHoverTimer();
  void SelectTextBox(class TextBox* a_TextBox);
  void OnDrag(float a_Ratio);
  void OnVerticalDrag(float a_Ratio);
  void NeedsUpdate();
  void OnCaptureStarted();
  float GetTopBarTextY();
  std::vector<std::string> GetContextMenu() override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex) override;
  void UpdateVerticalSlider();
  void SendProcess();

 private:
  void LoadIcons();

 private:
  TimeGraph time_graph_;
  OutputWindow m_StatsWindow;
  Timer m_HoverTimer;
  std::string m_ToolTip;
  int m_HoverDelayMs;
  bool m_IsHovering;
  bool m_CanHover;
  bool m_DrawHelp;
  bool m_DrawFilter;
  bool m_DrawMemTracker;
  bool m_FirstHelpDraw;
  bool m_DrawStats;
  GlSlider m_Slider;
  GlSlider m_VerticalSlider;
  int m_ProcessX;

  // Toolbars.
  uint64_t start_capture_icon_id_;
  uint64_t stop_capture_icon_id_;
  uint64_t save_capture_icon_id_;
  uint64_t load_capture_icon_id_;
  uint64_t clear_capture_icon_id_;
  uint64_t help_icon_id_;
  uint64_t filter_tracks_icon_id_;
  uint64_t search_icon_id_;
  uint64_t time_icon_id_;
  uint64_t feedback_icon_id_;
  uint64_t info_icon_id_;
  std::string icons_path_;
  float toolbar_height_ = 0;

  static constexpr size_t kFilterLength = 512;
  char track_filter_[kFilterLength] = "";
  char find_filter_[kFilterLength] = "";

  static const std::string MENU_ACTION_GO_TO_CALLSTACK;
  static const std::string MENU_ACTION_GO_TO_SOURCE;
};
