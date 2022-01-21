// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <stddef.h>

#include <algorithm>
#include <limits>
#include <utility>

#include "AccessibleTrackContainer.h"
#include "App.h"
#include "CGroupAndProcessMemoryTrack.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "ClientData/FunctionUtils.h"
#include "ClientFlags/ClientFlags.h"
#include "DisplayFormats/DisplayFormats.h"
#include "Geometry.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Logging.h"
#include "PageFaultsTrack.h"
#include "PickingManager.h"
#include "SchedulerTrack.h"
#include "SystemMemoryTrack.h"
#include "TrackManager.h"
#include "VariableTrack.h"

namespace orbit_gl {

using orbit_capture_client::CaptureEventProcessor;

using orbit_client_data::CaptureData;
using orbit_client_data::TimerChain;
using orbit_client_protos::ApiTrackValue;
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::InstrumentedFunction;

TrackContainer::TrackContainer(CaptureViewElement* parent, TimelineInfoInterface* timeline_info,
                               Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                               CaptureData* capture_data)
    : CaptureViewElement(parent, viewport, &layout_),
      capture_data_{capture_data},
      thread_track_data_provider_(capture_data->GetThreadTrackDataProvider()),
      timeline_info_(timeline_info) {
  track_manager_ =
      std::make_unique<TrackManager>(this, timeline_info, viewport, layout, app, capture_data);
  track_manager_->GetOrCreateSchedulerTrack();
}

float TrackContainer::GetHeight() const {
  // Top and Bottom Margin. TODO: Margins should be treated in a different way (http://b/192070555).
  float total_height = layout_.GetSchedulerTrackOffset() + layout_.GetBottomMargin();

  // Track height including space between them
  for (auto& track : GetNonHiddenChildren()) {
    total_height += (track->GetHeight() + layout_.GetSpaceBetweenTracks());
  }
  return total_height;
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
void TrackContainer::VerticallyMoveIntoView(Track& track) {
  float pos = track.GetPos()[1] + vertical_scrolling_offset_;
  float height = track.GetHeight();

  float max_vertical_scrolling_offset = pos;
  float min_vertical_scrolling_offset =
      pos + height - viewport_->GetWorldHeight() + layout_.GetBottomMargin();
  SetVerticalScrollingOffset(std::clamp(vertical_scrolling_offset_, min_vertical_scrolling_offset,
                                        max_vertical_scrolling_offset));
}

std::vector<const TimerChain*> TrackContainer::GetAllThreadTrackTimerChains() const {
  return thread_track_data_provider_->GetAllThreadTimerChains();
}

int TrackContainer::GetNumVisiblePrimitives() const {
  int num_visible_primitives = 0;
  for (auto track : track_manager_->GetAllTracks()) {
    num_visible_primitives += track->GetVisiblePrimitiveCount();
  }
  return num_visible_primitives;
}

const TimerInfo* TrackContainer::FindPreviousFunctionCall(uint64_t function_address,
                                                          uint64_t current_time,
                                                          std::optional<uint32_t> thread_id) const {
  const TimerInfo* previous_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::lowest();
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    for (const auto& block : *chain) {
      if (!block.Intersects(goal_time, current_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        auto timer_end_time = timer_info.end();
        if ((timer_info.function_id() == function_address) &&
            (!thread_id || thread_id.value() == timer_info.thread_id()) &&
            (timer_end_time < current_time) && (goal_time < timer_end_time)) {
          previous_timer = &timer_info;
          goal_time = timer_end_time;
        }
      }
    }
  }
  return previous_timer;
}

const TimerInfo* TrackContainer::FindNextFunctionCall(uint64_t function_address,
                                                      uint64_t current_time,
                                                      std::optional<uint32_t> thread_id) const {
  const TimerInfo* next_timer = nullptr;
  uint64_t goal_time = std::numeric_limits<uint64_t>::max();
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    ORBIT_CHECK(chain != nullptr);
    for (const auto& block : *chain) {
      if (!block.Intersects(current_time, goal_time)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        auto timer_end_time = timer_info.end();
        if ((timer_info.function_id() == function_address) &&
            (!thread_id || thread_id.value() == timer_info.thread_id()) &&
            (timer_end_time > current_time) && (goal_time > timer_end_time)) {
          next_timer = &timer_info;
          goal_time = timer_end_time;
        }
      }
    }
  }
  return next_timer;
}

std::vector<const TimerInfo*> TrackContainer::GetAllTimersForHookedFunction(
    uint64_t function_address) const {
  std::vector<const TimerInfo*> timers;
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    ORBIT_CHECK(chain != nullptr);
    for (const auto& block : *chain) {
      for (uint64_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer = block[i];
        if (timer.function_id() == function_address) timers.push_back(&timer);
      }
    }
  }
  return timers;
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

  float current_y = layout_.GetSchedulerTrackOffset() - vertical_scrolling_offset_;

