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
  explicit GlCanvas(uint32_t font_size);
  ~GlCanvas() override;

  void Initialize() override;
  void Resize(int width, int height) override;
  void Render(int width, int height) override;
  virtual void PostRender() {}

  virtual int GetWidth() const;
  virtual int GetHeight() const;

  void Prepare2DViewport(int top_left_x, int top_left_y, int bottom_right_x, int bottom_right_y);
  void PrepareScreenSpaceViewport();
  void PrepareGlState();
  static void CleanupGlState();
  void ScreenToWorld(int x, int y, float& wx, float& wy) const;
  float ScreenToWorldHeight(int height) const;
  float ScreenToWorldWidth(int width) const;

  // events
  void MouseMoved(int x, int y, bool left, bool right, bool middle) override;
  void LeftDown(int x, int y) override;
  void MouseWheelMoved(int x, int y, int delta, bool ctrl) override;
  void LeftUp() override;
  void LeftDoubleClick() override;
  void RightDown(int x, int y) override;
  bool RightUp() override;
  void CharEvent(unsigned int character) override;
  void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) override;
  void KeyReleased(unsigned int key_code, bool ctrl, bool shift, bool alt) override;
  virtual void UpdateSpecialKeys(bool ctrl, bool shift, bool alt);
  virtual bool ControlPressed();

  float GetWorldWidth() const { return world_width_; }
  float GetWorldHeight() const { return world_height_; }
  float GetWorldTopLeftX() const { return world_top_left_x_; }
  float GetWorldTopLeftY() const { return world_top_left_y_; }
  void SetWorldTopLeftY(float val) { world_top_left_y_ = val; }

  TextRenderer& GetTextRenderer() { return text_renderer_; }

  virtual void UpdateWheelMomentum(float delta_time);
  virtual void OnTimer();

  float GetMouseX() const { return mouse_x_; }

  float GetMousePosX() const { return static_cast<float>(mouse_pos_x_); }
  float GetMousePosY() const { return static_cast<float>(mouse_pos_y_); }

  void ResetHoverTimer();

  float GetDeltaTimeSeconds() const { return delta_time_; }

  virtual void Draw() {}
  virtual void DrawScreenSpace() {}
  virtual void RenderUI() {}
  virtual void RenderText() {}

  virtual void Hover(int /*X*/, int /*Y*/) {}

  ImGuiContext* GetImGuiContext() { return im_gui_context_; }
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
  static float kZValueOverlayTextBackground;
  static float kZValueOverlay;
  static float kZValueRoundingCorner;
  static float kZValueEvent;
  static float kZValueBox;
  static float kZValueEventBar;
  static float kZValueTrack;

  static const Color kBackgroundColor;
  static const Color kTabTextColorSelected;

 protected:
  [[nodiscard]] PickingMode GetPickingMode();

  int width_;
  int height_;
  float world_width_;
  float world_height_;
  float world_top_left_x_;
  float world_top_left_y_;
  float world_max_y_;
  float world_min_width_;
  float world_click_x_;
  float world_click_y_;
  float mouse_x_;
  float mouse_y_;
  int mouse_pos_x_;
  int mouse_pos_y_;
  Vec2 select_start_;
  Vec2 select_stop_;
  uint64_t time_start_;
  uint64_t time_stop_;
  int screen_click_x_;
  int screen_click_y_;
  int min_wheel_delta_;
  int max_wheel_delta_;
  float wheel_momentum_;
  float delta_time_;
  bool is_selecting_;
  double mouse_ratio_;
  bool im_gui_active_;
  Timer hover_timer_;
  int hover_delay_ms_;
  bool is_hovering_;
  bool can_hover_;

  ImGuiContext* im_gui_context_;
  uint64_t ref_time_click_;
  TextRenderer text_renderer_;
  Timer update_timer_;
  PickingManager picking_manager_;
  bool picking_;
  bool double_clicking_;
  bool control_key_;

  // Batcher to draw elements in the UI.
  Batcher ui_batcher_;
};

#endif  // ORBIT_GL_GL_CANVAS_H_
