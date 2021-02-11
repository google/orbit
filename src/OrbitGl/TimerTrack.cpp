// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimerTrack.h"

#include <GteVector.h>
#include <absl/flags/declare.h>
#include <absl/synchronization/mutex.h>
#include <stddef.h>

#include <algorithm>
#include <limits>
#include <utility>

#include "App.h"
#include "Batcher.h"
#include "GlCanvas.h"
#include "TextBox.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TriangleToggle.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::TimerInfo;

ABSL_DECLARE_FLAG(bool, show_return_values);

TimerTrack::TimerTrack(TimeGraph* time_graph, OrbitApp* app, CaptureData* capture_data)
    : Track(time_graph, capture_data), app_{app} {
  text_renderer_ = time_graph->GetTextRenderer();
}

void TimerTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  float track_height = GetHeight();
  float track_width = canvas->GetWorldWidth();

  SetPos(canvas->GetWorldTopLeftX(), pos_[1]);
  SetSize(track_width, track_height);

  Track::Draw(canvas, picking_mode, z_offset);
}

std::string TimerTrack::GetExtraInfo(const TimerInfo& timer_info) {
  std::string info;
  static bool show_return_value = absl::GetFlag(FLAGS_show_return_values);
  if (show_return_value && timer_info.type() == TimerInfo::kNone) {
    info = absl::StrFormat("[%lu]", timer_info.user_data_key());
  }
  return info;
}

float TimerTrack::GetYFromTimer(const TimerInfo& timer_info) const {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  return pos_[1] - GetHeaderHeight() - layout.GetSpaceBetweenTracksAndThread() -
         box_height_ * static_cast<float>(timer_info.depth() + 1);
}

void TimerTrack::UpdateBoxHeight() { box_height_ = time_graph_->GetLayout().GetTextBoxHeight(); }

float TimerTrack::GetTextBoxHeight(const TimerInfo& /*timer_info*/) const { return box_height_; }

namespace {
struct WorldXInfo {
  float world_x_start;
  float world_x_width;
};

WorldXInfo ToWorldX(double start_us, double end_us, double inv_time_window, float world_start_x,
                    float world_width) {
  double width_us = end_us - start_us;

  double normalized_start = start_us * inv_time_window;
  double normalized_width = width_us * inv_time_window;

  WorldXInfo result{};
  result.world_x_start = static_cast<float>(world_start_x + normalized_start * world_width);
  result.world_x_width = static_cast<float>(normalized_width * world_width);
  return result;
}

}  // namespace

