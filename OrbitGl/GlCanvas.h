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

  void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
  void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
  void prepareScreenSpaceViewport();
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
  float GetWorldTopLeftX() const { return m_WorldTopLeftX; }
  void SetWorldTopLeftX(float val) { m_WorldTopLeftX = val; }
  float GetWorldTopLeftY() const { return m_WorldTopLeftY; }
  void SetWorldTopLeftY(float val) { m_WorldTopLeftY = val; }

  TextRenderer& GetTextRenderer() { return m_TextRenderer; }
  void SetBackgroundColor(const Vec4& a_Color) { m_BackgroundColor = a_Color; }

  virtual void UpdateWheelMomentum(float a_DeltaTime);
  virtual void OnTimer();

  float GetMouseX() const { return m_MouseX; }
  float GetMouseY() const { return m_MouseY; }

  float GetMousePosX() const { return m_MousePosX; }
  float GetMousePosY() const { return m_MousePosY; }

  Vec2 ToScreenSpace(const Vec2& a_Point);
  Vec2 ToWorldSpace(const Vec2& a_Point);

  void AddText(const char* a_Text, float a_X, float a_Y, float a_Z, const Color& a_Color,
               float a_MaxSize = -1.f, bool a_RightJustified = false);
  int AddText2D(const char* a_Text, int a_X, int a_Y, float a_Z, const Color& a_Color,
                float a_MaxSize = -1.f, bool a_RightJustified = false, bool a_InvertY = true);

  void ResetHoverTimer();

  float GetDeltaTimeSeconds() const { return m_DeltaTime; }

  virtual void Draw() {}
  virtual void DrawScreenSpace() {}
  virtual void RenderUI() {}
  virtual void RenderText() {}

  virtual void Hover(int /*a_X*/, int /*a_Y*/) {}

  ImGuiContext* GetImGuiContext() { return m_ImGuiContext; }
  Batcher* GetBatcher() { return &ui_batcher_; }

  PickingManager& GetPickingManager() { return m_PickingManager; }

  static float Z_VALUE_SLIDER;
  static float Z_VALUE_MARGIN;
  static float Z_VALUE_TEXT_UI;
  static float Z_VALUE_UI;
  static float Z_VALUE_TIME_BAR_BG;
  static float Z_VALUE_OVERLAY;
  static float Z_VALUE_OVERLAY_BG;
  static float Z_VALUE_ROUNDING_CORNER;
  static float Z_VALUE_TEXT;
  static float Z_VALUE_EVENT;
  static float Z_VALUE_BOX;
  static float Z_VALUE_EVENT_BAR;
  static float Z_VALUE_TRACK;

  static float Z_VALUE_EVENT_BAR_PICKING;

 protected:
  [[nodiscard]] PickingMode GetPickingMode();

  int m_Width;
  int m_Height;
  float m_WorldWidth;
  float m_WorldHeight;
  float m_WorldTopLeftX;
  float m_WorldTopLeftY;
  float m_WorldMaxY;
  float m_WorldMinWidth;
  float m_WorldClickX;
  float m_WorldClickY;
  float m_MouseX;
  float m_MouseY;
  int m_MousePosX;
  int m_MousePosY;
  Vec2 m_SelectStart;
  Vec2 m_SelectStop;
  uint64_t m_TimeStart;
  uint64_t m_TimeStop;
  int m_ScreenClickX;
  int m_ScreenClickY;
  int m_MinWheelDelta;
  int m_MaxWheelDelta;
  float m_WheelMomentum;
  float m_DeltaTime;
  double m_DeltaTimeMs;
  bool m_IsSelecting;
  double m_MouseRatio;
  bool m_DrawUI;
  bool m_ImguiActive;
  int m_ID;
  Vec4 m_BackgroundColor;

  Timer m_HoverTimer;
  int m_HoverDelayMs;
  bool m_IsHovering;
  bool m_CanHover;

  ImGuiContext* m_ImGuiContext;
  uint64_t m_RefTimeClick;
  uint64_t m_SelectedInterval;
  TextRenderer m_TextRenderer;
  Timer m_UpdateTimer;
  PickingManager m_PickingManager;
  bool m_Picking;
  bool m_DoubleClicking;
  bool m_ControlKey;
  bool m_ShiftKey;
  bool m_AltKey;

  // Batcher to draw elements in the UI.
  Batcher ui_batcher_;
};

#endif  // ORBIT_GL_GL_CANVAS_H_
