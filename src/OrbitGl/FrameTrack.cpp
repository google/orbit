// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FrameTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <algorithm>
#include <utility>

#include "Batcher.h"
#include "ClientData/FunctionUtils.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "TriangleToggle.h"

using orbit_client_model::CaptureData;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::InstrumentedFunction;

namespace {
constexpr const double kHeightCapAverageMultipleDouble = 6.0;
constexpr const uint64_t kHeightCapAverageMultipleUint64 = 6;
constexpr const float kBoxHeightMultiplier = 3.f;
}  // namespace

float FrameTrack::GetMaximumScaleFactor() const {
  if (stats_.average_time_ns() == 0) {
    return 0.f;
  }
  // Compute the scale factor in double first as we convert time values in nanoseconds to
  // floating point. Single-precision floating point (float type) can only exactly
  // represent all integer values up to 2^24 - 1, which given the ns time unit is fairly
  // small (only ~16ms).
  double scale_factor =
      static_cast<double>(stats_.max_ns()) / static_cast<double>(stats_.average_time_ns());
  scale_factor = std::min(scale_factor, kHeightCapAverageMultipleDouble);
  return static_cast<float>(scale_factor);
}

float FrameTrack::GetMaximumBoxHeight() const {
  const bool is_collapsed = collapse_toggle_->IsCollapsed();
  float scale_factor = GetMaximumScaleFactor();
  const float box_height_normalizer = is_collapsed ? scale_factor : 1.f;
  if (scale_factor == 0.f) {
    return 0.f;
  }
  return scale_factor * box_height_ / box_height_normalizer;
}

float FrameTrack::GetAverageBoxHeight() const {
  const bool is_collapsed = collapse_toggle_->IsCollapsed();
  float scale_factor = GetMaximumScaleFactor();
  const float box_height_normalizer = is_collapsed ? scale_factor : 1.f;
  if (scale_factor == 0.f) {
    return 0.f;
  }
  return box_height_ / box_height_normalizer;
}

FrameTrack::FrameTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                       InstrumentedFunction function, OrbitApp* app,
                       const CaptureData* capture_data, uint32_t indentation_level)
    : TimerTrack(parent, time_graph, viewport, layout, app, capture_data, indentation_level),
      function_(std::move(function)) {
  // TODO(b/169554463): Support manual instrumentation.
  std::string name = absl::StrFormat("Frame track based on %s", function_.function_name());
  SetName(name);
  SetLabel(name);

  // Frame tracks are collapsed by default.
  collapse_toggle_->SetState(TriangleToggle::State::kCollapsed,
                             TriangleToggle::InitialStateUpdate::kReplaceInitialState);
}

float FrameTrack::GetHeaderHeight() const { return layout_->GetTrackTabHeight(); }

float FrameTrack::GetHeight() const {
  return GetHeaderHeight() + GetMaximumBoxHeight() + layout_->GetTrackBottomMargin();
}

float FrameTrack::GetYFromTimer(const TimerInfo& /*timer_info*/) const {
  return pos_[1] - GetHeaderHeight() - GetMaximumBoxHeight();
}

float FrameTrack::GetTextBoxHeight(const TimerInfo& timer_info) const {
  uint64_t timer_duration_ns = timer_info.end() - timer_info.start();
  if (stats_.average_time_ns() == 0) {
    return 0.f;
  }
  double ratio =
      static_cast<double>(timer_duration_ns) / static_cast<double>(stats_.average_time_ns());
  ratio = std::min(ratio, kHeightCapAverageMultipleDouble);
  return static_cast<float>(ratio) * GetAverageBoxHeight();
}

