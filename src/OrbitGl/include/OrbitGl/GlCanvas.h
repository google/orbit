// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GL_CANVAS_H_
#define ORBIT_GL_GL_CANVAS_H_

#include <stdint.h>

#include <QOpenGLFunctions>
#include <QPainter>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/AccessibleInterfaceProvider.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/OpenGlBatcher.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/QtTextRenderer.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/Timer.h"
#include "OrbitGl/Viewport.h"

class GlCanvas : public orbit_gl::AccessibleInterfaceProvider, protected QOpenGLFunctions {
 public:
  explicit GlCanvas();
  virtual ~GlCanvas() = default;

  void Resize(int width, int height);
  void Render(QPainter* painter, int width, int height);

  virtual void PreRender();
  virtual void PostRender(QPainter* painter);

  void PrepareGlViewport();
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
  virtual void RightUp();
  virtual void MiddleDown(int x, int y) { RightDown(x, y); }
  virtual void MiddleUp() { RightUp(); }
  virtual void CharEvent(unsigned int character);
  virtual void KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt);
  virtual void KeyReleased(unsigned int key_code, bool ctrl, bool shift, bool alt);

  [[nodiscard]] orbit_gl::TextRenderer& GetTextRenderer() { return text_renderer_; }

  [[nodiscard]] virtual bool IsRedrawNeeded() const;
  void RequestRedraw() { redraw_requested_ = true; }

  virtual void SetIsMouseOver(bool value) { is_mouse_over_ = value; }

  [[nodiscard]] PickingManager& GetPickingManager() { return picking_manager_; }

  [[nodiscard]] const orbit_gl::Viewport& GetViewport() const { return viewport_; }
  [[nodiscard]] orbit_gl::Viewport& GetViewport() { return viewport_; }

  static float kZValueTrack;
  static float kZValueIncompleteDataOverlayPicking;
  static float kZValueEventBar;
  static float kZValueBox;
  static float kZValueBoxBorder;
  static float kZValueEvent;
  static float kZValueTrackText;
  static float kZValueTrackLabel;
  static float kZValueTimeBar;
  static float kZValueTimeBarLabel;
  static float kZValueTimeBarMouseLabel;
  static float kZValueIncompleteDataOverlay;
  static float kZValueOverlay;
  static float kZValueOverlayLabel;
  static float kZValueEventBarPicking;
  static float kZValueUi;
  static float kZValueMargin;
  static float kZValueButtonBg;
  static float kZValueButton;

  static float kZOffsetMovingTrack;
  static float kZOffsetPinnedTrack;
  static unsigned kMaxNumberRealZLayers;

  static const Color kBackgroundColor;
  static const Color kTimeBarBackgroundColor;
  static const Color kTabTextColorSelected;

 protected:
  virtual void Draw(QPainter* /*painter*/) {}

  void UpdateSpecialKeys(bool ctrl, bool shift, bool alt);
  [[nodiscard]] bool ControlPressed() const;

  void ResetHoverTimer();

  void SetPickingMode(PickingMode mode);

  Vec2 mouse_click_pos_world_;
  Vec2i mouse_move_pos_screen_ = Vec2i(0, 0);
  Vec2 select_start_pos_world_ = Vec2(0, 0);
  Vec2 select_stop_pos_world_ = Vec2(0, 0);

  bool is_selecting_{false};
  Timer hover_timer_;
  int hover_delay_ms_{300};
  bool can_hover_ = false;

  PickingMode picking_mode_ = PickingMode::kNone;

  double ref_time_click_{0.0};
  float track_container_click_scrolling_offset_ = 0;
  orbit_gl::QtTextRenderer text_renderer_;
  PickingManager picking_manager_;
  bool double_clicking_{false};
  bool control_key_{false};
  bool is_mouse_over_ = false;
  bool redraw_requested_{true};

  orbit_gl::Viewport viewport_;

  // PrimitiveAssembler to draw elements in the UI.
  orbit_gl::OpenGlBatcher ui_batcher_;
  orbit_gl::PrimitiveAssembler primitive_assembler_;

 private:
  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  void Pick(PickingMode picking_mode, int x, int y);
  virtual void HandlePickedElement(PickingMode /*picking_mode*/, PickingId /*picking_id*/,
                                   int /*x*/, int /*y*/) = 0;
};

#endif  // ORBIT_GL_GL_CANVAS_H_
