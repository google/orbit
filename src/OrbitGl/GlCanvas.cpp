// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlCanvas.h"

#include <GteVector.h>
#include <absl/base/casts.h>
#include <limits.h>
#include <math.h>

#include "App.h"
#include "CaptureWindow.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "IntrospectionWindow.h"
#include "OpenGl.h"
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

float GlCanvas::kZOffsetMovingTack = 0.1f;
float GlCanvas::kZOffsetPinnedTrack = 0.2f;

constexpr unsigned kNumberOriginalLayers = 17;
constexpr unsigned kExtraLayersForMovingTracks = 6;
constexpr unsigned kExtraLayersForPinnedTracks = 6;
constexpr unsigned kExtraLayersForSliderEpsilons = 4;
unsigned GlCanvas::kMaxNumberRealZLayers = kNumberOriginalLayers + kExtraLayersForMovingTracks +
                                           kExtraLayersForPinnedTracks +
                                           kExtraLayersForSliderEpsilons;

const Color GlCanvas::kBackgroundColor = Color(67, 67, 67, 255);
const Color GlCanvas::kTabTextColorSelected = Color(100, 181, 246, 255);

GlCanvas::GlCanvas() : ui_batcher_(BatcherId::kUi, &picking_manager_) {
  text_renderer_.SetCanvas(this);

  screen_width_ = 0;
  screen_height_ = 0;
  world_width_ = 0;
  world_height_ = 0;
  world_top_left_x_ = -5.f;
  world_top_left_y_ = 5.f;
  world_min_width_ = 1.f;
  select_start_ = Vec2(0.f, 0.f);
  select_stop_ = Vec2(0.f, 0.f);
  is_selecting_ = false;
  picking_ = false;
  double_clicking_ = false;
  control_key_ = false;
  redraw_requested_ = true;

  mouse_screen_x_ = 0.0;
  mouse_screen_y_ = 0.0;
  mouse_world_x_ = 0.0;
  mouse_world_y_ = 0.0;
  world_click_x_ = 0.0;
  world_click_y_ = 0.0;
  world_max_y_ = 0.0;
  screen_click_x_ = 0;
  screen_click_y_ = 0;
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

void GlCanvas::Initialize() {
  static bool first_init = true;
  if (first_init) {
    GLenum err = glewInit();
    CheckGlError();
    if (err != GLEW_OK) {
      /* Problem: glewInit failed, something is seriously wrong. */
      FATAL("Problem: glewInit failed, something is seriously wrong: %s",
            absl::bit_cast<const char*>(glewGetErrorString(err)));
    }
    if (!glewIsSupported("GL_VERSION_2_0")) {
      FATAL("Problem: OpenGL version supported by GLEW must be at least 2.0!");
    }
    LOG("Using GLEW %s", absl::bit_cast<const char*>(glewGetString(GLEW_VERSION)));
    first_init = false;
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
         (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_);
}

void GlCanvas::MouseMoved(int x, int y, bool left, bool /*right*/, bool /*middle*/) {
  int mouse_x = x;
  int mouse_y = y;

  float world_x, world_y;
  ScreenToWorld(mouse_x, mouse_y, world_x, world_y);

  mouse_world_x_ = world_x;
  mouse_world_y_ = world_y;
  mouse_screen_x_ = mouse_x;
  mouse_screen_y_ = mouse_y;

  // Pan
  if (left && !picking_manager_.IsDragging()) {
    UpdateWorldTopLeftX(world_click_x_ - static_cast<float>(mouse_x) /
                                             static_cast<float>(GetWidth()) * world_width_);
    UpdateWorldTopLeftY(world_top_left_y_ = world_click_y_ + static_cast<float>(mouse_y) /
                                                                 static_cast<float>(GetHeight()) *
                                                                 world_height_);
  }

  if (left) {
    picking_manager_.Drag(x, y);
  }

  if (is_selecting_) {
    select_stop_ = Vec2(world_x, world_y);
  }

  ResetHoverTimer();
  RequestRedraw();
}

void GlCanvas::LeftDown(int x, int y) {
  // Store world clicked pos for panning
  ScreenToWorld(x, y, world_click_x_, world_click_y_);

  screen_click_x_ = x;
  screen_click_y_ = y;
  picking_ = true;
  is_selecting_ = false;

  Orbit_ImGui_MouseButtonCallback(imgui_context_, 0, true);

  RequestRedraw();
}

void GlCanvas::MouseWheelMoved(int /*x*/, int /*y*/, int delta, bool /*ctrl*/) {
  // Normalize and invert sign, so that delta < 0 is zoom in.
  int delta_normalized = delta < 0 ? 1 : -1;

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(imgui_context_, -delta_normalized);

  can_hover_ = true;

  RequestRedraw();
}

void GlCanvas::LeftUp() {
  picking_manager_.Release();
  Orbit_ImGui_MouseButtonCallback(imgui_context_, 0, false);
  RequestRedraw();
}

void GlCanvas::LeftDoubleClick() {
  double_clicking_ = true;
  picking_ = true;
  RequestRedraw();
}

void GlCanvas::RightDown(int x, int y) {
  ScreenToWorld(x, y, world_click_x_, world_click_y_);
  screen_click_x_ = x;
  screen_click_y_ = y;

  select_start_ = select_stop_ = Vec2(world_click_x_, world_click_y_);
  is_selecting_ = true;
  picking_ = true;

  Orbit_ImGui_MouseButtonCallback(imgui_context_, 1, true);
  RequestRedraw();
}

bool GlCanvas::RightUp() {
  Orbit_ImGui_MouseButtonCallback(imgui_context_, 1, false);
  is_selecting_ = false;
  select_start_ = select_stop_;
  RequestRedraw();

  bool show_context_menu = select_start_ == select_stop_;
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
void GlCanvas::Prepare2DViewport(int top_left_x, int top_left_y, int bottom_right_x,
                                 int bottom_right_y) {
  glViewport(top_left_x, top_left_y, bottom_right_x - top_left_x, bottom_right_y - top_left_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  world_width_ = static_cast<float>(screen_width_);
  world_height_ = static_cast<float>(screen_height_);

  if (world_width_ <= 0) world_width_ = 1.f;
  if (world_height_ <= 0) world_height_ = 1.f;

  glOrtho(world_top_left_x_, world_top_left_x_ + world_width_, world_top_left_y_ - world_height_,
          world_top_left_y_, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GlCanvas::PrepareScreenSpaceViewport() {
  ORBIT_SCOPE_FUNCTION;
  glViewport(0, 0, GetWidth(), GetHeight());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, GetWidth(), 0, GetHeight(), -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GlCanvas::PrepareGlState() {
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

  glClearColor(static_cast<float>(kBackgroundColor[0]) / 255.0f,
               static_cast<float>(kBackgroundColor[1]) / 255.0f,
               static_cast<float>(kBackgroundColor[2]) / 255.0f,
               static_cast<float>(kBackgroundColor[3]) / 255.0f);
  if (picking_) glClearColor(0.f, 0.f, 0.f, 0.f);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  picking_ ? glDisable(GL_BLEND) : glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);  // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);   // The Type Of Depth Testing To Do
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void GlCanvas::CleanupGlState() { glPopAttrib(); }

void GlCanvas::ScreenToWorld(int x, int y, float& wx, float& wy) const {
  wx = world_top_left_x_ + (static_cast<float>(x) / static_cast<float>(GetWidth())) * world_width_;
  wy =
      world_top_left_y_ - (static_cast<float>(y) / static_cast<float>(GetHeight())) * world_height_;
}

Vec2 GlCanvas::ScreenToWorld(Vec2 screen_pos) const {
  Vec2 world_pos;
  world_pos[0] = world_top_left_x_ + screen_pos[0] / static_cast<float>(GetWidth()) * world_width_;
  world_pos[1] =
      world_top_left_y_ - screen_pos[1] / static_cast<float>(GetHeight()) * world_height_;
  return world_pos;
}

float GlCanvas::ScreenToWorldHeight(int height) const {
  return (static_cast<float>(height) / static_cast<float>(GetHeight())) * world_height_;
}

float GlCanvas::ScreenToWorldWidth(int width) const {
  return (static_cast<float>(width) / static_cast<float>(GetWidth())) * world_width_;
}

Vec2 GlCanvas::WorldToScreen(Vec2 world_pos) const {
  Vec2 screen_pos;
  screen_pos[0] = floorf((world_pos[0] - world_top_left_x_) / world_width_ * GetWidth());
  screen_pos[1] = floorf((world_top_left_y_ - world_pos[1]) / world_height_ * GetHeight());
  return screen_pos;
}

// TODO (b/177350599): Unify QtScreen and GlScreen
// QtScreen(x,y) --> GlScreen(x,height-y)
Vec2 GlCanvas::QtScreenToGlScreen(Vec2 qt_pos) const {
  Vec2 gl_pos = qt_pos;
  gl_pos[1] = GetHeight() - qt_pos[1];
  return gl_pos;
}

int GlCanvas::WorldToScreenHeight(float height) const {
  return static_cast<int>(height / world_height_ * GetHeight());
}

int GlCanvas::WorldToScreenWidth(float width) const {
  return static_cast<int>(width / world_width_ * GetWidth());
}

int GlCanvas::GetWidth() const { return screen_width_; }

int GlCanvas::GetHeight() const { return screen_height_; }

void GlCanvas::Render(int width, int height) {
  ORBIT_SCOPE("GlCanvas::Render");
  screen_width_ = width;
  screen_height_ = height;

  if (!IsRedrawNeeded()) {
    return;
  }

  redraw_requested_ = false;
  ui_batcher_.StartNewFrame();

  PrepareGlState();
  Prepare2DViewport(0, 0, GetWidth(), GetHeight());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  // Clear text renderer
  text_renderer_.Init();
  text_renderer_.Clear();

  // Reset picking manager before each draw.
  picking_manager_.Reset();

  Draw();

  if (GetPickingMode() == PickingMode::kNone) {
    for (const auto& render_callback : render_callbacks_) {
      render_callback();
    }
  }

  glFlush();
  CleanupGlState();

  PostRender();

  picking_ = false;
  double_clicking_ = false;
}

void GlCanvas::PreRender() {
  if (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_) {
    is_hovering_ = true;
    picking_ = true;
  }
}

void GlCanvas::PostRender() {
  PickingMode picking_mode = GetPickingMode();

  picking_ = false;

  if (picking_mode == PickingMode::kHover) {
    ResetHoverTimer();
  }

  if (picking_mode != PickingMode::kNone) {
    Pick(picking_mode, screen_click_x_, screen_click_y_);
    GlCanvas::Render(screen_width_, screen_height_);
  }
}

void GlCanvas::Resize(int width, int height) {
  screen_width_ = width;
  screen_height_ = height;
  RequestRedraw();
}

void GlCanvas::ResetHoverTimer() {
  hover_timer_.Restart();
  is_hovering_ = false;
  can_hover_ = true;
}

PickingMode GlCanvas::GetPickingMode() {
  if (picking_ && !is_hovering_) {
    return PickingMode::kClick;
  }
  if (is_hovering_) {
    return PickingMode::kHover;
  }

  return PickingMode::kNone;
}

void GlCanvas::Pick(PickingMode picking_mode, int x, int y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  std::array<uint8_t, 4 * 1 * 1> pixels{};
  glReadPixels(x, screen_height_ - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
  uint32_t value;
  std::memcpy(&value, &pixels[0], sizeof(uint32_t));
  PickingId pick_id = PickingId::FromPixelValue(value);

  HandlePickedElement(picking_mode, pick_id, x, y);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> GlCanvas::CreateAccessibleInterface() {
  return std::make_unique<orbit_accessibility::AccessibleWidgetBridge>();
}

orbit_accessibility::AccessibleInterface* GlCanvas::GetOrCreateAccessibleInterface() {
  if (accessibility_ == nullptr) {
    accessibility_ = CreateAccessibleInterface();
  }
  return accessibility_.get();
}
