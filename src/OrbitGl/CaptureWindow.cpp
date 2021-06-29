// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureWindow.h"

#include <absl/time/time.h>
#include <glad/glad.h>
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
#include "ClientData/CallstackData.h"
#include "ClientModel/CaptureData.h"
#include "CoreMath.h"
#include "DisplayFormats/DisplayFormats.h"
#include "Geometry.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "Introspection/Introspection.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "Timer.h"
#include "TrackManager.h"
#include "absl/base/casts.h"
#include "capture_data.pb.h"

using orbit_accessibility::AccessibleInterface;
using orbit_accessibility::AccessibleWidgetBridge;
using orbit_client_model::CaptureData;

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

  slider_ = std::make_shared<orbit_gl::GlHorizontalSlider>(viewport_);
  vertical_slider_ = std::make_shared<orbit_gl::GlVerticalSlider>(viewport_);

  slider_->SetDragCallback([&](float ratio) {
    this->UpdateHorizontalScroll(ratio);
    RequestUpdatePrimitives();
  });
  slider_->SetResizeCallback([&](float normalized_start, float normalized_end) {
    this->UpdateHorizontalZoom(normalized_start, normalized_end);
    RequestUpdatePrimitives();
  });

  vertical_slider_->SetDragCallback([&](float ratio) {
    this->UpdateVerticalScroll(ratio);
    RequestUpdatePrimitives();
  });

  vertical_slider_->SetOrthogonalSliderPixelHeight(slider_->GetPixelHeight());
  slider_->SetOrthogonalSliderPixelHeight(vertical_slider_->GetPixelHeight());
}

void CaptureWindow::ZoomAll() {
  ResetHoverTimer();
  RequestUpdatePrimitives();
  if (time_graph_ == nullptr) return;
  time_graph_->ZoomAll();
}

void CaptureWindow::MouseMoved(int x, int y, bool left, bool right, bool middle) {
  GlCanvas::MouseMoved(x, y, left, right, middle);

  if (time_graph_ == nullptr) return;

  // Pan
  if (left && !picking_manager_.IsDragging() && !app_->IsCapturing()) {
    Vec2i mouse_click_screen = viewport_.WorldToScreenPos(mouse_click_pos_world_);
    time_graph_->PanTime(mouse_click_screen[0], x, viewport_.GetScreenWidth(), ref_time_click_);
    RequestUpdatePrimitives();

    click_was_drag_ = true;
  }

  // Update selection timestamps
  if (is_selecting_) {
    select_stop_time_ = time_graph_->GetTickFromWorld(select_stop_pos_world_[0]);
  }
}

void CaptureWindow::LeftDown(int x, int y) {
  GlCanvas::LeftDown(x, y);

  click_was_drag_ = false;

  if (time_graph_ == nullptr) return;
  ref_time_click_ = time_graph_->GetTime(static_cast<double>(x) / viewport_.GetScreenWidth());
}

void CaptureWindow::LeftUp() {
  GlCanvas::LeftUp();

  if (!click_was_drag_ && background_clicked_) {
    app_->SelectTextBox(nullptr);
    app_->set_selected_thread_id(orbit_base::kAllProcessThreadsTid);
    RequestUpdatePrimitives();
  }
}

void CaptureWindow::HandlePickedElement(PickingMode picking_mode, PickingId picking_id, int x,
                                        int y) {
  // Early-out: This makes sure the timegraph was not deleted in between redraw and mouse click
  if (time_graph_ == nullptr) return;
  PickingType type = picking_id.type;

  Batcher& batcher = GetBatcherById(picking_id.batcher_id);

  if (picking_mode == PickingMode::kClick) {
    background_clicked_ = false;
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
  } else if (picking_mode == PickingMode::kHover) {
    std::string tooltip;

    if (type == PickingType::kPickable) {
      auto pickable = GetPickingManager().GetPickableFromId(picking_id);
      if (pickable) {
        tooltip = pickable->GetTooltip();
      }
    } else {
      PickingUserData* user_data = batcher.GetUserData(picking_id);

      if (user_data && user_data->generate_tooltip_) {
        tooltip = user_data->generate_tooltip_(picking_id);
      }
    }

    app_->SendTooltipToUi(tooltip);
  }
}

