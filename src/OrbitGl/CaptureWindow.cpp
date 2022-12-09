// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/CaptureWindow.h"

#include <GteVector.h>
#include <absl/container/btree_map.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "CaptureClient/AppInterface.h"
#include "ClientData/CallstackData.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientFlags/ClientFlags.h"
#include "ClientProtos/capture_data.pb.h"
#include "DisplayFormats/DisplayFormats.h"
#include "Introspection/Introspection.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitAccessibility/AccessibleWidgetBridge.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlUtils.h"
#include "OrbitGl/OpenGlBatcher.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/QtTextRenderer.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TrackContainer.h"
#include "OrbitGl/TrackManager.h"
#include "OrbitGl/Viewport.h"

using orbit_accessibility::AccessibleInterface;
using orbit_accessibility::AccessibleWidgetBridge;

using orbit_client_data::CaptureData;
using orbit_gl::Batcher;
using orbit_gl::CaptureViewElement;
using orbit_gl::ModifierKeys;
using orbit_gl::TextRenderer;

constexpr const char* kTimingDraw = "Draw";
constexpr const char* kTimingDrawAndUpdatePrimitives = "Draw & Update Primitives";
constexpr const char* kTimingFrame = "Complete Frame";

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

CaptureWindow::CaptureWindow(
    OrbitApp* app, orbit_capture_client::CaptureControlInterface* capture_control_interface,
    TimeGraphLayout* time_graph_layout)
    : app_{app},
      capture_client_app_{capture_control_interface},
      time_graph_layout_{time_graph_layout} {
  draw_help_ = true;

  scoped_frame_times_[kTimingDraw] = std::make_unique<orbit_gl::SimpleTimings>(30);
  scoped_frame_times_[kTimingDrawAndUpdatePrimitives] =
      std::make_unique<orbit_gl::SimpleTimings>(30);
  scoped_frame_times_[kTimingFrame] = std::make_unique<orbit_gl::SimpleTimings>(30);
}

void CaptureWindow::PreRender() {
  GlCanvas::PreRender();

  if (ShouldAutoZoom()) {
    ZoomAll();
  }

  if (time_graph_ != nullptr) {
    int layout_loops = 0;

    // Layout changes of one element may require other elements to be updated as well,
    // so layouting needs to be done until all elements report that they do not need to
    // be updated further. As layout requests bubble up, it's enough to check this for
    // the root element (time graph) of the tree.
    // During loading or capturing, only a single layouting loop is executed as we're
    // streaming in data from a seperate thread (for performance reasons)
    const int max_layout_loops =
        (app_ != nullptr && (app_->IsCapturing() || app_->IsLoadingCapture()))
            ? 1
            : time_graph_layout_->GetMaxLayoutingLoops();

    // TODO (b/229222095) Log when the max loop count is exceeded
    do {
      time_graph_->UpdateLayout();
    } while (++layout_loops < max_layout_loops && time_graph_->HasLayoutChanged());
  }
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

  std::ignore = time_graph_->HandleMouseEvent(
      CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kMouseMove,
                                     viewport_.ScreenToWorld(Vec2i(x, y)), left, right, middle});

  // Pan
  if (left && !picking_manager_.IsDragging() && !capture_client_app_->IsCapturing()) {
    Vec2i mouse_click_screen = viewport_.WorldToScreen(mouse_click_pos_world_);
    Vec2 mouse_pos_world = viewport_.ScreenToWorld({x, y});
    time_graph_->GetTrackContainer()->SetVerticalScrollingOffset(
        track_container_click_scrolling_offset_ + mouse_click_pos_world_[1] - mouse_pos_world[1]);

    int timeline_width = viewport_.WorldToScreen(Vec2(time_graph_->GetTimelineWidth(), 0))[0];
    time_graph_->PanTime(mouse_click_screen[0], x, timeline_width, ref_time_click_);

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

  int timeline_width = viewport_.WorldToScreen(Vec2(time_graph_->GetTimelineWidth(), 0))[0];
  ref_time_click_ = time_graph_->GetTime(static_cast<double>(x) / timeline_width);
  track_container_click_scrolling_offset_ =
      time_graph_->GetTrackContainer()->GetVerticalScrollingOffset();
}

