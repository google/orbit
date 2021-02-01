// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureWindow.h"

#include <GteVector.h>
#include <absl/container/flat_hash_map.h>
#include <absl/time/time.h>
#include <imgui.h>
#include <string.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iterator>
#include <ostream>
#include <string_view>

#include "AccessibleTimeGraph.h"
#include "App.h"
#include "CoreMath.h"
#include "CoreUtils.h"
#include "Geometry.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "OpenGl.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/Tracing.h"
#include "OrbitClientData/CallstackData.h"
#include "OrbitClientModel/CaptureData.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "Timer.h"
#include "TrackManager.h"
#include "absl/base/casts.h"
#include "capture_data.pb.h"

using orbit_accessibility::AccessibleInterface;
using orbit_accessibility::AccessibleWidgetBridge;

class AccessibleCaptureWindow : public AccessibleWidgetBridge {
 public:
  explicit AccessibleCaptureWindow(CaptureWindow* window) : window_(window) {}

  [[nodiscard]] int AccessibleChildCount() const override {
    if (window_->GetTimeGraph() == nullptr) {
      return 0;
    }
    return 1;
  }

  [[nodiscard]] const AccessibleInterface* AccessibleChild(int /*index*/) const override {
    if (window_->GetTimeGraph() == nullptr) {
      return nullptr;
    }
    return window_->GetTimeGraph()->GetOrCreateAccessibleInterface();
  }

 private:
  CaptureWindow* window_;
};

using orbit_client_protos::TimerInfo;

CaptureWindow::CaptureWindow(OrbitApp* app) : GlCanvas(), app_{app} {
  draw_help_ = true;
  draw_filter_ = false;
  world_top_left_x_ = 0;
  world_top_left_y_ = 0;
  world_max_y_ = 0;

  slider_ = std::make_shared<GlHorizontalSlider>();
  vertical_slider_ = std::make_shared<GlVerticalSlider>();

  slider_->SetDragCallback([&](float ratio) {
    this->UpdateHorizontalScroll(ratio);
    NeedsUpdate();
  });
  slider_->SetResizeCallback([&](float normalized_start, float normalized_end) {
    this->UpdateHorizontalZoom(normalized_start, normalized_end);
    NeedsUpdate();
  });
  slider_->SetCanvas(this);

  vertical_slider_->SetDragCallback([&](float ratio) {
    this->UpdateVerticalScroll(ratio);
    NeedsUpdate();
  });
  vertical_slider_->SetCanvas(this);

  vertical_slider_->SetOrthogonalSliderPixelHeight(slider_->GetPixelHeight());
  slider_->SetOrthogonalSliderPixelHeight(vertical_slider_->GetPixelHeight());
}

void CaptureWindow::OnTimer() { GlCanvas::OnTimer(); }

void CaptureWindow::ZoomAll() {
  ResetHoverTimer();
  NeedsUpdate();
  if (time_graph_ == nullptr) return;
  time_graph_->ZoomAll();
}

void CaptureWindow::UpdateWheelMomentum(float delta_time) {
  GlCanvas::UpdateWheelMomentum(delta_time);

  if (time_graph_ == nullptr) return;
  bool zoom_width = true;  // TODO: !wxGetKeyState(WXK_CONTROL);
  if (zoom_width && wheel_momentum_ != 0.f) {
    time_graph_->ZoomTime(wheel_momentum_, mouse_ratio_);
  }
}

void CaptureWindow::MouseMoved(int x, int y, bool left, bool /*right*/, bool /*middle*/) {
  if (time_graph_ == nullptr) return;
  // TODO: Reduce code duplication, call super!
  float world_x, world_y;
  ScreenToWorld(x, y, world_x, world_y);

  mouse_world_x_ = world_x;
  mouse_world_y_ = world_y;
  mouse_screen_x_ = x;
  mouse_screen_y_ = y;

  // Pan
  if (left && !im_gui_active_ && !picking_manager_.IsDragging() && !app_->IsCapturing()) {
    float world_min;
    float world_max;

    time_graph_->GetWorldMinMax(world_min, world_max);

    world_top_left_x_ =
        world_click_x_ - static_cast<float>(x) / static_cast<float>(GetWidth()) * world_width_;
    world_top_left_y_ =
        world_click_y_ + static_cast<float>(y) / static_cast<float>(GetHeight()) * world_height_;

    world_top_left_x_ = clamp(world_top_left_x_, world_min, world_max - world_width_);
    world_top_left_y_ =
        clamp(world_top_left_y_,
              world_height_ - time_graph_->GetTrackManager()->GetTracksTotalHeight(), world_max_y_);

    time_graph_->PanTime(screen_click_x_, x, GetWidth(), ref_time_click_);
    NeedsUpdate();

    click_was_drag_ = true;
  }

  if (is_selecting_) {
    ScreenToWorld(std::max(x, 0), std::max(y, 0), world_x, world_y);
    select_stop_ = Vec2(world_x, world_y);
    time_stop_ = time_graph_->GetTickFromWorld(world_x);
  }

  if (left) {
    picking_manager_.Drag(x, y);
  }

  ResetHoverTimer();

  NeedsRedraw();
}