void CaptureWindow::SelectTextBox(const TextBox* text_box) {
  CHECK(time_graph_ != nullptr);
  if (text_box == nullptr) return;

  app_->SelectTextBox(text_box);
  app_->set_selected_thread_id(text_box->GetTimerInfo().thread_id());

  const TimerInfo& timer_info = text_box->GetTimerInfo();

  if (double_clicking_) {
    // Zoom and center the text_box into the screen and make its track fully visible.
    time_graph_->Zoom(timer_info);
    time_graph_->SelectAndMakeVisible(text_box);
  }
}

void CaptureWindow::PostRender() {
  if (picking_mode_ != PickingMode::kNone) {
    RequestUpdatePrimitives();
  }

  GlCanvas::PostRender();
}

void CaptureWindow::RightDown(int x, int y) {
  GlCanvas::RightDown(x, y);
  if (time_graph_ != nullptr) {
    select_start_time_ = time_graph_->GetTickFromWorld(select_start_pos_world_[0]);
  }
}

bool CaptureWindow::RightUp() {
  if (time_graph_ != nullptr && is_selecting_ &&
      (select_start_pos_world_[0] != select_stop_pos_world_[0]) && ControlPressed()) {
    float min_world = std::min(select_start_pos_world_[0], select_stop_pos_world_[0]);
    float max_world = std::max(select_start_pos_world_[0], select_stop_pos_world_[0]);

    double new_min =
        TicksToMicroseconds(time_graph_->GetCaptureMin(), time_graph_->GetTickFromWorld(min_world));
    double new_max =
        TicksToMicroseconds(time_graph_->GetCaptureMin(), time_graph_->GetTickFromWorld(max_world));

    time_graph_->SetMinMax(new_min, new_max);

    // Clear the selection display
    select_stop_pos_world_ = select_start_pos_world_;
  }

  if (app_->IsDevMode()) {
    auto result = selection_stats_.Generate(this, select_start_time_, select_stop_time_);
    if (result.has_error()) {
      ERROR("%s", result.error().message());
    }
  }

  return GlCanvas::RightUp();
}

void CaptureWindow::Zoom(ZoomDirection dir, int delta) {
  if (delta == 0) return;

  auto delta_float = static_cast<float>(-delta);

  if (time_graph_ != nullptr) {
    switch (dir) {
      case ZoomDirection::kHorizontal: {
        double mouse_ratio =
            static_cast<double>(mouse_move_pos_screen_[0]) / viewport_.GetScreenWidth();
        time_graph_->ZoomTime(delta_float, mouse_ratio);
        break;
      }
      case ZoomDirection::kVertical: {
        float mouse_ratio = static_cast<float>(mouse_move_pos_screen_[1]) /
                            static_cast<float>(viewport_.GetScreenHeight());
        time_graph_->VerticalZoom(delta_float, mouse_ratio);
      }
    }
  }

  RequestUpdatePrimitives();
}

void CaptureWindow::Pan(float ratio) {
  if (time_graph_ == nullptr) return;
  double ref_time = time_graph_->GetTime(static_cast<double>(mouse_move_pos_screen_[0]) /
                                         viewport_.GetScreenWidth());
  time_graph_->PanTime(mouse_move_pos_screen_[0],
                       mouse_move_pos_screen_[0] +
                           static_cast<int>(ratio * static_cast<float>(viewport_.GetScreenWidth())),
                       viewport_.GetScreenWidth(), ref_time);
  RequestUpdatePrimitives();
}

void CaptureWindow::MouseWheelMoved(int x, int y, int delta, bool ctrl) {
  GlCanvas::MouseWheelMoved(x, y, delta, ctrl);

  const int delta_normalized = delta < 0 ? -1 : 1;
  Zoom(ctrl ? ZoomDirection::kVertical : ZoomDirection::kHorizontal, delta_normalized);
}

void CaptureWindow::MouseWheelMovedHorizontally(int x, int y, int delta, bool ctrl) {
  GlCanvas::MouseWheelMovedHorizontally(x, y, delta, ctrl);

  if (delta == 0) return;

  if (delta > 0) {
    Pan(0.1f);
  } else {
    Pan(-0.1f);
  }
}

void CaptureWindow::KeyPressed(unsigned int key_code, bool ctrl, bool shift, bool alt) {
  GlCanvas::KeyPressed(key_code, ctrl, shift, alt);

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
      Zoom(ZoomDirection::kHorizontal, 1);
      break;
    case 'S':
      Zoom(ZoomDirection::kHorizontal, -1);
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

bool CaptureWindow::ShouldAutoZoom() const { return app_->IsCapturing(); }

std::unique_ptr<AccessibleInterface> CaptureWindow::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureWindow>(this);
}

