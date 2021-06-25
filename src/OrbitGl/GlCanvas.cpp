// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlCanvas.h"

#include <GteVector.h>
#include <absl/base/casts.h>
#include <glad/glad.h>
#include <limits.h>
#include <math.h>

#include "AccessibleInterfaceProvider.h"
#include "App.h"
#include "CaptureWindow.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "IntrospectionWindow.h"
#include "OrbitAccessibility/AccessibleWidgetBridge.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"

// Tracks: 0.0 - 0.1
// World Overlay: 0.4 - 0.5
// UI: 0.6 - 0.7
// ScreenSpace: 0.8 - 0.9
float GlCanvas::kZValueTrack = 0.01f;
float GlCanvas::kZValueEventBar = 0.03f;
float GlCanvas::kZValueBox = 0.05f;
float GlCanvas::kZValueEvent = 0.07f;
float GlCanvas::kZValueTrackText = 0.08f;
float GlCanvas::kZValueTrackLabel = 0.09f;
float GlCanvas::kZValueOverlay = 0.43f;
float GlCanvas::kZValueOverlayTextBackground = 0.45f;
float GlCanvas::kZValueEventBarPicking = 0.49f;
float GlCanvas::kZValueUi = 0.61f;
float GlCanvas::kZValueTextUi = 0.61f;
float GlCanvas::kScreenSpaceCutPoint = 0.8f;
float GlCanvas::kZValueTimeBarBg = 0.81f;
float GlCanvas::kZValueTimeBar = 0.83f;
float GlCanvas::kZValueMargin = 0.85f;
float GlCanvas::kZValueSliderBg = 0.87f;
float GlCanvas::kZValueSlider = 0.89f;

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
const Color GlCanvas::kTimeBarBackgroundColor = Color(90, 90, 90, 255);
const Color GlCanvas::kTabTextColorSelected = Color(100, 181, 246, 255);

GlCanvas::GlCanvas()
    : AccessibleInterfaceProvider(),
      viewport_(0, 0),
      ui_batcher_(BatcherId::kUi, &picking_manager_) {
  // Note that `GlCanvas` is the bridge to OpenGl content, and `GlCanvas`'s parent needs special
  // handling for accessibility. Thus, we use `nullptr` here.
  text_renderer_.SetViewport(&viewport_);

  is_selecting_ = false;
  double_clicking_ = false;
  control_key_ = false;
  redraw_requested_ = true;

  ref_time_click_ = 0.0;

  delta_time_ = 0.0f;

  hover_delay_ms_ = 300;
  ResetHoverTimer();
}

GlCanvas::~GlCanvas() {
  if (imgui_context_ != nullptr) {
    Orbit_ImGui_Shutdown();
    ImGui::DestroyContext();
  }
}

std::unique_ptr<GlCanvas> GlCanvas::Create(CanvasType canvas_type, OrbitApp* app) {
  switch (canvas_type) {
    case CanvasType::kCaptureWindow: {
      auto main_capture_window = std::make_unique<CaptureWindow>(app);
      app->SetCaptureWindow(main_capture_window.get());
      return main_capture_window;
    }
    case CanvasType::kIntrospectionWindow: {
      auto introspection_window = std::make_unique<IntrospectionWindow>(app);
      app->SetIntrospectionWindow(introspection_window.get());
      return introspection_window;
    }
    case CanvasType::kDebug:
      return std::make_unique<GlCanvas>();
    default:
      UNREACHABLE();
  }
}

void GlCanvas::EnableImGui() {
  if (imgui_context_ == nullptr) {
    imgui_context_ = ImGui::CreateContext();
    constexpr uint32_t kImGuiFontSize = 12;
    Orbit_ImGui_Init(kImGuiFontSize);
  }
}

bool GlCanvas::IsRedrawNeeded() const {
  return redraw_requested_ ||
         (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_) ||
         viewport_.IsDirty();
}

void GlCanvas::MouseMoved(int x, int y, bool left, bool /*right*/, bool /*middle*/) {
  mouse_move_pos_screen_ = Vec2i(x, y);
  Vec2 mouse_pos_world = viewport_.ScreenToWorldPos(mouse_move_pos_screen_);

  // Pan
  if (left && !picking_manager_.IsDragging()) {
    viewport_.SetWorldTopLeftX(mouse_click_pos_world_[0] - viewport_.ScreenToWorldWidth(x));
    viewport_.SetWorldTopLeftY(mouse_click_pos_world_[1] + viewport_.ScreenToWorldHeight(y));
  }

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
  mouse_click_pos_world_ = viewport_.ScreenToWorldPos(Vec2i(x, y));
  SetPickingMode(PickingMode::kClick);
  is_selecting_ = false;

  Orbit_ImGui_MouseButtonCallback(imgui_context_, 0, true);

  RequestRedraw();
}

void GlCanvas::MouseWheelMoved(int /*x*/, int /*y*/, int delta, bool /*ctrl*/) {
  // Normalize and invert sign, so that delta < 0 is zoom in.
  int delta_normalized = delta < 0 ? 1 : -1;

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(imgui_context_, -delta_normalized);

  ResetHoverTimer();
  RequestRedraw();
}