void CaptureWindow::LeftDown(int x, int y) {
  if (time_graph_ == nullptr) return;
  // Store world clicked pos for panning
  ScreenToWorld(x, y, world_click_x_, world_click_y_);
  screen_click_x_ = x;
  screen_click_y_ = y;
  ref_time_click_ = time_graph_->GetTime(static_cast<double>(x) / GetWidth());

  is_selecting_ = false;

  picking_ = true;
  click_was_drag_ = false;
  NeedsRedraw();
}

void CaptureWindow::LeftUp() {
  GlCanvas::LeftUp();

  if (!click_was_drag_ && background_clicked_) {
    app_->SelectTextBox(nullptr);
    app_->set_selected_thread_id(orbit_base::kAllProcessThreadsTid);
    NeedsUpdate();
  }
}

void CaptureWindow::LeftDoubleClick() {
  GlCanvas::LeftDoubleClick();
  double_clicking_ = true;
  picking_ = true;
}

void CaptureWindow::Pick() {
  picking_ = true;
  NeedsRedraw();
}

void CaptureWindow::Pick(int x, int y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  std::array<uint8_t, 4 * 1 * 1> pixels{};
  glReadPixels(x, m_MainWindowHeight - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
  uint32_t value;
  std::memcpy(&value, &pixels[0], sizeof(uint32_t));
  PickingId pick_id = PickingId::FromPixelValue(value);

  Pick(pick_id, x, y);

  NeedsUpdate();
}

void CaptureWindow::Pick(PickingId picking_id, int x, int y) {
  if (time_graph_ == nullptr) return;
  PickingType type = picking_id.type;
  background_clicked_ = false;

  Batcher& batcher = GetBatcherById(picking_id.batcher_id);
  const TextBox* text_box = batcher.GetTextBox(picking_id);
  if (text_box) {
    SelectTextBox(text_box);
  } else if (type == PickingType::kPickable) {
    picking_manager_.Pick(picking_id, x, y);
  } else {
    // If the background is clicked: The selection should only be cleared
    // if the user doesn't drag around the capture window.
    // This is handled later in CaptureWindow::LeftUp()
    background_clicked_ = true;
  }
}

void CaptureWindow::SelectTextBox(const TextBox* text_box) {
  CHECK(time_graph_ != nullptr);
  if (text_box == nullptr) return;

  app_->SelectTextBox(text_box);
  app_->set_selected_thread_id(text_box->GetTimerInfo().thread_id());

  const TimerInfo& timer_info = text_box->GetTimerInfo();

  if (double_clicking_) {
    time_graph_->Zoom(timer_info);
  }
}

void CaptureWindow::Hover(int x, int y) {
  if (time_graph_ == nullptr) return;
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  uint8_t pixels[1 * 1 * 4] = {0, 0, 0, 0};
  glReadPixels(x, m_MainWindowHeight - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

  auto pick_id = PickingId::FromPixelValue(absl::bit_cast<uint32_t>(pixels));
  Batcher& batcher = GetBatcherById(pick_id.batcher_id);

  std::string tooltip;

  if (pick_id.type == PickingType::kPickable) {
    auto pickable = GetPickingManager().GetPickableFromId(pick_id);
    if (pickable) {
      tooltip = pickable->GetTooltip();
    }
  } else {
    PickingUserData* user_data = batcher.GetUserData(pick_id);

    if (user_data && user_data->generate_tooltip_) {
      tooltip = user_data->generate_tooltip_(pick_id);
    }
  }

  app_->SendTooltipToUi(tooltip);
}

void CaptureWindow::PreRender() {
  // TODO: Move to GlCanvas?
  if (is_mouse_over_ && can_hover_ && hover_timer_.ElapsedMillis() > hover_delay_ms_) {
    is_hovering_ = true;
    picking_ = true;
    NeedsRedraw();
  }
  m_NeedsRedraw = m_NeedsRedraw || (time_graph_ != nullptr && time_graph_->IsRedrawNeeded());
}

void CaptureWindow::PostRender() {
  if (is_hovering_) {
    is_hovering_ = false;
    can_hover_ = false;
    picking_ = false;
    hover_timer_.Restart();

    Hover(mouse_screen_x_, mouse_screen_y_);
    NeedsUpdate();
    GlCanvas::Render(screen_width_, screen_height_);
    hover_timer_.Restart();
  }

  if (picking_) {
    picking_ = false;
    Pick(screen_click_x_, screen_click_y_);
    NeedsRedraw();
    GlCanvas::Render(screen_width_, screen_height_);
  }
}

void CaptureWindow::Resize(int width, int height) {
  GlCanvas::Resize(width, height);
  NeedsUpdate();
}

void CaptureWindow::RightDown(int x, int y) {
  if (time_graph_ == nullptr) return;
  ScreenToWorld(x, y, world_click_x_, world_click_y_);
  screen_click_x_ = x;
  screen_click_y_ = y;
  Pick();

  is_selecting_ = true;
  select_start_ = Vec2(world_click_x_, world_click_y_);
  select_stop_ = select_start_;
  time_start_ = time_graph_->GetTickFromWorld(world_click_x_);
  time_stop_ = time_start_;
}

bool CaptureWindow::RightUp() {
  if (time_graph_ == nullptr) return false;
  if (is_selecting_ && (select_start_[0] != select_stop_[0]) && ControlPressed()) {
    float min_world = std::min(select_stop_[0], select_start_[0]);
    float max_world = std::max(select_stop_[0], select_start_[0]);

    double new_min = time_graph_->GetTime((min_world - world_top_left_x_) / world_width_);
    double new_max = time_graph_->GetTime((max_world - world_top_left_x_) / world_width_);

    time_graph_->SetMinMax(new_min, new_max);
    select_start_ = select_stop_;
  }

  bool show_context_menu = select_start_[0] == select_stop_[0];
  is_selecting_ = false;
  NeedsRedraw();
  return show_context_menu;
}

void CaptureWindow::MiddleDown(int x, int y) {
  float world_x, world_y;
  ScreenToWorld(x, y, world_x, world_y);
  is_selecting_ = true;
  select_start_ = Vec2(world_x, world_y);
  select_stop_ = select_start_;
}

void CaptureWindow::MiddleUp(int x, int y) {
  float world_x, world_y;
  ScreenToWorld(x, y, world_x, world_y);
  is_selecting_ = false;

  select_stop_ = Vec2(world_x, world_y);

  NeedsRedraw();
}

void CaptureWindow::Zoom(int delta) {
  if (delta == 0) return;

  delta = -delta;
  auto delta_float = static_cast<float>(delta);

  float world_x;
  float world_y;

  ScreenToWorld(mouse_screen_x_, mouse_screen_y_, world_x, world_y);
  mouse_ratio_ = static_cast<double>(mouse_screen_x_) / GetWidth();

  if (time_graph_ != nullptr) {
    time_graph_->ZoomTime(delta_float, mouse_ratio_);
  }
  wheel_momentum_ = delta_float * wheel_momentum_ < 0 ? 0 : wheel_momentum_ + delta_float;

  NeedsUpdate();
}

void CaptureWindow::Pan(float ratio) {
  if (time_graph_ == nullptr) return;
  double ref_time = time_graph_->GetTime(static_cast<double>(mouse_screen_x_) / GetWidth());
  time_graph_->PanTime(mouse_screen_x_,
                       mouse_screen_x_ + static_cast<int>(ratio * static_cast<float>(GetWidth())),
                       GetWidth(), ref_time);
  NeedsUpdate();
}

void CaptureWindow::MouseWheelMoved(int x, int y, int delta, bool ctrl) {
  if (delta == 0) return;

  // Normalize and invert sign, so that delta < 0 is zoom in.
  const int delta_normalized = delta < 0 ? 1 : -1;
  const auto delta_float = static_cast<float>(delta_normalized);

  if (delta < min_wheel_delta_) min_wheel_delta_ = delta;
  if (delta > max_wheel_delta_) max_wheel_delta_ = delta;

  float worldx;
  float worldy;

  ScreenToWorld(x, y, worldx, worldy);

  bool zoom_width = !ctrl;
  if (zoom_width) {
    mouse_ratio_ = static_cast<double>(x) / GetWidth();

    if (time_graph_ != nullptr) {
      time_graph_->ZoomTime(delta_float, mouse_ratio_);
    }
    wheel_momentum_ = delta_float * wheel_momentum_ < 0 ? 0 : wheel_momentum_ + delta_float;
  } else {
    float mouse_relative_y_position = static_cast<float>(y) / static_cast<float>(GetHeight());
    if (time_graph_ != nullptr) {
      time_graph_->VerticalZoom(delta_float, mouse_relative_y_position);
    }
  }

  can_hover_ = true;

  NeedsUpdate();
}

void CaptureWindow::MouseWheelMovedHorizontally(int /*x*/, int /*y*/, int delta, bool /*ctrl*/) {
  if (delta == 0) return;

  // Normalize and invert sign, so that delta < 0 is left.
  int delta_normalized = delta < 0 ? 1 : -1;

  if (delta_normalized < 0) {
    Pan(0.1f);
  } else {
    Pan(-0.1f);
  }
}

void CaptureWindow::KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) {
  UpdateSpecialKeys(ctrl, shift, alt);

  if (!im_gui_active_) {
    switch (key_code) {
      case ' ':
        if (!shift) {
          ZoomAll();
        }
        break;
      case 'A':
        Pan(0.1f);
        break;
      case 'D':
        Pan(-0.1f);
        break;
      case 'W':
        Zoom(1);
        break;
      case 'S':
        Zoom(-1);
        break;
      case 'F':
        draw_filter_ = !draw_filter_;
        break;
      case 'H':
        draw_help_ = !draw_help_;
        break;
      case 'X':
        ToggleRecording();
        break;
      case 18:  // Left
        if (time_graph_ == nullptr) return;
        if (shift) {
          time_graph_->JumpToNeighborBox(app_->selected_text_box(),
                                         TimeGraph::JumpDirection::kPrevious,
                                         TimeGraph::JumpScope::kSameFunction);
        } else if (alt) {
          time_graph_->JumpToNeighborBox(app_->selected_text_box(),
                                         TimeGraph::JumpDirection::kPrevious,
                                         TimeGraph::JumpScope::kSameThreadSameFunction);
        } else {
          time_graph_->JumpToNeighborBox(app_->selected_text_box(),
                                         TimeGraph::JumpDirection::kPrevious,
                                         TimeGraph::JumpScope::kSameDepth);
        }
        break;
      case 20:  // Right
        if (time_graph_ == nullptr) return;
        if (shift) {
          time_graph_->JumpToNeighborBox(app_->selected_text_box(), TimeGraph::JumpDirection::kNext,
                                         TimeGraph::JumpScope::kSameFunction);
        } else if (alt) {
          time_graph_->JumpToNeighborBox(app_->selected_text_box(), TimeGraph::JumpDirection::kNext,
                                         TimeGraph::JumpScope::kSameThreadSameFunction);
        } else {
          time_graph_->JumpToNeighborBox(app_->selected_text_box(), TimeGraph::JumpDirection::kNext,
                                         TimeGraph::JumpScope::kSameDepth);
        }
        break;
      case 19:  // Up

        if (time_graph_ == nullptr) return;
        time_graph_->JumpToNeighborBox(app_->selected_text_box(), TimeGraph::JumpDirection::kTop,
                                       TimeGraph::JumpScope::kSameThread);
        break;
      case 21:  // Down
        if (time_graph_ == nullptr) return;
        time_graph_->JumpToNeighborBox(app_->selected_text_box(), TimeGraph::JumpDirection::kDown,
                                       TimeGraph::JumpScope::kSameThread);
        break;
    }
  }

  NeedsRedraw();
}

std::vector<std::string> CaptureWindow::GetContextMenu() { return std::vector<std::string>{}; }

void CaptureWindow::OnContextMenu(const std::string& /*action*/, int /*menu_index*/) {}

bool CaptureWindow::ShouldAutoZoom() const { return app_->IsCapturing(); }

std::unique_ptr<AccessibleWidgetBridge> CaptureWindow::CreateAccessibilityInterface() {
  return std::make_unique<AccessibleCaptureWindow>(this);
}

void CaptureWindow::Draw() {
  ORBIT_SCOPE("CaptureWindow::Draw");
  if (ShouldAutoZoom()) {
    ZoomAll();
  }

  // Reset picking manager before each draw.
  picking_manager_.Reset();

  if (time_graph_ != nullptr) {
    time_graph_->Draw(this, GetPickingMode());
  }

  RenderSelectionOverlay();

  if (GetPickingMode() == PickingMode::kNone) {
    RenderTimeBar();

    Vec2 pos(mouse_world_x_, world_top_left_y_);
    // Vertical green line at mouse x position
    ui_batcher_.AddVerticalLine(pos, -world_height_, kZValueUi, Color(0, 255, 0, 127));

    if (draw_help_) {
      RenderHelpUi();
    }
  }

  DrawScreenSpace();

  if (GetPickingMode() == PickingMode::kNone) {
    text_renderer_.RenderDebug(&ui_batcher_);
  }

  // Get all layers.
  std::vector<float> all_layers{};
  if (time_graph_ != nullptr) {
    all_layers = time_graph_->GetBatcher().GetLayers();
    Append(all_layers, time_graph_->GetTextRenderer()->GetLayers());
  }
  Append(all_layers, ui_batcher_.GetLayers());
  Append(all_layers, text_renderer_.GetLayers());

  // Sort and remove duplicates.
  std::sort(all_layers.begin(), all_layers.end());
  auto it = std::unique(all_layers.begin(), all_layers.end());
  all_layers.resize(std::distance(all_layers.begin(), it));
  if (all_layers.size() > GlCanvas::kMaxNumberRealZLayers) {
    ERROR("Too many z-layers. The current number is %d", all_layers.size());
  }

  for (float layer : all_layers) {
    // We use different coordinate systems for ScreenSpace items (margin, scrollbar, ...)
    // and for the text than the rest of items inside CaptureWindow. So, we have to switch
    // between these 2 systems while the layer is changing (with these "Prepare.." functions).
    if (layer < GlCanvas::kScreenSpaceCutPoint) {
      Prepare2DViewport(0, 0, GetWidth(), GetHeight());
    }
    if (time_graph_ != nullptr) {
      time_graph_->GetBatcher().DrawLayer(layer, GetPickingMode() != PickingMode::kNone);
    }
    ui_batcher_.DrawLayer(layer, GetPickingMode() != PickingMode::kNone);

    PrepareScreenSpaceViewport();
    // Text needs to be drawn in screen space.
    if (GetPickingMode() == PickingMode::kNone) {
      text_renderer_.RenderLayer(&ui_batcher_, layer);
      RenderText(layer);
    }
  }
}

void CaptureWindow::DrawScreenSpace() {
  ORBIT_SCOPE("CaptureWindow::DrawScreenSpace");
  if (time_graph_ == nullptr) return;
  double time_span = time_graph_->GetCaptureTimeSpanUs();

  Color col = slider_->GetBarColor();
  auto canvas_height = static_cast<float>(GetHeight());

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float right_margin = layout.GetRightMargin();

  if (time_span > 0) {
    UpdateHorizontalSliderFromWorld();
    slider_->Draw(this);

    UpdateVerticalSliderFromWorld();
    if (vertical_slider_->GetLengthRatio() < 1.f) {
      vertical_slider_->Draw(this);
      int slider_width = static_cast<int>(time_graph_->GetLayout().GetSliderWidth());
      right_margin += slider_width;
    }
  }

  // Right vertical margin.
  time_graph_->UpdateRightMargin(right_margin);
  auto margin_x1 = static_cast<float>(GetWidth());
  float margin_x0 = margin_x1 - right_margin;

  Box box(Vec2(margin_x0, 0), Vec2(margin_x1 - margin_x0, canvas_height), GlCanvas::kZValueMargin);
  ui_batcher_.AddBox(box, kBackgroundColor);

  // Time bar background
  if (time_graph_->GetCaptureTimeSpanUs() > 0) {
    Box background_box(Vec2(0, time_graph_->GetLayout().GetSliderWidth()),
                       Vec2(GetWidth(), time_graph_->GetLayout().GetTimeBarHeight()),
                       GlCanvas::kZValueTimeBarBg);
    ui_batcher_.AddBox(background_box,
                       Color(kBackgroundColor[0], kBackgroundColor[1], kBackgroundColor[2], 200));
  }
}

void CaptureWindow::UpdateHorizontalScroll(float ratio) {
  if (time_graph_ == nullptr) return;
  time_graph_->UpdateHorizontalScroll(ratio);
}

void CaptureWindow::UpdateVerticalScroll(float ratio) {
  if (time_graph_ == nullptr) return;
  float min = world_max_y_;
  float max = world_height_ - time_graph_->GetTrackManager()->GetTracksTotalHeight();
  float range = max - min;
  float new_top_left_y = min + ratio * range;
  if (new_top_left_y != world_top_left_y_) {
    world_top_left_y_ = new_top_left_y;
    NeedsUpdate();
  }
}

void CaptureWindow::UpdateHorizontalZoom(float normalized_start, float normalized_end) {
  if (time_graph_ == nullptr) return;
  double time_span = time_graph_->GetCaptureTimeSpanUs();
  time_graph_->SetMinMax(normalized_start * time_span, normalized_end * time_span);
}

void CaptureWindow::UpdateHorizontalSliderFromWorld() {
  if (time_graph_ == nullptr) return;
  double time_span = time_graph_->GetCaptureTimeSpanUs();
  double start = time_graph_->GetMinTimeUs();
  double stop = time_graph_->GetMaxTimeUs();
  double width = stop - start;
  double max_start = time_span - width;
  double ratio = app_->IsCapturing() ? 1 : (max_start != 0 ? start / max_start : 0);
  int slider_width = static_cast<int>(time_graph_->GetLayout().GetSliderWidth());
  slider_->SetPixelHeight(slider_width);
  slider_->SetNormalizedPosition(static_cast<float>(ratio));
  slider_->SetNormalizedLength(static_cast<float>(width / time_span));
  slider_->SetOrthogonalSliderPixelHeight(vertical_slider_->GetPixelHeight());
}

void CaptureWindow::UpdateVerticalSliderFromWorld() {
  if (time_graph_ == nullptr) return;
  float min = world_max_y_;
  float max = world_height_ - time_graph_->GetTrackManager()->GetTracksTotalHeight();
  float ratio = (world_top_left_y_ - min) / (max - min);
  float vertical_ratio = world_height_ / time_graph_->GetTrackManager()->GetTracksTotalHeight();
  int slider_width = static_cast<int>(time_graph_->GetLayout().GetSliderWidth());
  vertical_slider_->SetPixelHeight(slider_width);
  vertical_slider_->SetNormalizedPosition(ratio);
  vertical_slider_->SetNormalizedLength(vertical_ratio);
  vertical_slider_->SetOrthogonalSliderPixelHeight(slider_->GetPixelHeight());
}

void CaptureWindow::UpdateWorldTopLeftY(float val) {
  if (time_graph_ == nullptr) return;
  float min_world_top_left =
      GetWorldHeight() - time_graph_->GetTrackManager()->GetTracksTotalHeight();
  GlCanvas::UpdateWorldTopLeftY(clamp(val, min_world_top_left, GetWorldMaxY()));
  NeedsUpdate();
}

void CaptureWindow::ToggleRecording() {
  app_->ToggleCapture();
  draw_help_ = false;
#ifdef __linux__
  ZoomAll();
#endif
}

void CaptureWindow::ToggleDrawHelp() { set_draw_help(!draw_help_); }

void CaptureWindow::set_draw_help(bool draw_help) {
  draw_help_ = draw_help;
  NeedsRedraw();
}

void CaptureWindow::CreateTimeGraph(const CaptureData* capture_data) {
  time_graph_ = std::make_unique<TimeGraph>(app_, &text_renderer_, this, capture_data);
}

Batcher& CaptureWindow::GetBatcherById(BatcherId batcher_id) {
  switch (batcher_id) {
    case BatcherId::kTimeGraph:
      CHECK(time_graph_ != nullptr);
      return time_graph_->GetBatcher();
    case BatcherId::kUi:
      return ui_batcher_;
    default:
      UNREACHABLE();
  }
}

void CaptureWindow::NeedsUpdate() {
  m_NeedsRedraw = true;
  if (time_graph_ == nullptr) return;
  time_graph_->NeedsUpdate();
}

template <class T>
static std::string VariableToString(std::string_view name, const T& value) {
  std::stringstream string_stream{};
  string_stream << name << " = " << value;
  return string_stream.str();
}

void CaptureWindow::RenderImGuiDebugUI() {
  if (ImGui::CollapsingHeader("Layout Properties")) {
    if (time_graph_ != nullptr && time_graph_->GetLayout().DrawProperties()) {
      NeedsUpdate();
    }

    static bool draw_text_outline = false;
    if (ImGui::Checkbox("Draw Text Outline", &draw_text_outline)) {
      TextRenderer::SetDrawOutline(draw_text_outline);
      NeedsUpdate();
    }

    if (ImGui::Checkbox("Render using nodes", &ThreadTrack::render_with_nodes_)) {
      NeedsUpdate();
    }
  }

  if (ImGui::CollapsingHeader("Capture Info")) {
    IMGUI_VAR_TO_TEXT(screen_width_);
    IMGUI_VAR_TO_TEXT(screen_height_);
    IMGUI_VAR_TO_TEXT(world_height_);
    IMGUI_VAR_TO_TEXT(world_width_);
    IMGUI_VAR_TO_TEXT(world_top_left_x_);
    IMGUI_VAR_TO_TEXT(world_top_left_y_);
    IMGUI_VAR_TO_TEXT(world_min_width_);
    IMGUI_VAR_TO_TEXT(mouse_screen_x_);
    IMGUI_VAR_TO_TEXT(mouse_screen_y_);
    IMGUI_VAR_TO_TEXT(mouse_world_x_);
    IMGUI_VAR_TO_TEXT(mouse_world_y_);
    if (time_graph_ != nullptr) {
      IMGUI_VAR_TO_TEXT(time_graph_->GetNumDrawnTextBoxes());
      IMGUI_VAR_TO_TEXT(time_graph_->GetNumTimers());
      IMGUI_VAR_TO_TEXT(time_graph_->GetTrackManager()->GetAllTracks().size());
      IMGUI_VAR_TO_TEXT(time_graph_->GetTrackManager()->GetTracksTotalHeight());
      IMGUI_VAR_TO_TEXT(time_graph_->GetMinTimeUs());
      IMGUI_VAR_TO_TEXT(time_graph_->GetMaxTimeUs());
      IMGUI_VAR_TO_TEXT(time_graph_->GetCaptureMin());
      IMGUI_VAR_TO_TEXT(time_graph_->GetCaptureMax());
      IMGUI_VAR_TO_TEXT(time_graph_->GetTimeWindowUs());
      const CaptureData* capture_data = time_graph_->GetCaptureData();
      if (capture_data != nullptr) {
        IMGUI_VAR_TO_TEXT(capture_data->GetCallstackData()->callstack_events_by_tid().size());
        IMGUI_VAR_TO_TEXT(capture_data->GetCallstackData()->GetCallstackEventsCount());
      }
    }
  }
}

void CaptureWindow::RenderText(float layer) {
  ORBIT_SCOPE_FUNCTION;
  if (time_graph_ == nullptr) return;
  if (GetPickingMode() == PickingMode::kNone) {
    time_graph_->DrawText(this, layer);
  }
}

void ColorToFloat(Color color, float* output) {
  for (size_t i = 0; i < 4; ++i) {
    output[i] = static_cast<float>(color[i]) / 255.f;
  }
}

Vec2 ScreenToWorld(GlCanvas* canvas, Vec2 screen_pos) {
  Vec2 world_pos;
  canvas->ScreenToWorld(static_cast<int>(screen_pos[0]), static_cast<int>(screen_pos[1]),
                        world_pos[0], world_pos[1]);
  return world_pos;
}

void CaptureWindow::RenderHelpUi() {
  constexpr int kOffset = 30;
  float world_x = 0;
  float world_y = 0;
  ScreenToWorld(kOffset, kOffset, world_x, world_y);

  Vec2 text_bounding_box_pos;
  Vec2 text_bounding_box_size;
  // TODO (b/180312795): Use TimeGraphLayout's font size again.
  text_renderer_.AddText(GetHelpText(), world_x, world_y, GlCanvas::kZValueTextUi,
                         Color(255, 255, 255, 255), 14, -1.f /*max_size*/,
                         false /*right_justified*/, &text_bounding_box_pos,
                         &text_bounding_box_size);

  const Color kBoxColor(50, 50, 50, 230);
  const float kMargin = 15.f;
  const float kRoundingRadius = 20.f;
  ui_batcher_.AddRoundedBox(text_bounding_box_pos, text_bounding_box_size, GlCanvas::kZValueUi,
                            kRoundingRadius, kBoxColor, kMargin);
}

const char* CaptureWindow::GetHelpText() const {
  const char* help_message =
      "Start/Stop Capture: 'X'\n"
      "Pan: 'A','D' or \"Left Click + Drag\"\n"
      "Zoom: 'W', 'S', Scroll or \"Ctrl + Right Click + Drag\"\n"
      "Vertical Zoom: \"Ctrl + Scroll\"\n"
      "Select: Left Click\n"
      "Measure: \"Right Click + Drag\"\n"
      "Toggle Help: 'H'";
  return help_message;
}

inline double GetIncrementMs(double milli_seconds) {
  constexpr double kDay = 24 * 60 * 60 * 1000;
  constexpr double kHour = 60 * 60 * 1000;
  constexpr double kMinute = 60 * 1000;
  constexpr double kSecond = 1000;
  constexpr double kMilli = 1;
  constexpr double kMicro = 0.001;
  constexpr double kNano = 0.000001;

  std::string res;

  if (milli_seconds < kMicro) {
    return kNano;
  }
  if (milli_seconds < kMilli) {
    return kMicro;
  }
  if (milli_seconds < kSecond) {
    return kMilli;
  }
  if (milli_seconds < kMinute) {
    return kSecond;
  }
  if (milli_seconds < kHour) {
    return kMinute;
  }
  if (milli_seconds < kDay) {
    return kHour;
  }
  return kDay;
}

void CaptureWindow::RenderTimeBar() {
  if (time_graph_ == nullptr) return;
  static int num_time_points = 10;

  if (time_graph_->GetCaptureTimeSpanUs() > 0) {
    const float time_bar_height = time_graph_->GetLayout().GetTimeBarHeight();

    double millis = time_graph_->GetCurrentTimeSpanUs() * 0.001;
    double incr = millis / float(num_time_points - 1);
    double unit = GetIncrementMs(incr);
    double norm_inc = static_cast<int>((incr + unit) / unit) * unit;
    double start_ms = time_graph_->GetMinTimeUs() * 0.001;
    double norm_start_us = 1000.0 * static_cast<int>(start_ms / norm_inc) * norm_inc;

    static constexpr int kPixelMargin = 2;
    int screen_y = GetHeight() - static_cast<int>(time_bar_height) - kPixelMargin;
    float dummy;
    float world_y;
    ScreenToWorld(0, screen_y, dummy, world_y);

    float height = time_graph_->GetLayout().GetTimeBarHeight() - kPixelMargin;
    float x_margin = ScreenToWorldWidth(4);

    for (int i = 0; i < num_time_points; ++i) {
      double current_micros = norm_start_us + i * 1000 * norm_inc;
      if (current_micros < 0) continue;

      std::string text = GetPrettyTime(absl::Microseconds(current_micros));
      float world_x = time_graph_->GetWorldFromUs(current_micros);
      text_renderer_.AddText(text.c_str(), world_x + x_margin, world_y, GlCanvas::kZValueTimeBar,
                             Color(255, 255, 255, 255), time_graph_->GetLayout().GetFontSize());

      // Lines from time bar are drawn in screen space.
      Vec2 pos = QtScreenToGlScreen(WorldToScreen(Vec2(world_x, world_y)));
      ui_batcher_.AddVerticalLine(pos, height, GlCanvas::kZValueTimeBar, Color(255, 255, 255, 255));
    }
  }
}

void CaptureWindow::RenderSelectionOverlay() {
  if (time_graph_ == nullptr) return;
  if (!picking_ && select_start_[0] != select_stop_[0]) {
    uint64_t min_time = std::min(time_start_, time_stop_);
    uint64_t max_time = std::max(time_start_, time_stop_);

    float from = time_graph_->GetWorldFromTick(min_time);
    float to = time_graph_->GetWorldFromTick(max_time);

    float size_x = to - from;
    Vec2 pos(from, world_top_left_y_ - world_height_);
    Vec2 size(size_x, world_height_);

    std::string text = GetPrettyTime(TicksToDuration(min_time, max_time));
    const Color color(0, 128, 0, 128);

    Box box(pos, size, GlCanvas::kZValueOverlay);
    ui_batcher_.AddBox(box, color);

    const Color text_color(255, 255, 255, 255);
    float pos_x = pos[0] + size[0];

    text_renderer_.AddText(text.c_str(), pos_x, select_stop_[1], GlCanvas::kZValueTextUi,
                           text_color, time_graph_->GetLayout().GetFontSize(), size[0], true);

    const unsigned char g = 100;
    Color grey(g, g, g, 255);
    ui_batcher_.AddVerticalLine(pos, size[1], GlCanvas::kZValueOverlay, grey);
  }
}

void CaptureWindow::Initialize() { GlCanvas::Initialize(); }
