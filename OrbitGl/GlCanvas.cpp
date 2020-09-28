// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlCanvas.h"

#include <string>

#include "App.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "OpenGl.h"
#include "RingBuffer.h"
#include "SamplingProfiler.h"
#include "TextBox.h"
#include "absl/strings/str_format.h"

RingBuffer<float, 512> GDeltaTimeBuffer;

float GlCanvas::kZValueSlider = 0.03f;
float GlCanvas::kZValueSliderBg = 0.02f;
float GlCanvas::kZValueMargin = 0.01f;
float GlCanvas::kZValueTextUi = 0.005f;
float GlCanvas::kZValueTimeBarBg = 0.004f;
float GlCanvas::kZValueUi = 0.0f;
float GlCanvas::kZValueEventBarPicking = -0.001f;
float GlCanvas::kZValueText = -0.003f;
float GlCanvas::kZValueOverlay = -0.004f;
float GlCanvas::kZValueOverlayBg = -0.005f;
float GlCanvas::kZValueRoundingCorner = -0.01f;
float GlCanvas::kZValueEvent = -0.02f;
float GlCanvas::kZValueBox = -0.03f;
float GlCanvas::kZValueEventBar = -0.1f;
float GlCanvas::kZValueTrack = -0.2f;

const Color GlCanvas::kBackgroundColor = Color(67, 67, 67, 255);
const Color GlCanvas::kTabColor = Color(50, 50, 50, 255);
const Color GlCanvas::kTabTextColorSelected = Color(100, 181, 246, 255);

GlCanvas::GlCanvas() : ui_batcher_(BatcherId::kUi, &picking_manager_) {
  text_renderer_.SetCanvas(this);

  width_ = 0;
  height_ = 0;
  world_width_ = 0;
  world_height_ = 0;
  world_top_left_x_ = -5.f;
  world_top_left_y_ = 5.f;
  world_min_width_ = 1.f;
  select_start_ = Vec2(0.f, 0.f);
  select_stop_ = Vec2(0.f, 0.f);
  time_start_ = 0.0;
  time_stop_ = 0.0;
  is_selecting_ = false;
  picking_ = false;
  double_clicking_ = false;
  control_key_ = false;
  shift_key_ = false;
  alt_key_ = false;
  m_NeedsRedraw = true;

  min_wheel_delta_ = INT_MAX;
  max_wheel_delta_ = INT_MIN;
  wheel_momentum_ = 0.f;
  delta_time_ = 0.0f;
  delta_time_ms_ = 0;
  mouse_ratio_ = 0.0;
  draw_ui_ = true;
  im_gui_active_ = false;

  static int counter = 0;
  id_ = counter++;

  update_timer_.Start();

  // SetCursor(wxCURSOR_BLANK);

  hover_delay_ms_ = 300;
  can_hover_ = false;
  is_hovering_ = false;
  ResetHoverTimer();

  im_gui_context_ = ImGui::CreateContext();
  ScopeImguiContext state(im_gui_context_);
}

GlCanvas::~GlCanvas() {
  ImGui::DestroyContext(im_gui_context_);
  ScopeImguiContext state(im_gui_context_);
}

void GlCanvas::Initialize() {
  static bool first_init = true;
  if (first_init) {
    // glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    CheckGlError();
    if (err != GLEW_OK) {
      /* Problem: glewInit failed, something is seriously wrong. */
      FATAL("Problem: glewInit failed, something is seriously wrong: %s",
            reinterpret_cast<const char*>(glewGetErrorString(err)));
    }
    LOG("Using GLEW %s", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)));
    first_init = false;
  }
}

void GlCanvas::MouseMoved(int x, int y, bool left, bool /*right*/, bool /*middle*/) {
  int mouse_x = x;
  int mouse_y = y;

  float world_x, world_y;
  ScreenToWorld(mouse_x, mouse_y, world_x, world_y);

  mouse_x_ = world_x;
  mouse_y_ = world_y;
  mouse_pos_x_ = mouse_x;
  mouse_pos_y_ = mouse_y;

  // Pan
  if (left && !im_gui_active_) {
    world_top_left_x_ = world_click_x_ - static_cast<float>(mouse_x) / GetWidth() * world_width_;
    world_top_left_y_ = world_click_y_ + static_cast<float>(mouse_y) / GetHeight() * world_height_;
  }

  if (is_selecting_) {
    select_stop_ = Vec2(world_x, world_y);
  }

  ResetHoverTimer();
  NeedsRedraw();
}

