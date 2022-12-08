// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TrackContainer.h"

#include <GteVector.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <stddef.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/TimestampIntervalSet.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Sort.h"
#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/GlUtils.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/ThreadColor.h"
#include "OrbitGl/ThreadTrack.h"
#include "OrbitGl/TrackManager.h"

namespace orbit_gl {

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleManager;
using orbit_client_data::ScopeId;
using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::InstrumentedFunction;

TrackContainer::TrackContainer(CaptureViewElement* parent, TimelineInfoInterface* timeline_info,
                               Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                               const ModuleManager* module_manager, CaptureData* capture_data)
    : CaptureViewElement(parent, viewport, layout),
      track_manager_{std::make_unique<TrackManager>(this, timeline_info, viewport, layout, app,
                                                    module_manager, capture_data)},
      capture_data_{capture_data},
      timeline_info_(timeline_info),
      app_{app} {
  track_manager_->GetOrCreateSchedulerTrack();
}

float TrackContainer::GetVisibleTracksTotalHeight() const {
  float visible_tracks_total_height = 0.f;

  // Track height including space between them.
  for (auto& track : GetNonHiddenChildren()) {
    visible_tracks_total_height += (track->GetHeight() + layout_->GetSpaceBetweenTracks());
  }
  return visible_tracks_total_height;
}

void TrackContainer::VerticalZoom(float real_ratio, float mouse_screen_y_position) {
  // Adjust the scrolling offset such that the point under the mouse stays the same if possible.
  // For this, calculate the "global" position (including scaling and scrolling offset) of the point
  // underneath the mouse with the old and new scaling, and adjust the scrolling to have them match.
  const float mouse_old_y_world_position = mouse_screen_y_position + vertical_scrolling_offset_;
  // Everything scales.
  const float mouse_new_y_world_position = mouse_old_y_world_position * real_ratio;

  SetVerticalScrollingOffset(mouse_new_y_world_position - mouse_screen_y_position);
}

void TrackContainer::VerticallyMoveIntoView(const TimerInfo& timer_info) {
  VerticallyMoveIntoView(*track_manager_->GetOrCreateTrackFromTimerInfo(timer_info));
}

// Move vertically the view to make a Track fully visible.
void TrackContainer::VerticallyMoveIntoView(const Track& track) {
  float relative_track_y_pos = track.GetPos()[1] - GetPos()[1] + vertical_scrolling_offset_;

  float max_vertical_scrolling_offset = relative_track_y_pos;
  float min_vertical_scrolling_offset = relative_track_y_pos + track.GetHeight() - GetHeight();
  SetVerticalScrollingOffset(std::clamp(vertical_scrolling_offset_, min_vertical_scrolling_offset,
                                        max_vertical_scrolling_offset));
}

int TrackContainer::GetNumVisiblePrimitives() const {
  int num_visible_primitives = 0;
  for (auto* track : track_manager_->GetAllTracks()) {
    num_visible_primitives += track->GetVisiblePrimitiveCount();
  }
  return num_visible_primitives;
}

void TrackContainer::DoUpdateLayout() {
  track_manager_->UpdateTrackListForRendering();
  UpdateTracksPosition();

  // This is called to make sure the current scrolling value is correctly clamped
  // in case any changes in track visibility occured before
  SetVerticalScrollingOffset(vertical_scrolling_offset_);
}

void TrackContainer::UpdateTracksPosition() {
  const float track_pos_x = GetPos()[0];

  float current_y = GetPos()[1] - vertical_scrolling_offset_;

  // Track height including space between them
  for (const auto& track : track_manager_->GetVisibleTracks()) {
    if (!track->IsMoving()) {
      track->SetPos(track_pos_x, current_y);
    }
    track->SetWidth(GetWidth());
    current_y += (track->GetHeight() + layout_->GetSpaceBetweenTracks());
  }
}

namespace {

[[nodiscard]] std::string GetLabelBetweenIterators(std::string_view function_from,
                                                   std::string_view function_to) {
  return absl::StrFormat("%s to %s", function_from, function_to);
}

std::string GetTimeString(const TimerInfo& timer_a, const TimerInfo& timer_b) {
  absl::Duration duration = TicksToDuration(timer_a.start(), timer_b.start());

  return orbit_display_formats::GetDisplayTime(duration);
}

[[nodiscard]] Color GetIteratorBoxColor(uint64_t index) {
  constexpr uint64_t kNumColors = 2;
  const Color light_blue_gray = Color(177, 203, 250, 60);
  const Color mid_blue_gray = Color(81, 102, 157, 60);
  Color colors[kNumColors] = {light_blue_gray, mid_blue_gray};
  return colors[index % kNumColors];
}

}  // namespace

void TrackContainer::DrawIteratorBox(PrimitiveAssembler& primitive_assembler,
                                     TextRenderer& text_renderer, Vec2 pos, Vec2 size,
                                     const Color& color, std::string_view label,
                                     std::string_view time, float text_box_y) {
  Quad box = MakeBox(pos, size);
  primitive_assembler.AddBox(box, GlCanvas::kZValueOverlay, color);

  std::string text = absl::StrFormat("%s: %s", label, time);

  float max_size = size[0];

  const Color black(0, 0, 0, 255);
  float text_width = text_renderer.AddTextTrailingCharsPrioritized(
      text.c_str(), pos[0], text_box_y + layout_->GetTextOffset(), GlCanvas::kZValueOverlayLabel,
      {layout_->GetFontSize(), black, max_size}, time.length());

  float box_height = layout_->GetTextBoxHeight();
  Vec2 white_box_size(std::min(static_cast<float>(text_width), max_size), box_height);
  Vec2 white_box_position(pos[0], text_box_y);

  Quad white_box = MakeBox(white_box_position, white_box_size);

  const Color white(255, 255, 255, 255);
  primitive_assembler.AddBox(white_box, GlCanvas::kZValueOverlayLabel, white);

  Vec2 line_from(pos[0] + white_box_size[0], white_box_position[1] + box_height / 2.f);
  Vec2 line_to(pos[0] + size[0], white_box_position[1] + box_height / 2.f);
  primitive_assembler.AddLine(line_from, line_to, GlCanvas::kZValueOverlay,
                              Color(255, 255, 255, 255));
}

void TrackContainer::DrawOverlay(PrimitiveAssembler& primitive_assembler,
                                 TextRenderer& text_renderer, PickingMode picking_mode) {
  if (picking_mode != PickingMode::kNone || iterator_timer_info_.empty()) {
    return;
  }

  std::vector<std::pair<uint64_t, const TimerInfo*>> timers(iterator_timer_info_.size());
  std::copy(iterator_timer_info_.begin(), iterator_timer_info_.end(), timers.begin());

  // Sort timers by start time.
  orbit_base::sort(timers.begin(), timers.end(),
                   [](const std::pair<uint64_t, const TimerInfo*>& timer_a) -> uint64_t {
                     return timer_a.second->start();
                   });

  // We will need the world x coordinates for the timers multiple times, so
  // we avoid recomputing them and just cache them here.
  std::vector<float> x_coords;
  x_coords.reserve(timers.size());

  float world_start_x = GetPos()[0];
  float width = GetWidth();

  float world_start_y = GetPos()[1];
  float height = GetHeight();

  double inv_time_window = 1.0 / timeline_info_->GetTimeWindowUs();

  // Draw lines for iterators.
  for (const auto& box : timers) {
    const TimerInfo* timer_info = box.second;

    double start_us = timeline_info_->GetUsFromTick(timer_info->start());
    double normalized_start = start_us * inv_time_window;
    auto world_timer_x = static_cast<float>(world_start_x + normalized_start * width);

    Vec2 pos(world_timer_x, world_start_y);
    x_coords.push_back(pos[0]);

    primitive_assembler.AddVerticalLine(pos, height, GlCanvas::kZValueOverlay,
                                        orbit_gl::GetThreadColor(timer_info->thread_id()));
  }

  // Draw timers with timings between iterators.
  for (size_t k = 1; k < timers.size(); ++k) {
    Vec2 pos(x_coords[k - 1], world_start_y);
    float size_x = x_coords[k] - pos[0];
    Vec2 size(size_x, height);
    Color color = GetIteratorBoxColor(k - 1);

    uint64_t id_a = timers[k - 1].first;
    uint64_t id_b = timers[k].first;
    ORBIT_CHECK(iterator_id_to_function_scope_id_.find(id_a) !=
                iterator_id_to_function_scope_id_.end());
    ORBIT_CHECK(iterator_id_to_function_scope_id_.find(id_b) !=
                iterator_id_to_function_scope_id_.end());
    ScopeId function_a_scope_id = iterator_id_to_function_scope_id_.at(id_a);
    ScopeId function_b_scope_id = iterator_id_to_function_scope_id_.at(id_b);
    const FunctionInfo* function_a = capture_data_->GetFunctionInfoByScopeId(function_a_scope_id);
    const FunctionInfo* function_b = capture_data_->GetFunctionInfoByScopeId(function_b_scope_id);
    ORBIT_CHECK(function_a != nullptr);
    ORBIT_CHECK(function_b != nullptr);
    const std::string& label =
        GetLabelBetweenIterators(function_a->pretty_name(), function_b->pretty_name());
    const std::string& time = GetTimeString(*timers[k - 1].second, *timers[k].second);

    // The height of text is chosen such that the text of the last box drawn is
    // at pos[1] (lowest possible position) and the height of the box showing the overall time (see
    // below) is at pos[1] + (height / 2.f), corresponding to the case k == 0 in the formula for
    // 'text_y'.
    float height_per_text = ((height / 2.f)) / static_cast<float>(iterator_timer_info_.size() - 1);
    float text_y = pos[1] + (height / 2.f) + static_cast<float>(k) * height_per_text -
                   layout_->GetTextBoxHeight();

    DrawIteratorBox(primitive_assembler, text_renderer, pos, size, color, label, time, text_y);
  }

  // When we have at least 3 boxes, we also draw the total time from the first
  // to the last iterator.
  if (timers.size() > 2) {
    size_t last_index = timers.size() - 1;

    Vec2 pos(x_coords[0], world_start_y);
    float size_x = x_coords[last_index] - pos[0];
    Vec2 size(size_x, height);

    std::string time = GetTimeString(*timers[0].second, *timers[last_index].second);
    std::string label("Total");

    float text_y = pos[1] + (height / 2.f);

    // We do not want the overall box to add any color, so we just set alpha to
    // 0.
    const Color color_black_transparent(0, 0, 0, 0);
    DrawIteratorBox(primitive_assembler, text_renderer, pos, size, color_black_transparent, label,
                    time, text_y);
  }
}

void TrackContainer::DrawIncompleteDataIntervals(PrimitiveAssembler& primitive_assembler,
                                                 PickingMode picking_mode) {
  if (picking_mode == PickingMode::kClick) return;  // Allow to click through.

  uint64_t min_visible_timestamp_ns = timeline_info_->GetTickFromUs(timeline_info_->GetMinTimeUs());
  uint64_t max_visible_timestamp_ns = timeline_info_->GetTickFromUs(timeline_info_->GetMaxTimeUs());

  std::vector<std::pair<float, float>> x_ranges;
  for (auto it = capture_data_->incomplete_data_intervals().LowerBound(min_visible_timestamp_ns);
       it != capture_data_->incomplete_data_intervals().end() &&
       it->start_inclusive() <= max_visible_timestamp_ns;
       ++it) {
    uint64_t start_timestamp_ns = it->start_inclusive();
    uint64_t end_timestamp_ns = it->end_exclusive();

    float start_x = timeline_info_->GetWorldFromTick(start_timestamp_ns);
    float end_x = timeline_info_->GetWorldFromTick(end_timestamp_ns);
    float width = end_x - start_x;
    constexpr float kMinWidth = 9.0f;
    // These intervals are very short, usually measurable in microseconds, but can have relatively
    // large effects on the capture. Extend ranges in order to make them visible even when not
    // zoomed very far in.
    if (width < kMinWidth) {
      const float center_x = (start_x + end_x) / 2;
      start_x = center_x - kMinWidth / 2;
      end_x = center_x + kMinWidth / 2;
      width = end_x - start_x;
    }

    // Merge ranges that are now overlapping due to having been extended for visibility.
    if (x_ranges.empty() || start_x > x_ranges.back().second) {
      x_ranges.emplace_back(start_x, end_x);
    } else {
      x_ranges.back().second = end_x;
    }
  }

  const float world_start_y = 0;
  const float world_height = viewport_->GetWorldHeight();

  // Actually draw the ranges.
  for (const auto& [start_x, end_x] : x_ranges) {
    const Vec2 pos{start_x, world_start_y};
    const Vec2 size{end_x - start_x, world_height};
    float z_value = GlCanvas::kZValueIncompleteDataOverlay;

    std::unique_ptr<PickingUserData> user_data = nullptr;
    // Show a tooltip when hovering.
    if (picking_mode == PickingMode::kHover) {
      // This overlay is placed in front of the tracks (with transparency), but when it comes to
      // tooltips give it a much lower Z value, so that it's possible to "hover through" it.
      z_value = GlCanvas::kZValueIncompleteDataOverlayPicking;
      user_data = std::make_unique<PickingUserData>(nullptr, [](PickingId /*id*/) {
        return std::string{
            "Capture data is incomplete in this time range. Some information might be inaccurate."};
      });
    }

    static const Color incomplete_data_interval_orange{255, 128, 0, 32};
    primitive_assembler.AddBox(MakeBox(pos, size), z_value, incomplete_data_interval_orange,
                               std::move(user_data));
  }
}

void TrackContainer::SetThreadFilter(std::string_view filter) {
  track_manager_->SetFilter(filter);
  RequestUpdate();
}

const TimerInfo* TrackContainer::FindPrevious(const TimerInfo& from) {
  Track* track = track_manager_->GetOrCreateTrackFromTimerInfo(from);
  if (track == nullptr) return nullptr;
  return track->GetLeft(from);
}

const TimerInfo* TrackContainer::FindNext(const TimerInfo& from) {
  Track* track = track_manager_->GetOrCreateTrackFromTimerInfo(from);
  if (track == nullptr) return nullptr;
  return track->GetRight(from);
}

const TimerInfo* TrackContainer::FindTop(const TimerInfo& from) {
  Track* track = track_manager_->GetOrCreateTrackFromTimerInfo(from);
  if (track == nullptr) return nullptr;
  return track->GetUp(from);
}

const TimerInfo* TrackContainer::FindDown(const TimerInfo& from) {
  Track* track = track_manager_->GetOrCreateTrackFromTimerInfo(from);
  if (track == nullptr) return nullptr;
  return track->GetDown(from);
}

void TrackContainer::SetIteratorOverlayData(
    const absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*>& iterator_timer_info,
    const absl::flat_hash_map<uint64_t, ScopeId>& iterator_id_to_function_scope_id) {
  iterator_timer_info_ = iterator_timer_info;
  iterator_id_to_function_scope_id_ = iterator_id_to_function_scope_id;
  RequestUpdate(RequestUpdateScope::kDraw);
}

void TrackContainer::DrawThreadDependencyArrow(
    PrimitiveAssembler& primitive_assembler,
    orbit_client_data::ThreadStateSliceInfo thread_state_slice, const Color& arrow_color,
    PickingMode picking_mode) {
  if (picking_mode != PickingMode::kNone) {
    return;
  }

  orbit_client_data::ThreadID start_arrow_thread = thread_state_slice.wakeup_tid();
  orbit_client_data::ThreadID end_arrow_thread = thread_state_slice.tid();

  std::optional<ThreadTrack*> start_arrow_track_opt =
      track_manager_->GetThreadTrack(start_arrow_thread);
  std::optional<ThreadTrack*> end_arrow_track_opt =
      track_manager_->GetThreadTrack(end_arrow_thread);

  if (!start_arrow_track_opt.has_value() || !end_arrow_track_opt.has_value()) {
    return;
  }

  ThreadTrack* start_arrow_track = start_arrow_track_opt.value();
  ThreadTrack* end_arrow_track = end_arrow_track_opt.value();

  const std::vector<Track*>& threads = track_manager_->GetVisibleTracks();

  if (std::find(threads.begin(), threads.end(), start_arrow_track) == threads.end() ||
      std::find(threads.begin(), threads.end(), end_arrow_track) == threads.end()) {
    return;
  }

  const float start_arrow_thread_state_half_height =
      start_arrow_track->GetThreadStateBarHeight() / 2;
  const float end_arrow_thread_state_half_height = end_arrow_track->GetThreadStateBarHeight() / 2;
  float x = timeline_info_->GetWorldFromTick(thread_state_slice.begin_timestamp_ns());
  float start_arrow_y =
      start_arrow_track->GetThreadStateBarPos()[1] + start_arrow_thread_state_half_height;
  float end_arrow_y =
      end_arrow_track->GetThreadStateBarPos()[1] + end_arrow_thread_state_half_height;
  float arrow_total_height = std::abs(end_arrow_y - start_arrow_y);
  float arrow_body_height = arrow_total_height - layout_->GetThreadDependencyArrowHeadHeight();
  PrimitiveAssembler::ArrowDirection arrow_direction =
      start_arrow_y < end_arrow_y ? PrimitiveAssembler::ArrowDirection::kDown
                                  : PrimitiveAssembler::ArrowDirection::kUp;
  primitive_assembler.AddVerticalArrow(
      Vec2{x, start_arrow_y}, Vec2{layout_->GetThreadDependencyArrowBodyWidth(), arrow_body_height},
      Vec2{layout_->GetThreadDependencyArrowHeadWidth(),
           layout_->GetThreadDependencyArrowHeadHeight()},
      GlCanvas::kZValueOverlay, arrow_color, arrow_direction);
}

void TrackContainer::DrawThreadDependency(PrimitiveAssembler& primitive_assembler,
                                          PickingMode picking_mode) {
  if (app_->selected_thread_state_slice() != std::nullopt) {
    const Color selected_slice_arrow_color{255, 255, 255, 255};
    DrawThreadDependencyArrow(primitive_assembler, app_->selected_thread_state_slice().value(),
                              selected_slice_arrow_color, picking_mode);
  }
  if (app_->hovered_thread_state_slice() != std::nullopt) {
    const Color hovered_slice_arrow_color{255, 255, 255, 64};
    DrawThreadDependencyArrow(primitive_assembler, app_->hovered_thread_state_slice().value(),
                              hovered_slice_arrow_color, picking_mode);
  }
}

void TrackContainer::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                            const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(primitive_assembler, text_renderer, draw_context);