void CaptureWindow::Draw(bool viewport_was_dirty) {
  ORBIT_SCOPE("CaptureWindow::Draw");

  if (ShouldSkipRendering()) {
    return;
  }

  if (ShouldAutoZoom()) {
    ZoomAll();
  }

  if (time_graph_ != nullptr) {
    if (viewport_was_dirty) {
      time_graph_->SetPos(0, 0);
      time_graph_->SetSize(viewport_.GetWorldExtents()[0], viewport_.GetWorldExtents()[1]);

      time_graph_->RequestUpdate();
    }
    uint64_t timegraph_current_mouse_time_ns =
        time_graph_->GetTickFromWorld(viewport_.ScreenToWorldPos(GetMouseScreenPos())[0]);
    time_graph_->Draw(GetBatcher(), GetTextRenderer(), timegraph_current_mouse_time_ns,
                      picking_mode_);
    viewport_.SetWorldExtents(viewport_.GetScreenWidth(),
                              time_graph_->GetTrackManager()->GetTracksTotalHeight());
  }

  RenderSelectionOverlay();

  if (picking_mode_ == PickingMode::kNone) {
    RenderTimeBar();

    Vec2 pos = viewport_.ScreenToWorldPos(Vec2i(mouse_move_pos_screen_[0], 0));
    // Vertical green line at mouse x position
    ui_batcher_.AddVerticalLine(pos, -viewport_.GetVisibleWorldHeight(), kZValueUi,
                                Color(0, 255, 0, 127));

    if (draw_help_) {
      RenderHelpUi();
    }
  }

  DrawScreenSpace();

  if (picking_mode_ == PickingMode::kNone) {
    text_renderer_.RenderDebug(&ui_batcher_);
  }

  // Get all layers.
  std::vector<float> all_layers{};
  if (time_graph_ != nullptr) {
    all_layers = time_graph_->GetBatcher().GetLayers();
    orbit_base::Append(all_layers, time_graph_->GetTextRenderer()->GetLayers());
  }
  orbit_base::Append(all_layers, ui_batcher_.GetLayers());
  orbit_base::Append(all_layers, text_renderer_.GetLayers());

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
      PrepareWorldSpaceViewport();
    }
    if (time_graph_ != nullptr) {
      time_graph_->GetBatcher().DrawLayer(layer, picking_mode_ != PickingMode::kNone);
    }
    ui_batcher_.DrawLayer(layer, picking_mode_ != PickingMode::kNone);

    PrepareScreenSpaceViewport();
    // Text needs to be drawn in screen space.
    if (picking_mode_ == PickingMode::kNone) {
      text_renderer_.RenderLayer(layer);
      RenderText(layer);
    }
  }
}

void CaptureWindow::DrawScreenSpace() {
  ORBIT_SCOPE("CaptureWindow::DrawScreenSpace");
  if (time_graph_ == nullptr) return;
  double time_span = time_graph_->GetCaptureTimeSpanUs();

  Color col = slider_->GetBarColor();
  auto canvas_height = static_cast<float>(viewport_.GetScreenHeight());

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  float right_margin = layout.GetRightMargin();

  if (time_span > 0) {
    UpdateHorizontalSliderFromWorld();
    slider_->Draw(ui_batcher_, picking_manager_.IsThisElementPicked(slider_.get()));

    UpdateVerticalSliderFromWorld();
    if (vertical_slider_->GetLengthRatio() < 1.f) {
      vertical_slider_->Draw(ui_batcher_,
                             picking_manager_.IsThisElementPicked(vertical_slider_.get()));
      int slider_width = static_cast<int>(time_graph_->GetLayout().GetSliderWidth());
      right_margin += slider_width;
    }
  }

  // Right vertical margin.
  time_graph_->UpdateRightMargin(right_margin);
  auto margin_x1 = static_cast<float>(viewport_.GetScreenWidth());
  float margin_x0 = margin_x1 - right_margin;

  Box box(Vec2(margin_x0, 0), Vec2(margin_x1 - margin_x0, canvas_height), GlCanvas::kZValueMargin);
  ui_batcher_.AddBox(box, kBackgroundColor);

  // Time bar background
  if (time_graph_->GetCaptureTimeSpanUs() > 0) {
    Box background_box(
        Vec2(0, time_graph_->GetLayout().GetSliderWidth()),
        Vec2(viewport_.GetScreenWidth(), time_graph_->GetLayout().GetTimeBarHeight()),
        GlCanvas::kZValueTimeBarBg);
    ui_batcher_.AddBox(background_box, kTimeBarBackgroundColor);
  }
}