void GlCanvas::LeftUp() {
  picking_manager_.Release();
  Orbit_ImGui_MouseButtonCallback(imgui_context_, 0, false);
  RequestRedraw();
}

void GlCanvas::LeftDoubleClick() {
  double_clicking_ = true;
  SetPickingMode(PickingMode::kClick);
  RequestRedraw();
}

void GlCanvas::RightDown(int x, int y) {
  mouse_click_pos_world_ = viewport_.ScreenToWorldPos(Vec2i(x, y));

  select_start_pos_world_ = select_stop_pos_world_ = mouse_click_pos_world_;
  is_selecting_ = true;
  SetPickingMode(PickingMode::kClick);

  Orbit_ImGui_MouseButtonCallback(imgui_context_, 1, true);
  RequestRedraw();
}

bool GlCanvas::RightUp() {
  Orbit_ImGui_MouseButtonCallback(imgui_context_, 1, false);
  is_selecting_ = false;
  RequestRedraw();

  bool show_context_menu = select_start_pos_world_ == select_stop_pos_world_;
  return show_context_menu;
}

void GlCanvas::CharEvent(unsigned int character) {
  Orbit_ImGui_CharCallback(imgui_context_, character);
  RequestRedraw();
}

void GlCanvas::KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);
  Orbit_ImGui_KeyCallback(imgui_context_, static_cast<int>(key_code), true, ctrl, shift, alt);
  RequestRedraw();
}

void GlCanvas::KeyReleased(unsigned int key_code, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);
  Orbit_ImGui_KeyCallback(imgui_context_, static_cast<int>(key_code), false, ctrl, shift, alt);
  RequestRedraw();
}

void GlCanvas::UpdateSpecialKeys(bool ctrl, bool /*shift*/, bool /*alt*/) { control_key_ = ctrl; }

bool GlCanvas::ControlPressed() { return control_key_; }

/** Inits the OpenGL viewport for drawing in 2D. */
void GlCanvas::PrepareWorldSpaceViewport() {
  const int top_left_x = 0, top_left_y = 0, bottom_right_x = viewport_.GetScreenWidth(),
            bottom_right_y = viewport_.GetScreenHeight();
  glViewport(top_left_x, top_left_y, bottom_right_x - top_left_x, bottom_right_y - top_left_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  Vec2 top_left = viewport_.GetWorldTopLeft();
  float width = viewport_.GetVisibleWorldWidth();
  float height = viewport_.GetVisibleWorldHeight();

  glOrtho(top_left[0], top_left[0] + width, top_left[1] - height, top_left[1], -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GlCanvas::PrepareScreenSpaceViewport() {
  ORBIT_SCOPE_FUNCTION;
  glViewport(0, 0, viewport_.GetScreenWidth(), viewport_.GetScreenHeight());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, viewport_.GetScreenWidth(), 0, viewport_.GetScreenHeight(), -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GlCanvas::PrepareGlState() {
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
  glEnable(GL_DEPTH_TEST);  // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);   // The Type Of Depth Testing To Do
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void GlCanvas::CleanupGlState() { glPopAttrib(); }

void GlCanvas::Render(int width, int height) {
  ORBIT_SCOPE("GlCanvas::Render");
  CHECK(width == viewport_.GetScreenWidth() && height == viewport_.GetScreenHeight());

  if (!IsRedrawNeeded()) {
    return;
  }

  redraw_requested_ = false;
  ui_batcher_.StartNewFrame();

  PrepareGlState();
  PrepareWorldSpaceViewport();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  // Clear text renderer
  text_renderer_.Init();
  text_renderer_.Clear();

  // Reset picking manager before each draw.
  picking_manager_.Reset();

  bool viewport_was_dirty = viewport_.IsDirty();
  // Clear viewport dirty flag so it can be set again during rendering if needed
  viewport_.ClearDirtyFlag();
  Draw(viewport_was_dirty);

  if (picking_mode_ == PickingMode::kNone) {
    for (const auto& render_callback : render_callbacks_) {
      render_callback();
    }
  }

  glFlush();
  CleanupGlState();

  PostRender();

  double_clicking_ = false;
}

void GlCanvas::PreRender() {
  if (viewport_.IsDirty()) {
    ResetHoverTimer();
  }

  if (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_) {
    SetPickingMode(PickingMode::kHover);
  }
}

void GlCanvas::PostRender() {
  PickingMode prev_picking_mode = picking_mode_;
  picking_mode_ = PickingMode::kNone;

  if (prev_picking_mode == PickingMode::kHover) {
    can_hover_ = false;
  }

  if (prev_picking_mode != PickingMode::kNone) {
    Pick(prev_picking_mode, mouse_move_pos_screen_[0], mouse_move_pos_screen_[1]);
    GlCanvas::Render(viewport_.GetScreenWidth(), viewport_.GetScreenHeight());
  }
}

void GlCanvas::Resize(int width, int height) {
  viewport_.Resize(width, height);
  viewport_.SetVisibleWorldWidth(width);
  viewport_.SetVisibleWorldHeight(height);
}

void GlCanvas::ResetHoverTimer() {
  hover_timer_.Restart();
  can_hover_ = true;
}

void GlCanvas::SetPickingMode(PickingMode mode) {
  if (imgui_context_ != nullptr) return;
  picking_mode_ = mode;
}

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