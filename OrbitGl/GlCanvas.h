// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_CANVAS_H_
#define ORBIT_GL_GL_CANVAS_H_

#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "PickingManager.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "Timer.h"

class GlCanvas {
 public:
  explicit GlCanvas(uint32_t font_size);
  virtual ~GlCanvas();

  enum class CanvasType { kCaptureWindow, kIntrospectionWindow, kDebug };
  static std::unique_ptr<GlCanvas> Create(CanvasType canvas_type, uint32_t font_size);

  virtual void Initialize();
  virtual void Resize(int width, int height);
  virtual void Render(int width, int height);
  virtual void PreRender(){};
  virtual void PostRender() {}

  virtual int GetWidth() const;
  virtual int GetHeight() const;

  virtual void SetMainWindowSize(int width, int height) {
    m_MainWindowWidth = width;
    m_MainWindowHeight = height;
  }

  void Prepare2DViewport(int top_left_x, int top_left_y, int bottom_right_x, int bottom_right_y);
  void PrepareScreenSpaceViewport();
  void PrepareGlState();
  static void CleanupGlState();
  void ScreenToWorld(int x, int y, float& wx, float& wy) const;
  Vec2 ScreenToWorld(Vec2 screen_pos);
  float ScreenToWorldHeight(int height) const;
  float ScreenToWorldWidth(int width) const;

  // events
  virtual void MouseMoved(int x, int y, bool left, bool right, bool middle);
  virtual void LeftDown(int x, int y);
  virtual void LeftUp();
  virtual void LeftDoubleClick();
  virtual void MouseWheelMoved(int x, int y, int delta, bool ctrl);
  virtual void MouseWheelMovedHorizontally(int x, int y, int delta, bool ctrl) {
    MouseWheelMoved(x, y, delta, ctrl);
  }
  virtual void RightDown(int x, int y);
  virtual bool RightUp();
  virtual void MiddleDown(int /*x*/, int /*y*/) {}
  virtual void MiddleUp(int /*x*/, int /*y*/) {}
  virtual void CharEvent(unsigned int character);
  virtual void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt);
  virtual void KeyReleased(unsigned int key_code, bool ctrl, bool shift, bool alt);
  virtual void UpdateSpecialKeys(bool ctrl, bool shift, bool alt);
  virtual bool ControlPressed();

  virtual std::vector<std::string> GetContextMenu() { return std::vector<std::string>(); }
  virtual void OnContextMenu(const std::string& /*a_Action*/, int /*a_MenuIndex*/) {}

  float GetWorldWidth() const { return world_width_; }
  float GetWorldHeight() const { return world_height_; }
  float GetWorldMaxY() const { return world_max_y_; }
  float GetWorldTopLeftX() const { return world_top_left_x_; }
  float GetWorldTopLeftY() const { return world_top_left_y_; }
  virtual void UpdateWorldTopLeftY(float val) { world_top_left_y_ = val; }

  TextRenderer& GetTextRenderer() { return text_renderer_; }
  uint32_t GetInitialFontSize() const { return initial_font_size_; }

  virtual void UpdateWheelMomentum(float delta_time);
  virtual void OnTimer();

  float GetMouseX() const { return mouse_world_x_; }

  float GetMousePosX() const { return static_cast<float>(mouse_screen_x_); }
  float GetMousePosY() const { return static_cast<float>(mouse_screen_y_); }

  void ResetHoverTimer();

  float GetDeltaTimeSeconds() const { return delta_time_; }

  virtual void Draw() {}
  virtual void DrawScreenSpace() {}
  virtual void RenderImGui() {}
  virtual void RenderText(float) {}

  virtual void Hover(int /*X*/, int /*Y*/) {}

  using RenderCallback = std::function<void()>;
  void AddRenderCallback(RenderCallback callback) {
    render_callbacks_.emplace_back(std::move(callback));
  }

  void EnableImGui();
  [[nodiscard]] ImGuiContext* GetImGuiContext() const { return imgui_context_; }
  [[nodiscard]] Batcher* GetBatcher() { return &ui_batcher_; }

  [[nodiscard]] virtual bool GetNeedsRedraw() const { return m_NeedsRedraw; }
  void NeedsRedraw() { m_NeedsRedraw = true; }

  [[nodiscard]] virtual bool GetNeedsCheckHighlightChange() const {
    return needs_check_highlight_change_;
  }
  void ResetNeedsCheckHighlightChange() { needs_check_highlight_change_ = false; };

  [[nodiscard]] bool GetIsMouseOver() const { return is_mouse_over_; }
  void SetIsMouseOver(bool value) { is_mouse_over_ = value; }

  [[nodiscard]] PickingManager& GetPickingManager() { return picking_manager_; }

  static float kZValueSlider;
  static float kZValueSliderBg;
  static float kZValueMargin;
  static float kZValueTimeBar;
  static float kZValueTimeBarBg;
  static float kScreenSpaceCutPoint;
  static float kZValueTextUi;
  static float kZValueUi;
  static float kZValueEventBarPicking;
  static float kZValueText;
  static float kZValueOverlayTextBackground;
  static float kZValueOverlay;
  static float kZValueRoundingCorner;
  static float kZValueEvent;
  static float kZValueBox;
  static float kZValueEventBar;
  static float kZValueTrack;

  static float kZOffsetMovingTack;
  static float kZOffsetPinnedTrack;
  static unsigned kMaxNumberRealZLayers;

  static const Color kBackgroundColor;
  static const Color kTabTextColorSelected;

 protected:
  [[nodiscard]] PickingMode GetPickingMode();

  int screen_width_;
  int screen_height_;
  float world_width_;
  float world_height_;
  float world_top_left_x_;
  float world_top_left_y_;
  float world_max_y_;
  float world_min_width_;
  float world_click_x_;
  float world_click_y_;
  float mouse_world_x_;
  float mouse_world_y_;
  int mouse_screen_x_;
  int mouse_screen_y_;
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
  uint32_t initial_font_size_;

  ImGuiContext* imgui_context_ = nullptr;
  double ref_time_click_;
  TextRenderer text_renderer_;
  Timer update_timer_;
  PickingManager picking_manager_;
  bool picking_;
  bool double_clicking_;
  bool control_key_;
  bool needs_check_highlight_change_ = false;
  bool is_mouse_over_ = false;
  bool m_NeedsRedraw;
  int m_MainWindowWidth = 0;
  int m_MainWindowHeight = 0;

  // Batcher to draw elements in the UI.
  Batcher ui_batcher_;
  std::vector<RenderCallback> render_callbacks_;
};

#endif  // ORBIT_GL_GL_CANVAS_H_