void TimerTrack::DrawTimer(TextBox* prev_text_box, TextBox* current_text_box,
                           TextBox* next_text_box, uint64_t min_tick, uint64_t max_tick,
                           float z_offset, Batcher* batcher, GlCanvas* canvas, float world_start_x,
                           float world_width, double inv_time_window, bool is_collapsed, float z,
                           const TextBox* selected_textbox, uint64_t highlighted_function_id,
                           uint64_t pixel_delta_in_ticks, uint64_t min_timegraph_tick,
                           uint64_t* min_ignore, uint64_t* max_ignore) {
  CHECK(min_ignore != nullptr);
  CHECK(max_ignore != nullptr);
  if (current_text_box == nullptr) return;
  const TimerInfo& current_timer_info = current_text_box->GetTimerInfo();
  if (min_tick > current_timer_info.end() || max_tick < current_timer_info.start()) return;
  if (current_timer_info.start() >= *min_ignore && current_timer_info.end() <= *max_ignore) return;
  if (!TimerFilter(current_timer_info)) return;

  UpdateDepth(current_timer_info.depth() + 1);
  double start_us = time_graph_->GetUsFromTick(current_timer_info.start());
  double start_or_prev_end_us = start_us;
  double end_us = time_graph_->GetUsFromTick(current_timer_info.end());
  double end_or_next_start_us = end_us;

  float world_timer_y = GetYFromTimer(current_timer_info);
  float box_height = GetTextBoxHeight(current_timer_info);

  // Check if the previous timer overlaps with the current one, and if so draw the overlap
  // as triangles rather than as overlapping rectangles.
  if (prev_text_box != nullptr) {
    const TimerInfo& prev_timer_info = prev_text_box->GetTimerInfo();
    // TODO(b/179985943): Turn this back into a check.
    if (prev_timer_info.start() < current_timer_info.start()) {
      // Note, that for timers that are completely inside the previous one, we will keep drawing
      // them above each other, as a proper solution would require us to keep a list of all
      // prev. intersecting timers. Further, we also compare the type, as for the Gpu timers,
      // timers of different type but same depth are drawn below each other (and thus do not
      // overlap).
      if (prev_timer_info.end() > current_timer_info.start() &&
          prev_timer_info.end() <= current_timer_info.end() &&
          prev_timer_info.type() == current_timer_info.type()) {
        start_or_prev_end_us = time_graph_->GetUsFromTick(prev_timer_info.end());
      }
    }
  }

  // Check if the next timer overlaps with the current one, and if so draw the overlap
  // as triangles rather than as overlapping rectangles.
  if (next_text_box != nullptr) {
    const TimerInfo& next_timer_info = next_text_box->GetTimerInfo();
    // TODO(b/179985943): Turn this back into a check.
    if (current_timer_info.start() < next_timer_info.start()) {
      // Note, that for timers that are completely inside the next one, we will keep drawing
      // them above each other, as a proper solution would require us to keep a list of all
      // upcoming intersecting timers. We also compare the type, as for the Gpu timers, timers
      // of different type but same depth are drawn below each other (and thus do not overlap).
      if (current_timer_info.end() > next_timer_info.start() &&
          current_timer_info.end() <= next_timer_info.end() &&
          next_timer_info.type() == current_timer_info.type()) {
        end_or_next_start_us = time_graph_->GetUsFromTick(next_timer_info.start());
      }
    }
  }

  double elapsed_us = end_us - start_us;

  // Draw the Timer's text if it is not collapsed.
  if (!is_collapsed) {
    // For overlaps, let us make the textbox just a bit wider:
    double left_overlap_width_us = start_or_prev_end_us - start_us;
    double text_x_start_us = start_or_prev_end_us - (.25 * left_overlap_width_us);
    double right_overlap_width_us = end_us - end_or_next_start_us;
    double text_x_end_us = end_or_next_start_us + (.25 * right_overlap_width_us);

    bool is_visible_width =
        (text_x_end_us - text_x_start_us) * inv_time_window * canvas->GetWidth() > 1;
    WorldXInfo world_x_info =
        ToWorldX(text_x_start_us, text_x_end_us, inv_time_window, world_start_x, world_width);

    if (is_visible_width) {
      Vec2 pos(world_x_info.world_x_start, world_timer_y);
      Vec2 size(world_x_info.world_x_width, GetTextBoxHeight(current_timer_info));
      current_text_box->SetPos(pos);
      current_text_box->SetSize(size);

      SetTimesliceText(current_timer_info, elapsed_us, world_start_x, z_offset, current_text_box);
    }
  }

  uint64_t function_id = current_timer_info.function_id();

  bool is_selected = current_text_box == selected_textbox;
  bool is_highlighted = !is_selected && function_id != orbit_grpc_protos::kInvalidFunctionId &&
                        function_id == highlighted_function_id;

  static const Color kHighlightColor(100, 181, 246, 255);
  Color color = is_highlighted ? kHighlightColor : GetTimerColor(current_timer_info, is_selected);

  bool is_visible_width = elapsed_us * inv_time_window * canvas->GetWidth() > 1;

  if (is_visible_width) {
    WorldXInfo world_x_info_left_overlap =
        ToWorldX(start_us, start_or_prev_end_us, inv_time_window, world_start_x, world_width);

    WorldXInfo world_x_info_right_overlap =
        ToWorldX(end_or_next_start_us, end_us, inv_time_window, world_start_x, world_width);

    Vec3 top_left(world_x_info_left_overlap.world_x_start, world_timer_y + box_height, z);
    Vec3 bottom_left(
        world_x_info_left_overlap.world_x_start + world_x_info_left_overlap.world_x_width,
        world_timer_y, z);
    Vec3 top_right(world_x_info_right_overlap.world_x_start, world_timer_y + box_height, z);
    Vec3 bottom_right(
        world_x_info_right_overlap.world_x_start + world_x_info_right_overlap.world_x_width,
        world_timer_y, z);
    batcher->AddShadedTrapezium(
        top_left, bottom_left, bottom_right, top_right, color,
        std::make_unique<PickingUserData>(current_text_box,
                                          [&](PickingId id) { return this->GetBoxTooltip(id); }));

  } else {
    auto user_data = std::make_unique<PickingUserData>(
        current_text_box, [&](PickingId id) { return this->GetBoxTooltip(id); });

    WorldXInfo world_x_info =
        ToWorldX(start_us, end_us, inv_time_window, world_start_x, world_width);

    Vec2 pos(world_x_info.world_x_start, world_timer_y);
    batcher->AddVerticalLine(pos, GetTextBoxHeight(current_timer_info), z, color,
                             std::move(user_data));
    // For lines, we can ignore the entire pixel into which this event
    // falls. We align this precisely on the pixel x-coordinate of the
    // current line being drawn (in ticks). If pixel_delta_in_ticks is
    // zero, we need to avoid dividing by zero, but we also wouldn't
    // gain anything here.
    if (pixel_delta_in_ticks != 0) {
      *min_ignore = min_timegraph_tick +
                    ((current_timer_info.start() - min_timegraph_tick) / pixel_delta_in_ticks) *
                        pixel_delta_in_ticks;
      *max_ignore = *min_ignore + pixel_delta_in_ticks;
    }
  }
}

