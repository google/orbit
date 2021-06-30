// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_CANVAS_H_
#define ORBIT_GL_GL_CANVAS_H_

#include <imgui.h>
#include <stdint.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AccessibleInterfaceProvider.h"
#include "AccessibleTimeGraph.h"
#include "Batcher.h"
#include "CoreMath.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitAccessibility/AccessibleWidgetBridge.h"
#include "PickingManager.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "Timer.h"
#include "Viewport.h"

class OrbitApp;

class GlCanvas : public orbit_gl::AccessibleInterfaceProvider {
 public:
  explicit GlCanvas();
  virtual ~GlCanvas();

  enum class CanvasType { kCaptureWindow, kIntrospectionWindow, kDebug };
  static std::unique_ptr<GlCanvas> Create(CanvasType canvas_type, OrbitApp* app);

  void Resize(int width, int height);
  void Render(int width, int height);

  virtual void PreRender();
  virtual void PostRender();

  void PrepareWorldSpaceViewport();
  void PrepareScreenSpaceViewport();
  void PrepareGlState();
  static void CleanupGlState();

  // events
  virtual void MouseMoved(int x, int y, bool left, bool right, bool middle);
  virtual void LeftDown(int x, int y);
  virtual void LeftUp();
  virtual void LeftDoubleClick();
  virtual void MouseWheelMoved(int x, int y, int delta, bool ctrl);
  virtual void MouseWheelMovedHorizontally(int /*x*/, int /*y*/, int /*delta*/, bool /*ctrl*/) {}
  virtual void RightDown(int x, int y);
  virtual bool RightUp();
  virtual void MiddleDown(int x, int y) { RightDown(x, y); }
  virtual void MiddleUp() { RightUp(); }
  virtual void CharEvent(unsigned int character);
  virtual void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt);
  virtual void KeyReleased(unsigned int key_code, bool ctrl, bool shift, bool alt);

  [[nodiscard]] virtual std::vector<std::string> GetContextMenu() {
    return std::vector<std::string>();
  }
  virtual void OnContextMenu(const std::string& /*a_Action*/, int /*a_MenuIndex*/) {}

  [[nodiscard]] TextRenderer& GetTextRenderer() { return text_renderer_; }

  [[nodiscard]] const Vec2i& GetMouseScreenPos() const { return mouse_move_pos_screen_; }

  // This could be removed if ImGui is removed
  [[nodiscard]] float GetDeltaTimeSeconds() const { return delta_time_; }

  virtual void RenderImGuiDebugUI() {}

  using RenderCallback = std::function<void()>;
  void AddRenderCallback(RenderCallback callback) {
    render_callbacks_.emplace_back(std::move(callback));
  }

  void EnableImGui();
  [[nodiscard]] ImGuiContext* GetImGuiContext() const { return imgui_context_; }
  [[nodiscard]] Batcher& GetBatcher() { return ui_batcher_; }

  [[nodiscard]] virtual bool IsRedrawNeeded() const;
  void RequestRedraw() { redraw_requested_ = true; }

  [[nodiscard]] bool GetIsMouseOver() const { return is_mouse_over_; }
  virtual void SetIsMouseOver(bool value) { is_mouse_over_ = value; }

  [[nodiscard]] PickingManager& GetPickingManager() { return picking_manager_; }

  [[nodiscard]] const orbit_gl::Viewport& GetViewport() const { return viewport_; }
  [[nodiscard]] orbit_gl::Viewport& GetViewport() { return viewport_; }

  static float kZValueSlider;
  static float kZValueSliderBg;
  static float kZValueMargin;
  static float kZValueTimeBar;
  static float kZValueTimeBarBg;
  static float kScreenSpaceCutPoint;
  static float kZValueTextUi;
  static float kZValueUi;
  static float kZValueEventBarPicking;
  static float kZValueOverlayTextBackground;
  static float kZValueOverlay;
  static float kZValueTrackText;
  static float kZValueTrackLabel;
  static float kZValueEvent;
  static float kZValueBox;
  static float kZValueEventBar;
  static float kZValueTrack;

  static float kZOffsetMovingTrack;
  static float kZOffsetPinnedTrack;
  static unsigned kMaxNumberRealZLayers;

  static const Color kBackgroundColor;
  static const Color kTimeBarBackgroundColor;
  static const Color kTabTextColorSelected;

 protected:
  virtual void Draw(bool /*viewport_was_dirty*/) {}

  void UpdateSpecialKeys(bool ctrl, bool shift, bool alt);
  bool ControlPressed();

  void ResetHoverTimer();

  void SetPickingMode(PickingMode mode);

  Vec2 mouse_click_pos_world_;
  Vec2i mouse_move_pos_screen_ = Vec2i(0, 0);
  Vec2 select_start_pos_world_ = Vec2(0, 0);
  Vec2 select_stop_pos_world_ = Vec2(0, 0);

  float delta_time_;
  bool is_selecting_;
  Timer hover_timer_;
  int hover_delay_ms_;
  bool can_hover_;

  PickingMode picking_mode_ = PickingMode::kNone;

  ImGuiContext* imgui_context_ = nullptr;
  double ref_time_click_;
  TextRenderer text_renderer_;
  PickingManager picking_manager_;
  bool double_clicking_;
  bool control_key_;
  bool is_mouse_over_ = false;
  bool redraw_requested_;

  orbit_gl::Viewport viewport_;

  // Batcher to draw elements in the UI.
  Batcher ui_batcher_;
  std::vector<RenderCallback> render_callbacks_;

 private:
  [[nodiscard]] virtual std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  void Pick(PickingMode picking_mode, int x, int y);
  virtual void HandlePickedElement(PickingMode /*picking_mode*/, PickingId /*picking_id*/,
                                   int /*x*/, int /*y*/) {}
};

#endif  // ORBIT_GL_GL_CANVAS_H_