Color FrameTrack::GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                bool /*is_selected*/, bool /*is_highlighted*/) const {
  Vec4 min_color(76.f, 175.f, 80.f, 255.f);
  Vec4 max_color(63.f, 81.f, 181.f, 255.f);
  Vec4 warn_color(244.f, 67.f, 54.f, 255.f);

  Vec4 color;
  uint64_t timer_duration_ns = timer_info.end() - timer_info.start();

  // A note on overflows here and below: The times in uint64_t represent durations of events
  // in nanoseconds. This means the maximum duration is ~600 years. That is, multiplying by values
  // in the single digits (and even much higher) as done here does not cause any issues.
  if (timer_duration_ns >= kHeightCapAverageMultipleUint64 * stats_.average_time_ns()) {
    color = warn_color;
  } else if (stats_.average_time_ns() > 0) {
    // We are interpolating colors between min_color and max_color based on how much
    // the duration (timer_duration_ns) differs from the average. This is asymmetric on
    // purpose, as frames that are shorter than the average time are fine and do not need
    // to stand out differently from the average. Durations below lower_bound and
    // durations above upper_bound are drawn with min_color and max_color, respectively.
    uint64_t lower_bound = 4 * stats_.average_time_ns() / 5;
    uint64_t upper_bound = 8 * stats_.average_time_ns() / 5;

    timer_duration_ns = std::min(std::max(timer_duration_ns, lower_bound), upper_bound);
    float fraction = static_cast<float>(timer_duration_ns - lower_bound) /
                     static_cast<float>(upper_bound - lower_bound);
    color = fraction * max_color + (1.f - fraction) * min_color;
  } else {
    // All times are zero here. Just draw the minimum color, this doesn't make a difference
    // as in this case all frame timeslices are not visible anyway.
    color = min_color;
  }

  if (timer_info.user_data_key() % 2 == 0) {
    color = 0.8f * color;
  }
  return Color(static_cast<uint8_t>(color[0]), static_cast<uint8_t>(color[1]),
               static_cast<uint8_t>(color[2]), static_cast<uint8_t>(color[3]));
}

void FrameTrack::OnTimer(const TimerInfo& timer_info) {
  uint64_t duration_ns = timer_info.end() - timer_info.start();
  stats_.set_count(stats_.count() + 1);
  stats_.set_total_time_ns(stats_.total_time_ns() + duration_ns);
  stats_.set_average_time_ns(stats_.total_time_ns() / stats_.count());

  if (duration_ns > stats_.max_ns()) {
    stats_.set_max_ns(duration_ns);
  }
  if (stats_.min_ns() == 0 || duration_ns < stats_.min_ns()) {
    stats_.set_min_ns(duration_ns);
  }

  TimerTrack::OnTimer(timer_info);
}

void FrameTrack::SetTimesliceText(const TimerInfo& timer_info, float min_x, float z_offset,
                                  TextBox* text_box) {
  if (text_box->GetText().empty()) {
    std::string time = orbit_display_formats::GetDisplayTime(
        absl::Nanoseconds(timer_info.end() - timer_info.start()));
    text_box->SetElapsedTimeTextLength(time.length());

    std::string text = absl::StrFormat("Frame #%u: %s", timer_info.user_data_key(), time.c_str());
    text_box->SetText(text);
  }

  const Color kTextWhite(255, 255, 255, 255);
  const Vec2& box_pos = text_box->GetPos();
  const Vec2& box_size = text_box->GetSize();
  float pos_x = std::max(box_pos[0], min_x);
  float max_size = box_pos[0] + box_size[0] - pos_x;
  text_renderer_->AddTextTrailingCharsPrioritized(
      text_box->GetText().c_str(), pos_x, text_box->GetPos()[1] + layout_->GetTextOffset(),
      GlCanvas::kZValueBox + z_offset, kTextWhite, text_box->GetElapsedTimeTextLength(),
      layout_->CalculateZoomedFontSize(), max_size);
}