void CaptureWindow::LeftUp() {
  GlCanvas::LeftUp();

  if (!click_was_drag_ && background_clicked_) {
    app_->SelectTimer(nullptr);
    app_->set_selected_thread_id(orbit_base::kAllProcessThreadsTid);
    app_->set_selected_thread_state_slice(std::nullopt);
    RequestUpdatePrimitives();
  }

  if (time_graph_ != nullptr) {
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kLeftUp,
                                       viewport_.ScreenToWorld(mouse_move_pos_screen_)});
  }
}

void CaptureWindow::HandlePickedElement(PickingMode picking_mode, PickingId picking_id, int x,
                                        int y) {
  // Early-out: This makes sure the timegraph was not deleted in between redraw and mouse click
  if (time_graph_ == nullptr) return;
  PickingType type = picking_id.type;

  orbit_gl::Batcher& batcher = GetBatcherById(picking_id.batcher_id);

  if (picking_mode == PickingMode::kClick) {
    background_clicked_ = false;
    const orbit_gl::PickingUserData* user_data = batcher.GetUserData(picking_id);
    const orbit_client_protos::TimerInfo* timer_info =
        (user_data == nullptr ? nullptr : user_data->timer_info_);
    if (timer_info != nullptr) {
      SelectTimer(timer_info);
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
      const orbit_gl::PickingUserData* user_data = batcher.GetUserData(picking_id);

      if (user_data && user_data->generate_tooltip_) {
        tooltip = user_data->generate_tooltip_(picking_id);
      }
    }

    app_->SendTooltipToUi(tooltip);
  }
}

void CaptureWindow::SelectTimer(const TimerInfo* timer_info) {
  ORBIT_CHECK(time_graph_ != nullptr);
  if (timer_info == nullptr) return;

  app_->SelectTimer(timer_info);
  app_->set_selected_thread_id(timer_info->thread_id());

  if (double_clicking_) {
    // Zoom and center the text_box into the screen.
    time_graph_->Zoom(*timer_info);
  }
}

void CaptureWindow::PostRender(QPainter* painter) {
  if (picking_mode_ != PickingMode::kNone) {
    RequestUpdatePrimitives();
  }

  GlCanvas::PostRender(painter);
}

void CaptureWindow::RightDown(int x, int y) {
  GlCanvas::RightDown(x, y);
  if (time_graph_ != nullptr) {
    select_start_time_ = time_graph_->GetTickFromWorld(select_start_pos_world_[0]);
  }
}

void CaptureWindow::RightUp() {
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
      ORBIT_ERROR("%s", result.error().message());
    }
  }

  if (absl::GetFlag(FLAGS_time_range_selection)) {
    if (select_start_pos_world_[0] == select_stop_pos_world_[0]) {
      app_->ClearTimeRangeSelection();
    } else {
      app_->OnTimeRangeSelection(std::min(select_start_time_, select_stop_time_),
                                 std::max(select_start_time_, select_stop_time_));
    }
  }

  if (time_graph_ != nullptr) {
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kRightUp,
                                       viewport_.ScreenToWorld(mouse_move_pos_screen_)});
  }

  GlCanvas::RightUp();
}

void CaptureWindow::ZoomHorizontally(int delta, int mouse_x) {
  if (delta == 0) return;
  if (time_graph_ != nullptr) {
    float timeline_pos_x = time_graph_->GetTimelinePos()[0];
    double mouse_ratio =
        static_cast<double>(mouse_x - timeline_pos_x) / time_graph_->GetTimelineWidth();
    time_graph_->ZoomTime(delta, mouse_ratio);
  }
}

void CaptureWindow::Pan(float ratio) {
  if (time_graph_ == nullptr) return;

  int timeline_width = viewport_.WorldToScreen(Vec2(time_graph_->GetTimelineWidth(), 0))[0];
  double ref_time =
      time_graph_->GetTime(static_cast<double>(mouse_move_pos_screen_[0]) / timeline_width);
  time_graph_->PanTime(
      mouse_move_pos_screen_[0],
      mouse_move_pos_screen_[0] + static_cast<int>(ratio * static_cast<float>(timeline_width)),
      timeline_width, ref_time);
  RequestUpdatePrimitives();
}

