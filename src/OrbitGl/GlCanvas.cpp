// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GlCanvas.h"

#include <GteVector.h>
#include <stdint.h>

#include <QOpenGLFunctions>
#include <array>
#include <cstring>

#include "ApiInterface/Orbit.h"
#include "OrbitAccessibility/AccessibleWidgetBridge.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/AccessibleInterfaceProvider.h"

// TODO(b/227341686) z-values should not be of `float` type. E.g. make them `uint`.
// Tracks: 0.0 - 0.1
// Tracks - Moving: 0.1 - 0.2
// Tracks - Pinned: 0.2 - 0.3
// TimeBar: 0.3 - 0.4
// World Overlay: 0.4 - 0.5
// UI: 0.6 - 0.7
// ScreenSpace: 0.8 - 0.9
float GlCanvas::kZValueTrack = 0.01f;
float GlCanvas::kZValueIncompleteDataOverlayPicking = 0.02f;
float GlCanvas::kZValueEventBar = 0.03f;
float GlCanvas::kZValueBox = 0.05f;
float GlCanvas::kZValueEvent = 0.06f;
float GlCanvas::kZValueBoxBorder = 0.07f;
float GlCanvas::kZValueTrackText = 0.08f;
float GlCanvas::kZValueTrackLabel = 0.09f;
float GlCanvas::kZValueTimeBar = 0.31f;
float GlCanvas::kZValueTimeBarLabel = 0.33f;
float GlCanvas::kZValueTimeBarMouseLabel = 0.35f;
float GlCanvas::kZValueIncompleteDataOverlay = 0.42f;
float GlCanvas::kZValueOverlay = 0.43f;
float GlCanvas::kZValueOverlayLabel = 0.45f;
float GlCanvas::kZValueEventBarPicking = 0.49f;
float GlCanvas::kZValueUi = 0.61f;
float GlCanvas::kZValueMargin = 0.85f;
float GlCanvas::kZValueButtonBg = 0.87f;
float GlCanvas::kZValueButton = 0.89f;

float GlCanvas::kZOffsetMovingTrack = 0.1f;
float GlCanvas::kZOffsetPinnedTrack = 0.2f;

constexpr unsigned kNumberOriginalLayers = 17;
constexpr unsigned kExtraLayersForMovingTracks = 6;
constexpr unsigned kExtraLayersForPinnedTracks = 6;
constexpr unsigned kExtraLayersForSliderEpsilons = 4;
unsigned GlCanvas::kMaxNumberRealZLayers = kNumberOriginalLayers + kExtraLayersForMovingTracks +
                                           kExtraLayersForPinnedTracks +
                                           kExtraLayersForSliderEpsilons;

const Color GlCanvas::kBackgroundColor = Color(67, 67, 67, 255);
const Color GlCanvas::kTimeBarBackgroundColor = Color(33, 32, 33, 255);
const Color GlCanvas::kTabTextColorSelected = Color(100, 181, 246, 255);

GlCanvas::GlCanvas()
    : AccessibleInterfaceProvider(),
      viewport_(0, 0),
      ui_batcher_(BatcherId::kUi),
      primitive_assembler_(&ui_batcher_, &picking_manager_) {
  // Note that `GlCanvas` is the bridge to OpenGl content, and `GlCanvas`'s parent needs special
  // handling for accessibility. Thus, we use `nullptr` here.
  text_renderer_.SetViewport(&viewport_);

  ResetHoverTimer();
}

bool GlCanvas::IsRedrawNeeded() const {
  return redraw_requested_ ||
         (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_) ||
         viewport_.IsDirty();
}

void GlCanvas::MouseMoved(int x, int y, bool left, bool /*right*/, bool /*middle*/) {
  mouse_move_pos_screen_ = Vec2i(x, y);
  Vec2 mouse_pos_world = viewport_.ScreenToWorld(mouse_move_pos_screen_);

  if (left) {
    picking_manager_.Drag(x, y);
  }

  if (is_selecting_) {
    select_stop_pos_world_ = mouse_pos_world;
  }

  ResetHoverTimer();
  RequestRedraw();
}

void GlCanvas::LeftDown(int x, int y) {
  // Store world clicked pos for panning
  mouse_click_pos_world_ = viewport_.ScreenToWorld(Vec2i(x, y));
  SetPickingMode(PickingMode::kClick);
  is_selecting_ = false;

  RequestRedraw();
}

void GlCanvas::MouseWheelMoved(int x, int y, int /*delta*/, bool /*ctrl*/) {
  mouse_move_pos_screen_ = Vec2i(x, y);

  ResetHoverTimer();
  RequestRedraw();
}