void TimerTrack::UpdatePrimitives(uint64_t min_tick, uint64_t max_tick,
                                  PickingMode /*picking_mode*/, float z_offset) {
  UpdateBoxHeight();

  Batcher* batcher = &time_graph_->GetBatcher();
  GlCanvas* canvas = time_graph_->GetCanvas();

  float world_start_x = canvas->GetWorldTopLeftX();
  float world_width = canvas->GetWorldWidth();
  double inv_time_window = 1.0 / time_graph_->GetTimeWindowUs();
  bool is_collapsed = collapse_toggle_->IsCollapsed();

  float z = GlCanvas::kZValueBox + z_offset;

  std::vector<std::shared_ptr<TimerChain>> chains_by_depth = GetTimers();
  const TextBox* selected_textbox = app_->selected_text_box();
  uint64_t highlighted_function_id = app_->GetFunctionIdToHighlight();

  // We minimize overdraw when drawing lines for small events by discarding
  // events that would just draw over an already drawn line. When zoomed in
  // enough that all events are drawn as boxes, this has no effect. When zoomed
  // out, many events will be discarded quickly.
  uint64_t time_window_ns = static_cast<uint64_t>(1000 * time_graph_->GetTimeWindowUs());
  uint64_t pixel_delta_in_ticks = time_window_ns / canvas->GetWidth();
  uint64_t min_timegraph_tick = time_graph_->GetTickFromUs(time_graph_->GetMinTimeUs());

  for (auto& chain : chains_by_depth) {
    if (!chain) continue;
    TextBox* prev_text_box = nullptr;
    TextBox* current_text_box = nullptr;
    TextBox* next_text_box = nullptr;

    // We have to reset this when we go to the next depth, as otherwise we
    // would miss drawing events that should be drawn.
    uint64_t min_ignore = std::numeric_limits<uint64_t>::max();
    uint64_t max_ignore = std::numeric_limits<uint64_t>::min();
    for (TimerBlock& block : *chain) {
      if (!block.Intersects(min_tick, max_tick)) continue;

      for (size_t k = 0; k < block.size(); ++k) {
        next_text_box = &block[k];

        DrawTimer(prev_text_box, current_text_box, next_text_box, min_tick, max_tick, z_offset,
                  batcher, canvas, world_start_x, world_width, inv_time_window, is_collapsed, z,
                  selected_textbox, highlighted_function_id, pixel_delta_in_ticks,
                  min_timegraph_tick, &min_ignore, &max_ignore);

        prev_text_box = current_text_box;
        current_text_box = next_text_box;
      }
    }
    next_text_box = nullptr;
    DrawTimer(prev_text_box, current_text_box, next_text_box, min_tick, max_tick, z_offset, batcher,
              canvas, world_start_x, world_width, inv_time_window, is_collapsed, z,
              selected_textbox, highlighted_function_id, pixel_delta_in_ticks, min_timegraph_tick,
              &min_ignore, &max_ignore);
  }
}