void CaptureWindow::MouseWheelMoved(int x, int y, int delta, bool ctrl) {
  GlCanvas::MouseWheelMoved(x, y, delta, ctrl);

  if (time_graph_ != nullptr) {
    ModifierKeys modifiers;
    modifiers.ctrl = ctrl;
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{
            (delta > 0 ? CaptureViewElement::MouseEventType::kMouseWheelUp
                       : CaptureViewElement::MouseEventType::kMouseWheelDown),
            viewport_.ScreenToWorld(Vec2i(x, y))},
        modifiers);
  }
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
  constexpr float kPanRatioPerLeftAndRightArrowKeys = 0.1f;
  constexpr float kScrollingRatioPerUpAndDownArrowKeys = 0.05f;
  constexpr float kScrollingRatioPerPageUpAndDown = 0.9f;

  // TODO(b/234116147): Move this part to TimeGraph and manage events similarly to HandleMouseEvent.
  switch (key_code) {
    case ' ':
      if (!shift) {
        ZoomAll();
      }
      break;
    case 'A':
      Pan(kPanRatioPerLeftAndRightArrowKeys);
      break;
    case 'D':
      Pan(-kPanRatioPerLeftAndRightArrowKeys);
      break;
    case 'W':
      ZoomHorizontally(1, mouse_move_pos_screen_[0]);
      break;
    case 'S':
      ZoomHorizontally(-1, mouse_move_pos_screen_[0]);
      break;
    case 'X':
      ToggleRecording();
      break;
    // For arrow keys, we will scroll horizontally or vertically if no timer is selected. Otherwise,
    // jump to the neighbour timer in that direction.
    case 18:  // Left
      if (time_graph_ == nullptr) return;
      if (app_ == nullptr || app_->selected_timer() == nullptr) {
        Pan(kPanRatioPerLeftAndRightArrowKeys);
      } else if (shift) {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(),
                                         TimeGraph::JumpDirection::kPrevious,
                                         TimeGraph::JumpScope::kSameFunction);
      } else if (alt) {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(),
                                         TimeGraph::JumpDirection::kPrevious,
                                         TimeGraph::JumpScope::kSameThreadSameFunction);
      } else {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(),
                                         TimeGraph::JumpDirection::kPrevious,
                                         TimeGraph::JumpScope::kSameDepth);
      }
      break;
    case 20:  // Right
      if (time_graph_ == nullptr) return;
      if (app_ == nullptr || app_->selected_timer() == nullptr) {
        Pan(-kPanRatioPerLeftAndRightArrowKeys);
      } else if (shift) {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(), TimeGraph::JumpDirection::kNext,
                                         TimeGraph::JumpScope::kSameFunction);
      } else if (alt) {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(), TimeGraph::JumpDirection::kNext,
                                         TimeGraph::JumpScope::kSameThreadSameFunction);
      } else {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(), TimeGraph::JumpDirection::kNext,
                                         TimeGraph::JumpScope::kSameDepth);
      }
      break;
    case 19:  // Up
      if (time_graph_ == nullptr) return;
      if (app_ == nullptr || app_->selected_timer() == nullptr) {
        time_graph_->GetTrackContainer()->IncrementVerticalScroll(
            /*ratio=*/kScrollingRatioPerUpAndDownArrowKeys);
      } else {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(), TimeGraph::JumpDirection::kTop,
                                         TimeGraph::JumpScope::kSameThread);
      }
      break;
    case 21:  // Down
      if (time_graph_ == nullptr) return;
      if (app_ == nullptr || app_->selected_timer() == nullptr) {
        time_graph_->GetTrackContainer()->IncrementVerticalScroll(
            /*ratio=*/-kScrollingRatioPerUpAndDownArrowKeys);
      } else {
        time_graph_->JumpToNeighborTimer(app_->selected_timer(), TimeGraph::JumpDirection::kDown,
                                         TimeGraph::JumpScope::kSameThread);
      }
      break;
    case 22:  // Page Up
      if (time_graph_ == nullptr) return;
      time_graph_->GetTrackContainer()->IncrementVerticalScroll(
          /*ratio=*/kScrollingRatioPerPageUpAndDown);
      break;
    case 23:  // Page Down
      if (time_graph_ == nullptr) return;
      time_graph_->GetTrackContainer()->IncrementVerticalScroll(
          /*ratio=*/-kScrollingRatioPerPageUpAndDown);
      break;
    // Adding ']' here such that with e.g. German keyboards the ctrl+"+" shortcut works.
    // The '=' is added such that on English keyboards the combination works without the shift key.
    // As an unwanted side effect e.g. ctrl+"]" on an English keyboard also acts as vertical zoom.
    // A better solution would require localisation - see b/237773876.
    case '+':
    case '=':
    case ']':
      if (time_graph_ == nullptr) return;
      if (ctrl) {
        time_graph_->VerticalZoom(1, viewport_.ScreenToWorld(mouse_move_pos_screen_)[1]);
      }
      break;
    // Adding '_' such that on an English keyboard zooming out with both with and without the
    // shift key - see b/237773876 for details.
    case '-':
    case '_':
      if (time_graph_ == nullptr) return;
      if (ctrl) {
        time_graph_->VerticalZoom(-1, viewport_.ScreenToWorld(mouse_move_pos_screen_)[1]);
      }
      break;
  }
}