void CaptureWindow::UpdateHorizontalScroll(float ratio) {
  if (time_graph_ == nullptr) return;
  time_graph_->UpdateHorizontalScroll(ratio);
}

void CaptureWindow::UpdateVerticalScroll(float ratio) {
  if (time_graph_ == nullptr) return;
  float min = viewport_.GetWorldMin()[1];
  float max = viewport_.GetVisibleWorldHeight() - viewport_.GetWorldExtents()[1] +
              viewport_.GetWorldMin()[1];
  float range = max - min;
  float new_top_left_y = min + ratio * range;
  if (new_top_left_y != viewport_.GetWorldTopLeft()[1]) {
    viewport_.SetWorldTopLeftY(new_top_left_y);
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
  float min = 0.0f;
  float max = viewport_.GetVisibleWorldHeight() - viewport_.GetWorldExtents()[1];
  float ratio = (viewport_.GetWorldTopLeft()[1] - min) / (max - min);
  float vertical_ratio = viewport_.GetVisibleWorldHeight() / viewport_.GetWorldExtents()[1];
  int slider_width = static_cast<int>(time_graph_->GetLayout().GetSliderWidth());
  vertical_slider_->SetPixelHeight(slider_width);
  vertical_slider_->SetNormalizedPosition(ratio);
  vertical_slider_->SetNormalizedLength(vertical_ratio);
  vertical_slider_->SetOrthogonalSliderPixelHeight(slider_->GetPixelHeight());
}

void CaptureWindow::ToggleRecording() {
  app_->ToggleCapture();
  draw_help_ = false;
#ifdef __linux__
  ZoomAll();
#endif
}

bool CaptureWindow::ShouldSkipRendering() const {
  // Don't render when loading a capture to avoid contention with the loading thread.
  return app_->IsLoadingCapture();
}

void CaptureWindow::ToggleDrawHelp() { set_draw_help(!draw_help_); }

void CaptureWindow::set_draw_help(bool draw_help) {
  draw_help_ = draw_help;
  RequestRedraw();
}

void CaptureWindow::CreateTimeGraph(const CaptureData* capture_data) {
  time_graph_ =
      std::make_unique<TimeGraph>(this, app_, &viewport_, capture_data, &GetPickingManager());
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

void CaptureWindow::RequestUpdatePrimitives() {
  redraw_requested_ = true;
  if (time_graph_ == nullptr) return;
  time_graph_->RequestUpdate();
}

[[nodiscard]] bool CaptureWindow::IsRedrawNeeded() const {
  return GlCanvas::IsRedrawNeeded() || (time_graph_ != nullptr && time_graph_->IsRedrawNeeded());
}

void CaptureWindow::RenderImGuiDebugUI() {
  if (ImGui::CollapsingHeader("Layout Properties")) {
    if (time_graph_ != nullptr && time_graph_->GetLayout().DrawProperties()) {
      RequestUpdatePrimitives();
    }

    static bool draw_text_outline = false;
    if (ImGui::Checkbox("Draw Text Outline", &draw_text_outline)) {
      TextRenderer::SetDrawOutline(draw_text_outline);
      RequestUpdatePrimitives();
    }
  }

  if (ImGui::CollapsingHeader("Capture Info")) {
    IMGUI_VAR_TO_TEXT(viewport_.GetScreenWidth());
    IMGUI_VAR_TO_TEXT(viewport_.GetScreenHeight());
    IMGUI_VAR_TO_TEXT(viewport_.GetVisibleWorldHeight());
    IMGUI_VAR_TO_TEXT(viewport_.GetVisibleWorldWidth());
    IMGUI_VAR_TO_TEXT(viewport_.GetWorldTopLeft()[0]);
    IMGUI_VAR_TO_TEXT(viewport_.GetWorldTopLeft()[1]);
    IMGUI_VAR_TO_TEXT(viewport_.GetWorldExtents()[0]);
    IMGUI_VAR_TO_TEXT(viewport_.GetWorldExtents()[1]);
    IMGUI_VAR_TO_TEXT(mouse_move_pos_screen_[0]);
    IMGUI_VAR_TO_TEXT(mouse_move_pos_screen_[1]);
    if (time_graph_ != nullptr) {
      IMGUI_VAR_TO_TEXT(time_graph_->GetNumDrawnTextBoxes());
      IMGUI_VAR_TO_TEXT(time_graph_->GetTrackManager()->GetNumTimers());
      IMGUI_VAR_TO_TEXT(time_graph_->GetTrackManager()->GetAllTracks().size());
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

  if (ImGui::CollapsingHeader("Selection Summary")) {
    const std::string& selection_summary = selection_stats_.GetSummary();

    if (ImGui::Button("Copy to clipboard")) {
      app_->SetClipboard(selection_summary);
    }

    ImGui::TextUnformatted(selection_summary.c_str(),
                           selection_summary.c_str() + selection_summary.size());
  }
}

void CaptureWindow::RenderText(float layer) {
  ORBIT_SCOPE_FUNCTION;
  if (time_graph_ == nullptr) return;
  if (picking_mode_ == PickingMode::kNone) {
    time_graph_->DrawText(layer);
  }
}

void CaptureWindow::RenderHelpUi() {
  constexpr int kOffset = 30;
  Vec2 world_pos = viewport_.ScreenToWorldPos(Vec2i(kOffset, kOffset));

  Vec2 text_bounding_box_pos;
  Vec2 text_bounding_box_size;
  // TODO(b/180312795): Use TimeGraphLayout's font size again.
  text_renderer_.AddText(GetHelpText(), world_pos[0], world_pos[1], GlCanvas::kZValueTextUi,
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
    int screen_y = viewport_.GetScreenHeight() - static_cast<int>(time_bar_height) - kPixelMargin;
    Vec2 world_pos = viewport_.ScreenToWorldPos(Vec2i(0, screen_y));

    float height = time_graph_->GetLayout().GetTimeBarHeight() - kPixelMargin;
    float x_margin = viewport_.ScreenToWorldWidth(4);

    for (int i = 0; i < num_time_points; ++i) {
      double current_micros = norm_start_us + i * 1000 * norm_inc;
      if (current_micros < 0) continue;

      std::string text = orbit_display_formats::GetDisplayTime(absl::Microseconds(current_micros));
      float world_x = time_graph_->GetWorldFromUs(current_micros);
      text_renderer_.AddText(text.c_str(), world_x + x_margin, world_pos[1],
                             GlCanvas::kZValueTimeBar, Color(255, 255, 255, 255),
                             time_graph_->GetLayout().GetFontSize());

      // Lines from time bar are drawn in screen space.
      Vec2i pos =
          viewport_.QtToGlScreenPos(viewport_.WorldToScreenPos(Vec2(world_x, world_pos[1])));
      ui_batcher_.AddVerticalLine(Vec2(pos[0], pos[1]), height, GlCanvas::kZValueTimeBar,
                                  Color(255, 255, 255, 255));
    }
  }
}

void CaptureWindow::RenderSelectionOverlay() {
  if (time_graph_ == nullptr) return;
  if (picking_mode_ != PickingMode::kNone) return;
  if (select_start_pos_world_[0] == select_stop_pos_world_[0]) return;

  uint64_t min_time = std::min(select_start_time_, select_stop_time_);
  uint64_t max_time = std::max(select_start_time_, select_stop_time_);

  float from_world = time_graph_->GetWorldFromTick(min_time);
  float to_world = time_graph_->GetWorldFromTick(max_time);

  float size_x = to_world - from_world;
  Vec2 pos(from_world, viewport_.GetWorldTopLeft()[1] - viewport_.GetVisibleWorldHeight());
  Vec2 size(size_x, viewport_.GetVisibleWorldHeight());

  std::string text = orbit_display_formats::GetDisplayTime(TicksToDuration(min_time, max_time));
  const Color color(0, 128, 0, 128);

  Box box(pos, size, GlCanvas::kZValueOverlay);
  ui_batcher_.AddBox(box, color);

  const Color text_color(255, 255, 255, 255);
  float pos_x = pos[0] + size[0];

  text_renderer_.AddText(text.c_str(), pos_x, select_stop_pos_world_[1], GlCanvas::kZValueTextUi,
                         text_color, time_graph_->GetLayout().GetFontSize(), size[0], true);

  const unsigned char g = 100;
  Color grey(g, g, g, 255);
  ui_batcher_.AddVerticalLine(pos, size[1], GlCanvas::kZValueOverlay, grey);
}