void GlCanvas::LeftUp() {
  picking_manager_.Release();
  RequestRedraw();
}

void GlCanvas::LeftDoubleClick() {
  double_clicking_ = true;
  SetPickingMode(PickingMode::kClick);
  RequestRedraw();
}

void GlCanvas::RightDown(int x, int y) {
  mouse_click_pos_world_ = viewport_.ScreenToWorld(Vec2i(x, y));

  select_start_pos_world_ = select_stop_pos_world_ = mouse_click_pos_world_;
  is_selecting_ = true;
  RequestRedraw();
}

void GlCanvas::RightUp() {
  is_selecting_ = false;
  RequestRedraw();
}

void GlCanvas::CharEvent(unsigned int /*character*/) { RequestRedraw(); }

void GlCanvas::KeyPressed(unsigned int /*key_code*/, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);
  RequestRedraw();
}

void GlCanvas::KeyReleased(unsigned int /*key_code*/, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);
  RequestRedraw();
}

void GlCanvas::UpdateSpecialKeys(bool ctrl, bool /*shift*/, bool /*alt*/) { control_key_ = ctrl; }

bool GlCanvas::ControlPressed() const { return control_key_; }

void GlCanvas::PrepareGlViewport() {
  glViewport(0, 0, viewport_.GetScreenWidth(), viewport_.GetScreenHeight());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, viewport_.GetScreenWidth(), viewport_.GetScreenHeight(), 0.f, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GlCanvas::PrepareGlState() {
  initializeOpenGLFunctions();

  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

  glClearColor(static_cast<float>(kBackgroundColor[0]) / 255.0f,
               static_cast<float>(kBackgroundColor[1]) / 255.0f,
               static_cast<float>(kBackgroundColor[2]) / 255.0f,
               static_cast<float>(kBackgroundColor[3]) / 255.0f);
  if (picking_mode_ != PickingMode::kNone) glClearColor(0.f, 0.f, 0.f, 0.f);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  picking_mode_ != PickingMode::kNone ? glDisable(GL_BLEND) : glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void GlCanvas::CleanupGlState() { glPopAttrib(); }

void GlCanvas::Render(QPainter* painter, int width, int height) {
  ORBIT_SCOPE("GlCanvas::Render");
  ORBIT_CHECK(width == viewport_.GetScreenWidth() && height == viewport_.GetScreenHeight());

  if (!IsRedrawNeeded()) {
    return;
  }
  painter->beginNativePainting();
  redraw_requested_ = false;
  ui_batcher_.ResetElements();

  PrepareGlState();
  PrepareGlViewport();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Clear text renderer
  text_renderer_.Init();
  text_renderer_.Clear();

  // Reset picking manager before each draw.
  picking_manager_.Reset();

  Draw(painter);

  glFlush();
  CleanupGlState();

  PostRender(painter);

  painter->endNativePainting();

  double_clicking_ = false;
  viewport_.ClearDirtyFlag();
}

void GlCanvas::PreRender() {
  if (viewport_.IsDirty()) {
    ResetHoverTimer();
  }

  if (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_) {
    SetPickingMode(PickingMode::kHover);
  }
}

void GlCanvas::PostRender(QPainter* painter) {
  PickingMode prev_picking_mode = picking_mode_;
  picking_mode_ = PickingMode::kNone;

  if (prev_picking_mode == PickingMode::kHover) {
    can_hover_ = false;
  }

  if (prev_picking_mode != PickingMode::kNone) {
    Pick(prev_picking_mode, mouse_move_pos_screen_[0], mouse_move_pos_screen_[1]);
    GlCanvas::Render(painter, viewport_.GetScreenWidth(), viewport_.GetScreenHeight());
  }
}

void GlCanvas::Resize(int width, int height) {
  viewport_.Resize(width, height);
  viewport_.SetWorldSize(width, height);
}

void GlCanvas::ResetHoverTimer() {
  hover_timer_.Restart();
  can_hover_ = true;
}

void GlCanvas::SetPickingMode(PickingMode mode) { picking_mode_ = mode; }

void GlCanvas::Pick(PickingMode picking_mode, int x, int y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  std::array<uint8_t, 4 * 1 * 1> pixels{};
  glReadPixels(x, viewport_.GetScreenHeight() - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
  uint32_t value;
  std::memcpy(&value, &pixels[0], sizeof(uint32_t));
  PickingId pick_id = PickingId::FromPixelValue(value);

  HandlePickedElement(picking_mode, pick_id, x, y);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> GlCanvas::CreateAccessibleInterface() {
  return std::make_unique<orbit_accessibility::AccessibleWidgetBridge>();
}