void CaptureWindow::SetIsMouseOver(bool value) {
  GlCanvas::SetIsMouseOver(value);

  if (time_graph_ != nullptr && !value) {
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kMouseLeave});
  }
}

bool CaptureWindow::ShouldAutoZoom() const { return capture_client_app_->IsCapturing(); }

std::unique_ptr<AccessibleInterface> CaptureWindow::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureWindow>(this);
}

void CaptureWindow::Draw(QPainter* painter) {
  ORBIT_SCOPE("CaptureWindow::Draw");
  uint64_t start_time_ns = orbit_base::CaptureTimestampNs();
  bool time_graph_was_redrawn = time_graph_ != nullptr && time_graph_->IsRedrawNeeded();

  text_renderer_.Init();

  if (ShouldSkipRendering()) {
    return;
  }

  if (ShouldAutoZoom()) {
    ZoomAll();
  }

  if (time_graph_ != nullptr) {
    time_graph_->DrawAllElements(primitive_assembler_, GetTextRenderer(), picking_mode_);
  }

  RenderSelectionOverlay();

  if (picking_mode_ == PickingMode::kNone) {
    if (draw_help_) {
      RenderHelpUi();
    }
  }

  if (picking_mode_ == PickingMode::kNone) {
    double update_duration_in_ms = (orbit_base::CaptureTimestampNs() - start_time_ns) / 1000000.0;
    if (time_graph_was_redrawn) {
      scoped_frame_times_[kTimingDrawAndUpdatePrimitives]->PushTimeMs(update_duration_in_ms);
    } else {
      scoped_frame_times_[kTimingDraw]->PushTimeMs(update_duration_in_ms);
    }
  }

  RenderAllLayers(painter);

  if (picking_mode_ == PickingMode::kNone) {
    if (last_frame_start_time_ != 0) {
      double frame_duration_in_ms =
          (orbit_base::CaptureTimestampNs() - last_frame_start_time_) / 1000000.0;
      scoped_frame_times_[kTimingFrame]->PushTimeMs(frame_duration_in_ms);
    }
  }

  last_frame_start_time_ = orbit_base::CaptureTimestampNs();
}

void CaptureWindow::RenderAllLayers(QPainter* painter) {
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
    ORBIT_ERROR("Too many z-layers. The current number is %d", all_layers.size());
  }

  for (float layer : all_layers) {
    if (time_graph_ != nullptr) {
      time_graph_->GetBatcher().DrawLayer(layer, picking_mode_ != PickingMode::kNone);
    }
    ui_batcher_.DrawLayer(layer, picking_mode_ != PickingMode::kNone);

    // The painter is in "native painting mode" all the time and we merely leave it for rendering
    // the text here. Compare GlCanvas::Render - that's where we enter native painting.
    CleanupGlState();
    painter->endNativePainting();

    if (picking_mode_ == PickingMode::kNone) {
      text_renderer_.RenderLayer(painter, layer);
      RenderText(painter, layer);
    }

    painter->beginNativePainting();
    PrepareGlState();
  }
}