void GlCanvas::LeftDown(int x, int y) {
  // Store world clicked pos for panning
  ScreenToWorld(x, y, world_click_x_, world_click_y_);
  screen_click_x_ = x;
  screen_click_y_ = y;
  is_selecting_ = false;

  Orbit_ImGui_MouseButtonCallback(this, 0, true);

  NeedsRedraw();
}

void GlCanvas::MouseWheelMoved(int x, int y, int delta, bool ctrl) {
  // Normalize and invert sign, so that delta < 0 is zoom in.
  int delta_normalized = delta < 0 ? 1 : -1;

  if (delta_normalized < min_wheel_delta_) min_wheel_delta_ = delta_normalized;
  if (delta_normalized > max_wheel_delta_) max_wheel_delta_ = delta_normalized;

  float mouse_x = x;
  float world_x;
  float world_y;

  ScreenToWorld(x, y, world_x, world_y);
  mouse_ratio_ = mouse_x / GetWidth();

  bool zoom_width = !ctrl;
  if (zoom_width) {
    wheel_momentum_ = delta_normalized * wheel_momentum_ < 0
                          ? 0.f
                          : static_cast<float>(wheel_momentum_ + delta_normalized);
  } else {
    // TODO: scale track height.
  }

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(this, -delta_normalized);

  NeedsRedraw();
}

void GlCanvas::LeftUp() {
  picking_manager_.Release();
  Orbit_ImGui_MouseButtonCallback(this, 0, false);
  NeedsRedraw();
}

void GlCanvas::LeftDoubleClick() {
  ScopeImguiContext state(im_gui_context_);
  double_clicking_ = true;
  NeedsRedraw();
}

void GlCanvas::RightDown(int x, int y) {
  float world_x, world_y;
  ScreenToWorld(x, y, world_x, world_y);

  select_start_ = select_stop_ = Vec2(world_x, world_y);
  is_selecting_ = true;

  Orbit_ImGui_MouseButtonCallback(this, 1, true);
  NeedsRedraw();
}

bool GlCanvas::RightUp() {
  Orbit_ImGui_MouseButtonCallback(this, 1, false);
  is_selecting_ = true;
  NeedsRedraw();
  return false;
}

void GlCanvas::MouseLeftWindow() {}

void GlCanvas::CharEvent(unsigned int character) { Orbit_ImGui_CharCallback(this, character); }

void GlCanvas::KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);
  ScopeImguiContext state(im_gui_context_);
  ImGuiIO& io = ImGui::GetIO();
  io.KeyCtrl = ctrl;
  io.KeyShift = shift;
  io.KeyAlt = alt;

  Orbit_ImGui_KeyCallback(this, key_code, true);
  NeedsRedraw();
}

void GlCanvas::KeyReleased(unsigned int key_code, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);
  Orbit_ImGui_KeyCallback(this, key_code, false);
  NeedsRedraw();
}

void GlCanvas::UpdateSpecialKeys(bool ctrl, bool shift, bool alt) {
  control_key_ = ctrl;
  shift_key_ = shift;
  alt_key_ = alt;
}

bool GlCanvas::ControlPressed() { return control_key_; }

bool GlCanvas::ShiftPressed() { return shift_key_; }

bool GlCanvas::AltPressed() { return alt_key_; }

void GlCanvas::UpdateWheelMomentum(float delta_time) {
  float sign = wheel_momentum_ > 0 ? 1.f : -1.f;
  static float inc = 15;
  float new_momentum = wheel_momentum_ - sign * inc * delta_time;
  wheel_momentum_ = new_momentum * wheel_momentum_ > 0.f ? new_momentum : 0.f;
}

void GlCanvas::OnTimer() {
  update_timer_.Stop();
  delta_time_ = static_cast<float>(update_timer_.ElapsedSeconds());
  delta_time_ms_ = update_timer_.ElapsedMillis();
  update_timer_.Start();
  UpdateWheelMomentum(delta_time_);
}