void TimerTrack::OnTimer(const TimerInfo& timer_info) {
  if (timer_info.type() != TimerInfo::kCoreActivity) {
    UpdateDepth(timer_info.depth() + 1);
  }

  if (process_id_ == -1) {
    process_id_ = timer_info.process_id();
  }

  TextBox text_box(Vec2(0, 0), Vec2(0, 0), "");
  text_box.SetTimerInfo(timer_info);

  std::shared_ptr<TimerChain> timer_chain = timers_[timer_info.depth()];
  if (timer_chain == nullptr) {
    timer_chain = std::make_shared<TimerChain>();
    timers_[timer_info.depth()] = timer_chain;
  }
  timer_chain->push_back(text_box);
  ++num_timers_;
  if (timer_info.start() < min_time_) min_time_ = timer_info.start();
  if (timer_info.end() > max_time_) max_time_ = timer_info.end();
}

float TimerTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  bool is_collapsed = collapse_toggle_->IsCollapsed();
  uint32_t collapsed_depth = (GetNumTimers() == 0) ? 0 : 1;
  uint32_t depth = is_collapsed ? collapsed_depth : GetDepth();
  return layout.GetTextBoxHeight() * depth +
         (depth > 0 ? layout.GetSpaceBetweenTracksAndThread() : 0) + layout.GetEventTrackHeight() +
         layout.GetTrackBottomMargin();
}

std::string TimerTrack::GetTooltip() const {
  return "Shows collected samples and timings from dynamically instrumented "
         "functions";
}

std::vector<std::shared_ptr<TimerChain>> TimerTrack::GetTimers() const {
  std::vector<std::shared_ptr<TimerChain>> timers;
  absl::MutexLock lock(&mutex_);
  for (auto& pair : timers_) {
    timers.push_back(pair.second);
  }
  return timers;
}

const TextBox* TimerTrack::GetFirstAfterTime(uint64_t time, uint32_t depth) const {
  std::shared_ptr<TimerChain> chain = GetTimers(depth);
  if (chain == nullptr) return nullptr;

  // TODO: do better than linear search...
  for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
    for (size_t k = 0; k < it->size(); ++k) {
      const TextBox& text_box = (*it)[k];
      if (text_box.GetTimerInfo().start() > time) {
        return &text_box;
      }
    }
  }
  return nullptr;
}

const TextBox* TimerTrack::GetFirstBeforeTime(uint64_t time, uint32_t depth) const {
  std::shared_ptr<TimerChain> chain = GetTimers(depth);
  if (chain == nullptr) return nullptr;

  const TextBox* text_box = nullptr;

  // TODO: do better than linear search...
  for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
    for (size_t k = 0; k < it->size(); ++k) {
      const TextBox& box = (*it)[k];
      if (box.GetTimerInfo().start() > time) {
        return text_box;
      }
      text_box = &box;
    }
  }

  return nullptr;
}

std::shared_ptr<TimerChain> TimerTrack::GetTimers(uint32_t depth) const {
  absl::MutexLock lock(&mutex_);
  auto it = timers_.find(depth);
  if (it != timers_.end()) return it->second;
  return nullptr;
}

const TextBox* TimerTrack::GetUp(const TextBox* text_box) const {
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  return GetFirstBeforeTime(timer_info.start(), timer_info.depth() - 1);
}

const TextBox* TimerTrack::GetDown(const TextBox* text_box) const {
  const TimerInfo& timer_info = text_box->GetTimerInfo();
  return GetFirstAfterTime(timer_info.start(), timer_info.depth() + 1);
}

std::vector<std::shared_ptr<TimerChain>> TimerTrack::GetAllChains() const {
  std::vector<std::shared_ptr<TimerChain>> chains;
  for (const auto& pair : timers_) {
    chains.push_back(pair.second);
  }
  return chains;
}

std::vector<std::shared_ptr<TimerChain>> TimerTrack::GetAllSerializableChains() const {
  return GetAllChains();
}

bool TimerTrack::IsEmpty() const { return GetNumTimers() == 0; }

std::string TimerTrack::GetBoxTooltip(PickingId /*id*/) const { return ""; }

float TimerTrack::GetHeaderHeight() const {
  const TimeGraphLayout& layout = time_graph_->GetLayout();
  return layout.GetEventTrackHeight();
}