void CaptureWindow::ToggleRecording() {
  capture_client_app_->ToggleCapture();
  draw_help_ = false;
#ifdef __linux__
  ZoomAll();
#endif
}

bool CaptureWindow::ShouldSkipRendering() const {
  // Don't render when loading a capture to avoid contention with the loading thread.
  return app_->IsLoadingCapture();
}

void CaptureWindow::set_draw_help(bool draw_help) {
  draw_help_ = draw_help;
  RequestRedraw();
}

void CaptureWindow::CreateTimeGraph(CaptureData* capture_data) {
  time_graph_ = std::make_unique<TimeGraph>(this, app_, &viewport_, capture_data,
                                            &GetPickingManager(), time_graph_layout_);
}

Batcher& CaptureWindow::GetBatcherById(BatcherId batcher_id) {
  switch (batcher_id) {
    case BatcherId::kTimeGraph:
      ORBIT_CHECK(time_graph_ != nullptr);
      return time_graph_->GetBatcher();
    case BatcherId::kUi:
      return ui_batcher_;
    default:
      ORBIT_UNREACHABLE();
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

std::string CaptureWindow::GetCaptureInfo() const {
  std::string capture_info;

  const auto append_variable = [&capture_info](std::string_view name, const auto& value) {
    if constexpr (std::is_floating_point_v<std::decay_t<decltype(value)>>) {
      absl::StrAppendFormat(&capture_info, "%s: %f\n", name, value);
    } else if constexpr (std::is_integral_v<std::decay_t<decltype(value)>>) {
      absl::StrAppendFormat(&capture_info, "%s: %d\n", name, value);
    } else {
      static_assert(!std::is_same_v<decltype(value), decltype(value)>,
                    "Value type is not supported.");
    }
  };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define APPEND_VARIABLE(name) append_variable(#name, name)

  APPEND_VARIABLE(viewport_.GetScreenWidth());
  APPEND_VARIABLE(viewport_.GetScreenHeight());
  APPEND_VARIABLE(viewport_.GetWorldWidth());
  APPEND_VARIABLE(viewport_.GetWorldHeight());
  APPEND_VARIABLE(mouse_move_pos_screen_[0]);
  APPEND_VARIABLE(mouse_move_pos_screen_[1]);
  if (time_graph_ != nullptr) {
    APPEND_VARIABLE(time_graph_->GetTrackContainer()->GetNumVisiblePrimitives());
    APPEND_VARIABLE(time_graph_->GetTrackManager()->GetAllTracks().size());
    APPEND_VARIABLE(time_graph_->GetMinTimeUs());
    APPEND_VARIABLE(time_graph_->GetMaxTimeUs());
    APPEND_VARIABLE(time_graph_->GetCaptureMin());
    APPEND_VARIABLE(time_graph_->GetCaptureMax());
    APPEND_VARIABLE(time_graph_->GetTimeWindowUs());
    const CaptureData* capture_data = time_graph_->GetCaptureData();
    if (capture_data != nullptr) {
      APPEND_VARIABLE(capture_data->GetCallstackData().GetCallstackEventsCount());
    }
  }

#undef APPEND_VARIABLE
  return capture_info;
}

std::string CaptureWindow::GetPerformanceInfo() const {
  std::string performance_info;
  for (const auto& item : scoped_frame_times_) {
    absl::StrAppendFormat(&performance_info, "Avg time for %s: %f ms\n", item.first,
                          item.second->GetAverageTimeMs());
    absl::StrAppendFormat(&performance_info, "Min time for %s: %f ms\n", item.first,
                          item.second->GetMinTimeMs());
    absl::StrAppendFormat(&performance_info, "Max time for %s: %f ms\n", item.first,
                          item.second->GetMaxTimeMs());
  }
  return performance_info;
}

std::string CaptureWindow::GetSelectionSummary() const { return selection_stats_.GetSummary(); }

void CaptureWindow::RenderText(QPainter* painter, float layer) {
  ORBIT_SCOPE_FUNCTION;
  if (time_graph_ == nullptr) return;
  if (picking_mode_ == PickingMode::kNone) {
    time_graph_->DrawText(painter, layer);
  }
}

void CaptureWindow::RenderHelpUi() {
  constexpr int kOffset = 30;
  Vec2 world_pos = viewport_.ScreenToWorld(Vec2i(kOffset, kOffset));

  Vec2 text_bounding_box_pos;
  Vec2 text_bounding_box_size;
  text_renderer_.AddText(
      GetHelpText(), world_pos[0], world_pos[1], GlCanvas::kZValueUi,
      {time_graph_layout_->GetFontSize(), Color(255, 255, 255, 255), -1.f /*max_size*/},
      &text_bounding_box_pos, &text_bounding_box_size);

  const Color box_color(50, 50, 50, 243);
  constexpr float kMargin = 15.f;
  constexpr float kRoundingRadius = 20.f;
  primitive_assembler_.AddRoundedBox(text_bounding_box_pos, text_bounding_box_size,
                                     GlCanvas::kZValueUi, kRoundingRadius, box_color, kMargin);
}

const char* CaptureWindow::GetHelpText() const {
  const char* help_message =
      u8"Start/Stop Capture: F5\n\n"
      "Pan:  \U0001F130 ,  \U0001F133  OR Left Click + Drag\n\n"
      "Scroll:  ← ,  ↑ ,  → ,  ↓  OR Mouse Wheel\n\n"
      "Timeline Zoom (10%):  \U0001F146 ,  \U0001F142  OR Ctrl + Mouse Wheel\n\n"
      "Zoom to Time Range: Ctrl + Right Click + Drag\n\n"
      "Select: Left Click\n\n"
      "Measure: Right Click + Drag\n\n"
      "UI Scale (10%): Ctrl + '±'\n\n"
      "Toggle Help: Ctrl +  \U0001F137";
  return help_message;
}

void CaptureWindow::RenderSelectionOverlay() {
  if (time_graph_ == nullptr) return;
  if (picking_mode_ != PickingMode::kNone) return;
  if (select_start_pos_world_[0] == select_stop_pos_world_[0]) return;

  uint64_t min_time = std::min(select_start_time_, select_stop_time_);
  uint64_t max_time = std::max(select_start_time_, select_stop_time_);

  float from_world = time_graph_->GetWorldFromTick(min_time);
  float to_world = time_graph_->GetWorldFromTick(max_time);
  float stop_pos_world = time_graph_->GetWorldFromTick(select_stop_time_);

  float size_x = to_world - from_world;
  // TODO(http://b/226401787): Allow green selection overlay to be on top of the Timeline after
  // modifying its design and how the overlay is drawn
  float initial_y_position = time_graph_layout_->GetTimeBarHeight();
  Vec2 pos(from_world, initial_y_position);
  Vec2 size(size_x, viewport_.GetWorldHeight() - initial_y_position);

  std::string text = orbit_display_formats::GetDisplayTime(TicksToDuration(min_time, max_time));
  const Color color(0, 128, 0, 128);

  Quad box = MakeBox(pos, size);
  primitive_assembler_.AddBox(box, GlCanvas::kZValueOverlay, color);

  TextRenderer::HAlign alignment = select_stop_pos_world_[0] < select_start_pos_world_[0]
                                       ? TextRenderer::HAlign::Left
                                       : TextRenderer::HAlign::Right;
  TextRenderer::TextFormatting formatting;
  formatting.font_size = time_graph_layout_->GetFontSize();
  formatting.color = Color(255, 255, 255, 255);
  formatting.halign = alignment;

  text_renderer_.AddText(text.c_str(), stop_pos_world, select_stop_pos_world_[1],
                         GlCanvas::kZValueOverlay, formatting);

  const unsigned char g = 100;
  Color grey(g, g, g, 255);
  primitive_assembler_.AddVerticalLine(pos, size[1], GlCanvas::kZValueOverlay, grey);
}