  DrawIncompleteDataIntervals(primitive_assembler, draw_context.picking_mode);
  DrawThreadDependency(primitive_assembler, draw_context.picking_mode);
  DrawOverlay(primitive_assembler, text_renderer, draw_context.picking_mode);
}

void TrackContainer::UpdateVerticalScrollUsingRatio(float ratio) {
  float range = std::max(0.f, GetVisibleTracksTotalHeight() - GetHeight());
  float new_scrolling_offset = ratio * range;
  SetVerticalScrollingOffset(new_scrolling_offset);
}

void TrackContainer::IncrementVerticalScroll(float ratio) {
  SetVerticalScrollingOffset(vertical_scrolling_offset_ - ratio * GetHeight());
}

void TrackContainer::SetVerticalScrollingOffset(float value) {
  float clamped_value = std::max(std::min(value, GetVisibleTracksTotalHeight() - GetHeight()), 0.f);
  if (clamped_value == vertical_scrolling_offset_) return;

  vertical_scrolling_offset_ = clamped_value;
  RequestUpdate();
}

bool TrackContainer::HasFrameTrack(uint64_t function_id) const {
  auto frame_tracks = track_manager_->GetFrameTracks();
  return (std::find_if(frame_tracks.begin(), frame_tracks.end(), [&](auto frame_track) {
            return frame_track->GetFunctionId() == function_id;
          }) != frame_tracks.end());
}

void TrackContainer::RemoveFrameTrack(uint64_t function_id) {
  track_manager_->RemoveFrameTrack(function_id);
  RequestUpdate();
}

std::vector<CaptureViewElement*> TrackContainer::GetAllChildren() const {
  std::vector<Track*> all_tracks = track_manager_->GetAllTracks();
  return {all_tracks.begin(), all_tracks.end()};
}

std::vector<CaptureViewElement*> TrackContainer::GetNonHiddenChildren() const {
  std::vector<Track*> all_tracks = track_manager_->GetVisibleTracks();
  return {all_tracks.begin(), all_tracks.end()};
}

std::unique_ptr<orbit_accessibility::AccessibleInterface>
TrackContainer::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureViewElement>(
      this, "TrackContainer", orbit_accessibility::AccessibilityRole::Pane,
      orbit_accessibility::AccessibilityState::Focusable);
}

}  // namespace orbit_gl
