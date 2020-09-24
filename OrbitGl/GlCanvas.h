// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_CANVAS_H_
#define ORBIT_GL_GL_CANVAS_H_

#include "GlPanel.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "PickingManager.h"
#include "ScopeTimer.h"
#include "TextRenderer.h"
#include "TimeGraph.h"

class GlCanvas : public GlPanel {
 public:
  GlCanvas();
  ~GlCanvas() override;

  void Initialize() override;
  void Resize(int a_Width, int a_Height) override;
  void Render(int a_Width, int a_Height) override;
  virtual void PostRender() {}

  int getWidth() const;
  int getHeight() const;

  void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
  void prepareScreenSpaceViewport();
  void prepareGlState();
  void cleanupGlState();
  void ScreenToWorld(int x, int y, float& wx, float& wy) const;
  void WorldToScreen(float wx, float wy, int& x, int& y) const;
  int WorldToScreenHeight(float a_Height) const;
  float ScreenToWorldHeight(int a_Height) const;
  float ScreenToworldWidth(int a_Width) const;

  // events
  void MouseMoved(int a_X, int a_Y, bool a_Left, bool a_Right, bool a_Middle) override;
  void LeftDown(int a_X, int a_Y) override;
  void MouseWheelMoved(int a_X, int a_Y, int a_Delta, bool a_Ctrl) override;
  void LeftUp() override;
  void LeftDoubleClick() override;
  void RightDown(int a_X, int a_Y) override;
  bool RightUp() override;
  virtual void mouseLeftWindow();
  void CharEvent(unsigned int a_Char) override;
  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt) override;
  void KeyReleased(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt) override;
  virtual void UpdateSpecialKeys(bool a_Ctrl, bool a_Shift, bool a_Alt);
  virtual bool ControlPressed();
  virtual bool ShiftPressed();
  virtual bool AltPressed();

  float GetWorldWidth() const { return m_WorldWidth; }
  void SetWorldWidth(float val) { m_WorldWidth = val; }
  float GetWorldHeight() const { return m_WorldHeight; }
  void SetWorldHeight(float val) { m_WorldHeight = val; }
  float GetWorldTopLeftX() const { return world_top_left_x_; }
  void SetWorldTopLeftX(float val) { world_top_left_x_ = val; }
  float GetWorldTopLeftY() const { return world_top_left_y_; }
  void SetWorldTopLeftY(float val) { world_top_left_y_ = val; }

  TextRenderer& GetTextRenderer() { return m_TextRenderer; }

  virtual void UpdateWheelMomentum(float a_DeltaTime);
  virtual void OnTimer();

  float GetMouseX() const { return mouse_x_; }
  float GetMouseY() const { return mouse_y_; }

  float GetMousePosX() const { return mouse_pos_x_; }
  float GetMousePosY() const { return mouse_pos_y_; }

  Vec2 ToScreenSpace(const Vec2& a_Point);
  Vec2 ToWorldSpace(const Vec2& a_Point);

  void ResetHoverTimer();

  float GetDeltaTimeSeconds() const { return m_DeltaTime; }

  virtual void Draw() {}
  virtual void DrawScreenSpace() {}
  virtual void RenderUI() {}
  virtual void RenderText() {}

  virtual void Hover(int /*a_X*/, int /*a_Y*/) {}

  ImGuiContext* GetImGuiContext() { return m_ImGuiContext; }
  Batcher* GetBatcher() { return &ui_batcher_; }

  PickingManager& GetPickingManager() { return picking_manager_; }

  static float kZValueSlider;
  static float kZValueSliderBg;
  static float kZValueMargin;
  static float kZValueTextUi;
  static float kZValueUi;
  static float kZValueEventBarPicking;
  static float kZValueTimeBarBg;
  static float kZValueText;
  static float kZValueOverlay;
  static float kZValueOverlayBg;
  static float kZValueRoundingCorner;
  static float kZValueEvent;
  static float kZValueBox;
  static float kZValueEventBar;
  static float kZValueTrack;

  static const Color kBackgroundColor;
  static const Color kTabColor;
  static const Color kTabTextColorSelected;

 protected:
  [[nodiscard]] PickingMode GetPickingMode();

  int m_Width;
  int m_Height;
  float m_WorldWidth;
  float m_WorldHeight;
  float world_top_left_x_;
  float world_top_left_y_;
  float world_max_y_;
  float m_WorldMinWidth;
  float world_click_x_;
  float world_click_y_;
  float mouse_x_;
  float mouse_y_;
  int mouse_pos_x_;
  int mouse_pos_y_;
  Vec2 select_start_;
  Vec2 select_stop_;
  uint64_t m_TimeStart;
  uint64_t time_stop;
  int screen_click_x_;
  int screen_click_y_;
  int m_MinWheelDelta;
  int m_MaxWheelDelta;
  float m_WheelMomentum;
  float m_DeltaTime;
  double m_DeltaTimeMs;
  bool is_selecting_;
  double m_MouseRatio;
  bool draw_ui_;
  bool m_ImguiActive;
  int m_ID;

  Timer m_HoverTimer;
  int m_HoverDelayMs;
  bool m_IsHovering;
  bool m_CanHover;

  ImGuiContext* m_ImGuiContext;
  uint64_t ref_time_click_;
  uint64_t m_SelectedInterval;
  TextRenderer m_TextRenderer;
  Timer m_UpdateTimer;
  PickingManager picking_manager_;
  bool picking_;
  bool m_DoubleClicking;
  bool m_ControlKey;
  bool m_ShiftKey;
  bool m_AltKey;

  // Batcher to draw elements in the UI.
  Batcher ui_batcher_;
};

#endif  // ORBIT_GL_GL_CANVAS_H_