std::string FrameTrack::GetTooltip() const {
  const std::string& function_name = function_.function_name();
  return absl::StrFormat(
      "<b>Frame track</b><br/>"
      "<i>Shows frame timings based on subsequent callst to %s. "
      "<br/><br/>"
      "<b>Coloring</b>: Colors are interpolated between green (low frame time) and blue "
      "(high frame time). The height of frames that strongly exceed average time are capped "
      "at %u times the average frame time for drawing purposes. These are drawn in red."
      "<br/><br/>"
      "<b>Note</b>: Timings are not the runtime of the function, but the difference "
      "between start timestamps of subsequent calls."
      "<br/><br/>"
      "<b>Frame marker function:</b> %s<br/>"
      "<b>Module:</b> %s<br/>"
      "<b>Frame count:</b> %u<br/>"
      "<b>Maximum frame time:</b> %s<br/>"
      "<b>Minimum frame time:</b> %s<br/>"
      "<b>Average frame time:</b> %s<br/>",
      function_name, kHeightCapAverageMultipleUint64, function_name,
      orbit_client_data::function_utils::GetLoadedModuleNameByPath(function_.file_path()),
      stats_.count(), orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats_.max_ns())),
      orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats_.min_ns())),
      orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats_.average_time_ns())));
}

std::string FrameTrack::GetBoxTooltip(const Batcher& batcher, PickingId id) const {
  const TextBox* text_box = batcher.GetTextBox(id);
  if (!text_box) {
    return "";
  }
  // TODO(b/169554463): Support manual instrumentation.
  const std::string& function_name = function_.function_name();

  return absl::StrFormat(
      "<b>Frame time</b><br/>"
      "<i>Frame time based on two subsequent calls to %s. Height and width of the box are "
      "proportional to time where height is capped at %u times the average time. Timeslices with "
      "capped height are shown in red.</i>"
      "<br/><br/>"
      "<b>Frame marker function:</b> %s<br/>"
      "<b>Module:</b> %s<br/>"
      "<b>Frame:</b> #%u<br/>"
      "<b>Frame time:</b> %s",
      function_name, kHeightCapAverageMultipleUint64, function_name,
      orbit_client_data::function_utils::GetLoadedModuleNameByPath(function_.file_path()),
      text_box->GetTimerInfo().user_data_key(),
      orbit_display_formats::GetDisplayTime(
          TicksToDuration(text_box->GetTimerInfo().start(), text_box->GetTimerInfo().end())));
}

void FrameTrack::Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
                      PickingMode picking_mode, float z_offset) {
  TimerTrack::Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode, z_offset);

  const Color kWhiteColor(255, 255, 255, 255);
  const Color kBlackColor(0, 0, 0, 255);

  float y = pos_[1] - GetHeaderHeight() - GetMaximumBoxHeight() + GetAverageBoxHeight();
  float x = pos_[0];
  Vec2 from(x, y);
  Vec2 to(x + size_[0], y);
  float text_z = GlCanvas::kZValueTrackText + z_offset;

  std::string avg_time =
      orbit_display_formats::GetDisplayTime(absl::Nanoseconds(stats_.average_time_ns()));
  std::string label = absl::StrFormat("Avg: %s", avg_time);
  uint32_t font_size = layout_->CalculateZoomedFontSize();
  float string_width = text_renderer.GetStringWidth(label.c_str(), font_size);
  Vec2 white_text_box_size(string_width, layout_->GetTextBoxHeight());
  Vec2 white_text_box_position(pos_[0] + layout_->GetRightMargin(),
                               y - layout_->GetTextBoxHeight() / 2.f);

  batcher.AddLine(from, from + Vec2(layout_->GetRightMargin() / 2.f, 0), text_z, kWhiteColor);
  batcher.AddLine(Vec2(white_text_box_position[0] + white_text_box_size[0], y), to, text_z,
                  kWhiteColor);

  text_renderer.AddText(label.c_str(), white_text_box_position[0],
                        white_text_box_position[1] + layout_->GetTextOffset(), text_z, kWhiteColor,
                        font_size, white_text_box_size[0]);
}

void FrameTrack::UpdateBoxHeight() {
  box_height_ = kBoxHeightMultiplier * layout_->GetTextBoxHeight();
}

std::vector<std::shared_ptr<TimerChain>> FrameTrack::GetAllSerializableChains() const {
  // Frametracks are just displaying existing data in a different way.
  // We don't want to write out all the timers of that track. Frame tracks are serialized
  // by storing the function ids of all functions with frame tracks enabled.
  return {};
}