  // Track height including space between them
  for (auto& track : track_manager_->GetVisibleTracks()) {
    if (!track->IsMoving()) {
      track->SetPos(track_pos_x, current_y);
    }
    track->SetWidth(GetWidth());
    current_y += (track->GetHeight() + layout_.GetSpaceBetweenTracks());
  }
}

namespace {

[[nodiscard]] std::string GetLabelBetweenIterators(const InstrumentedFunction& function_a,
                                                   const InstrumentedFunction& function_b) {
  const std::string& function_from = function_a.function_name();
  const std::string& function_to = function_b.function_name();
  return absl::StrFormat("%s to %s", function_from, function_to);
}

std::string GetTimeString(const TimerInfo& timer_a, const TimerInfo& timer_b) {
  absl::Duration duration = TicksToDuration(timer_a.start(), timer_b.start());

  return orbit_display_formats::GetDisplayTime(duration);
}

[[nodiscard]] Color GetIteratorBoxColor(uint64_t index) {
  constexpr uint64_t kNumColors = 2;
  const Color kLightBlueGray = Color(177, 203, 250, 60);
  const Color kMidBlueGray = Color(81, 102, 157, 60);
  Color colors[kNumColors] = {kLightBlueGray, kMidBlueGray};
  return colors[index % kNumColors];
}

}  // namespace

void TrackContainer::DrawIteratorBox(Batcher& batcher, TextRenderer& text_renderer, Vec2 pos,
                                     Vec2 size, const Color& color, const std::string& label,
                                     const std::string& time, float text_box_y) {
  Box box(pos, size, GlCanvas::kZValueOverlay);
  batcher.AddBox(box, color);

  std::string text = absl::StrFormat("%s: %s", label, time);

  float max_size = size[0];

  const Color kBlack(0, 0, 0, 255);
  float text_width = text_renderer.AddTextTrailingCharsPrioritized(
      text.c_str(), pos[0], text_box_y + layout_.GetTextOffset(), GlCanvas::kZValueTextUi,
      {GetLayout().GetFontSize(), kBlack, max_size}, time.length());

  Vec2 white_box_size(std::min(static_cast<float>(text_width), max_size), GetTextBoxHeight());
  Vec2 white_box_position(pos[0], text_box_y);

  Box white_box(white_box_position, white_box_size, GlCanvas::kZValueOverlayTextBackground);

  const Color kWhite(255, 255, 255, 255);
  batcher.AddBox(white_box, kWhite);

  Vec2 line_from(pos[0] + white_box_size[0], white_box_position[1] + GetTextBoxHeight() / 2.f);
  Vec2 line_to(pos[0] + size[0], white_box_position[1] + GetTextBoxHeight() / 2.f);
  batcher.AddLine(line_from, line_to, GlCanvas::kZValueOverlay, Color(255, 255, 255, 255));
}