/** Inits the OpenGL viewport for drawing in 2D. */
void GlCanvas::Prepare2DViewport(int top_left_x, int top_left_y, int bottom_right_x,
                                 int bottom_right_y) {
  glViewport(top_left_x, top_left_y, bottom_right_x - top_left_x, bottom_right_y - top_left_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Text renderer
  // mat4_set_orthographic( &m_TextRenderer.GetProjection(), topleft_x,
  // bottomrigth_x, topleft_y, bottomrigth_y, -1, 1);

  world_width_ = width_;
  world_height_ = height_;

  // TRACE_VAR( m_ScreenClickY );
  // TRACE_VAR( GPdbDbg->GetFunctions().size() );
  // TRACE_VAR( GPdbDbg->GetTypes().size() );
  // TRACE_VAR( GPdbDbg->GetGlobals().size() );
  // TRACE_VAR( GPdbDbg->GetLoadTime() );
  // TRACE_VAR( m_WorldTopLeftX );
  // TRACE_VAR( m_WorldTopLeftY );
  // TRACE_VAR( m_WorldWidth );
  // TRACE_VAR( m_WorldHeight );
  // TRACE_VAR( GPdbDbg->GetHModule() );
  // TRACE_VAR( m_MinWheelDelta );
  // TRACE_VAR( m_MaxWheelDelta );
  // TRACE_VAR( m_WheelMomentum );
  // TRACE_VAR( m_DeltaTime );
  // TRACE_VAR( GDeltaTimeBuffer.Size() );
  // TRACE_VAR( GDeltaTimeBuffer[0] );
  // TRACE_VAR( GDeltaTimeBuffer[512] );
  // TRACE_VAR( Capture::GNumContextSwitches );

  if (world_width_ <= 0) world_width_ = 1.f;
  if (world_height_ <= 0) world_height_ = 1.f;

  gluOrtho2D(world_top_left_x_, world_top_left_x_ + world_width_, world_top_left_y_ - world_height_,
             world_top_left_y_);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GlCanvas::PrepareScreenSpaceViewport() {
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

  // glEnable(GL_DEBUG_OUTPUT);
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
  wx = world_top_left_x_ + (static_cast<float>(x) / GetWidth()) * world_width_;
  wy = world_top_left_y_ - (static_cast<float>(y) / GetHeight()) * world_height_;
}

void GlCanvas::WorldToScreen(float wx, float wy, int& x, int& y) const {
  x = static_cast<int>((wx - world_top_left_x_) / world_width_) * GetWidth();

  float bottom_y = world_top_left_y_ - world_height_;
  y = static_cast<int>((1.f - ((wy - bottom_y) / world_height_)) * GetHeight());
}

int GlCanvas::WorldToScreenHeight(float height) const {
  return static_cast<int>((height / world_height_) * GetHeight());
}

float GlCanvas::ScreenToWorldHeight(int height) const {
  return (static_cast<float>(height) / GetHeight()) * world_height_;
}

float GlCanvas::ScreenToWorldWidth(int width) const {
  return (static_cast<float>(width) / GetWidth()) * world_width_;
}

int GlCanvas::GetWidth() const { return width_; }

int GlCanvas::GetHeight() const { return height_; }

void GlCanvas::Render(int width, int height) {
  width_ = width;
  height_ = height;

  if (!m_NeedsRedraw) {
    return;
  }

  m_NeedsRedraw = false;
  ui_batcher_.StartNewFrame();

  ScopeImguiContext state(im_gui_context_);

  Timer timer;
  timer.Start();

  PrepareGlState();
  Prepare2DViewport(0, 0, GetWidth(), GetHeight());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  // Clear text renderer
  text_renderer_.Init();
  text_renderer_.Clear();

  Draw();

  // We have to draw everything collected in the batcher at this point,
  // as PrepareScreenSpaceViewport() changes the coordinate system.
  ui_batcher_.Draw(GetPickingMode() != PickingMode::kNone);
  ui_batcher_.ResetElements();

  PrepareScreenSpaceViewport();

  DrawScreenSpace();

  // Draw remaining elements collected with the batcher.
  ui_batcher_.Draw(GetPickingMode() != PickingMode::kNone);

  text_renderer_.Display(&ui_batcher_);
  RenderText();
  RenderUI();

  glFlush();
  CleanupGlState();

  timer.Stop();

  im_gui_active_ = ImGui::IsAnyItemActive();

  PostRender();

  picking_ = false;
  double_clicking_ = false;
}

void GlCanvas::Resize(int width, int height) {
  width_ = width;
  height_ = height;
  NeedsRedraw();
}

Vec2 GlCanvas::ToScreenSpace(const Vec2& point) {
  float x = (point[0] / world_min_width_) * width_;
  float y = (point[1] / world_height_) * height_;

  return Vec2(x, y);
}

Vec2 GlCanvas::ToWorldSpace(const Vec2& point) {
  float x = (point[0] / width_) * world_min_width_;
  float y = (point[1] / height_) * world_height_;

  return Vec2(x, y);
}

void GlCanvas::ResetHoverTimer() {
  hover_timer_.Reset();
  can_hover_ = true;
}

[[nodiscard]] PickingMode GlCanvas::GetPickingMode() {
  if (picking_ && !is_hovering_) {
    return PickingMode::kClick;
  }
  if (is_hovering_) {
    return PickingMode::kHover;
  }

  return PickingMode::kNone;
}