void TrackContainer::DrawOverlay(Batcher& batcher, TextRenderer& text_renderer,
                                 PickingMode picking_mode) {
  if (picking_mode != PickingMode::kNone || iterator_timer_info_.empty()) {
    return;
  }

  std::vector<std::pair<uint64_t, const TimerInfo*>> timers(iterator_timer_info_.size());
  std::copy(iterator_timer_info_.begin(), iterator_timer_info_.end(), timers.begin());

  // Sort timers by start time.
  std::sort(timers.begin(), timers.end(),
            [](const std::pair<uint64_t, const TimerInfo*>& timer_a,
               const std::pair<uint64_t, const TimerInfo*>& timer_b) -> bool {
              return timer_a.second->start() < timer_b.second->start();
            });

  // We will need the world x coordinates for the timers multiple times, so
  // we avoid recomputing them and just cache them here.
  std::vector<float> x_coords;
  x_coords.reserve(timers.size());

  float world_start_x = 0;
  float world_width = GetWidth();

  float world_start_y = 0;
  float world_height = viewport_->GetWorldHeight();

  double inv_time_window = 1.0 / timeline_info_->GetTimeWindowUs();

  // Draw lines for iterators.
  for (const auto& box : timers) {
    const TimerInfo* timer_info = box.second;

    double start_us = timeline_info_->GetUsFromTick(timer_info->start());
    double normalized_start = start_us * inv_time_window;
    auto world_timer_x = static_cast<float>(world_start_x + normalized_start * world_width);

    Vec2 pos(world_timer_x, world_start_y);
    x_coords.push_back(pos[0]);

    batcher.AddVerticalLine(pos, world_height, GlCanvas::kZValueOverlay,
                            TimeGraph::GetThreadColor(timer_info->thread_id()));
  }

  // Draw timers with timings between iterators.
  for (size_t k = 1; k < timers.size(); ++k) {
    Vec2 pos(x_coords[k - 1], world_start_y);
    float size_x = x_coords[k] - pos[0];
    Vec2 size(size_x, world_height);
    Color color = GetIteratorBoxColor(k - 1);

    uint64_t id_a = timers[k - 1].first;
    uint64_t id_b = timers[k].first;
    ORBIT_CHECK(iterator_id_to_function_id_.find(id_a) != iterator_id_to_function_id_.end());
    ORBIT_CHECK(iterator_id_to_function_id_.find(id_b) != iterator_id_to_function_id_.end());
    uint64_t function_a_id = iterator_id_to_function_id_.at(id_a);
    uint64_t function_b_id = iterator_id_to_function_id_.at(id_b);
    const InstrumentedFunction* function_a =
        capture_data_->GetInstrumentedFunctionById(function_a_id);
    const InstrumentedFunction* function_b =
        capture_data_->GetInstrumentedFunctionById(function_b_id);
    ORBIT_CHECK(function_a != nullptr);
    ORBIT_CHECK(function_b != nullptr);
    const std::string& label = GetLabelBetweenIterators(*function_a, *function_b);
    const std::string& time = GetTimeString(*timers[k - 1].second, *timers[k].second);

    // Distance from the bottom where we don't want to draw.
    float bottom_margin = layout_.GetBottomMargin();

    // The height of text is chosen such that the text of the last box drawn is
    // at pos[1] + bottom_margin (lowest possible position) and the height of
    // the box showing the overall time (see below) is at pos[1] + (world_height
    // / 2.f), corresponding to the case k == 0 in the formula for 'text_y'.
    float height_per_text = ((world_height / 2.f) - bottom_margin) /
                            static_cast<float>(iterator_timer_info_.size() - 1);
    float text_y = pos[1] + (world_height / 2.f) + static_cast<float>(k) * height_per_text -
                   layout_.GetTextBoxHeight();

    DrawIteratorBox(batcher, text_renderer, pos, size, color, label, time, text_y);
  }

  // When we have at least 3 boxes, we also draw the total time from the first
  // to the last iterator.
  if (timers.size() > 2) {
    size_t last_index = timers.size() - 1;

    Vec2 pos(x_coords[0], world_start_y);
    float size_x = x_coords[last_index] - pos[0];
    Vec2 size(size_x, world_height);

    std::string time = GetTimeString(*timers[0].second, *timers[last_index].second);
    std::string label("Total");

    float text_y = pos[1] + (world_height / 2.f);

    // We do not want the overall box to add any color, so we just set alpha to
    // 0.
    const Color kColorBlackTransparent(0, 0, 0, 0);
    DrawIteratorBox(batcher, text_renderer, pos, size, kColorBlackTransparent, label, time, text_y);
  }
}

void TrackContainer::DrawIncompleteDataIntervals(Batcher& batcher, PickingMode picking_mode) {
  if (picking_mode == PickingMode::kClick) return;  // Allow to click through.

  auto min_visible_timestamp_ns = timeline_info_->GetNsSinceStart(
      static_cast<uint64_t>(timeline_info_->GetMinTimeUs() * 1000L));
  auto max_visible_timestamp_ns = timeline_info_->GetNsSinceStart(
      static_cast<uint64_t>(timeline_info_->GetMaxTimeUs() * 1000L));

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

    static const Color kIncompleteDataIntervalOrange{255, 128, 0, 32};
    batcher.AddBox(Box{pos, size, z_value}, kIncompleteDataIntervalOrange, std::move(user_data));
  }
}

void TrackContainer::SetThreadFilter(const std::string& filter) {
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

std::pair<const TimerInfo*, const TimerInfo*> TrackContainer::GetMinMaxTimerInfoForFunction(
    uint64_t function_id) const {
  const TimerInfo* min_timer = nullptr;
  const TimerInfo* max_timer = nullptr;
  std::vector<const TimerChain*> chains = GetAllThreadTrackTimerChains();
  for (const TimerChain* chain : chains) {
    for (const auto& block : *chain) {
      for (size_t i = 0; i < block.size(); i++) {
        const TimerInfo& timer_info = block[i];
        if (timer_info.function_id() != function_id) continue;

        uint64_t elapsed_nanos = timer_info.end() - timer_info.start();
        if (min_timer == nullptr || elapsed_nanos < (min_timer->end() - min_timer->start())) {
          min_timer = &timer_info;
        }
        if (max_timer == nullptr || elapsed_nanos > (max_timer->end() - max_timer->start())) {
          max_timer = &timer_info;
        }
      }
    }
  }
  return std::make_pair(min_timer, max_timer);
}

void TrackContainer::DoDraw(Batcher& batcher, TextRenderer& text_renderer,
                            const DrawContext& draw_context) {
  CaptureViewElement::DoDraw(batcher, text_renderer, draw_context);

  DrawIncompleteDataIntervals(batcher, draw_context.picking_mode);
  DrawOverlay(batcher, text_renderer, draw_context.picking_mode);
}

void TrackContainer::SetVerticalScrollingOffset(float value) {
  float clamped_value = std::max(std::min(value, GetHeight() - viewport_->GetWorldHeight()), 0.f);
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
  return std::make_unique<AccessibleTrackContainer>(this);
}

}  // namespace orbit_gl
