// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FrameTrack.h"

#include "App.h"
#include "OrbitClientData/FunctionUtils.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

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
  } else {
    return scale_factor * box_height_ / box_height_normalizer;
  }
}

float FrameTrack::GetAverageBoxHeight() const {
  const bool is_collapsed = collapse_toggle_->IsCollapsed();
  float scale_factor = GetMaximumScaleFactor();
  const float box_height_normalizer = is_collapsed ? scale_factor : 1.f;
  if (scale_factor == 0.f) {
    return 0.f;
  } else {
    return box_height_ / box_height_normalizer;
  }
}

FrameTrack::FrameTrack(TimeGraph* time_graph, const FunctionInfo& function)
    : TimerTrack(time_graph), function_(function) {
  // TODO(b/169554463): Support manual instrumentation.
  std::string function_name = FunctionUtils::GetDisplayName(function_);
  std::string name = absl::StrFormat("Frame track based on %s", function_name);
  SetName(name);
  SetLabel(name);

  // Frame tracks are collapsed by default.
  collapse_toggle_->SetState(TriangleToggle::State::kCollapsed,
                             TriangleToggle::InitialStateUpdate::kReplaceInitialState);
}

float FrameTrack::GetHeaderHeight() const { return 0.f; }

float FrameTrack::GetHeight() const {
  TimeGraphLayout& layout = time_graph_->GetLayout();
  return GetMaximumBoxHeight() + layout.GetTrackBottomMargin();
}

float FrameTrack::GetYFromDepth(uint32_t /*depth*/) const {
  return pos_[1] - GetMaximumBoxHeight();
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
                                bool /*is_selected*/) const {
  Vec4 min_color(76.f, 175.f, 80.f, 255.f);
  Vec4 max_color(63.f, 81.f, 181.f, 255.f);
  Vec4 warn_color(244.f, 67.f, 54.f, 255.f);

  Vec4 color;
  uint64_t timer_duration_ns = timer_info.end() - timer_info.start();

  if (timer_duration_ns >= kHeightCapAverageMultipleUint64 * stats_.average_time_ns()) {
    color = warn_color;
  } else {
    uint64_t lower_bound = stats_.min_ns();
    uint64_t upper_bound =
        std::min(kHeightCapAverageMultipleUint64 * stats_.average_time_ns(), stats_.max_ns());
    if (upper_bound == lower_bound) {
      // This implies that min_ns == max_ns and thus all times are the same. We can just render
      // everything using min_color.
      color = min_color;
    }
    timer_duration_ns = std::min(timer_duration_ns, upper_bound);
    float fraction = static_cast<float>(timer_duration_ns - lower_bound) /
                     static_cast<float>(upper_bound - lower_bound);
    color = fraction * max_color + (1.f - fraction) * min_color;
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

void FrameTrack::SetTimesliceText(const TimerInfo& timer_info, double elapsed_us, float min_x,
                                  float z_offset, TextBox* text_box) {
  TimeGraphLayout layout = time_graph_->GetLayout();
  if (text_box->GetText().empty()) {
    std::string time = GetPrettyTime(absl::Microseconds(elapsed_us));
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
      text_box->GetText().c_str(), pos_x, text_box->GetPos()[1] + layout.GetTextOffset(),
      GlCanvas::kZValueBox + z_offset, kTextWhite, text_box->GetElapsedTimeTextLength(),
      time_graph_->CalculateZoomedFontSize(), max_size);
}

std::string FrameTrack::GetTooltip() const {
  std::string function_name = FunctionUtils::GetDisplayName(function_);
  return absl::StrFormat(
      "<b>Frame track</b><br/>"
      "<i>Shows frame timings based on subsequent callst to %s. "
      "<br/><br/>"
      "<b>Coloring</b>: Colors are interpolated between green (minimum frame time) and blue "
      "(maximum frame time). The height of frames that strongly exceed average time are capped at "
      "%u times the average frame time for drawing purposes. These are drawn in red."
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
      FunctionUtils::GetLoadedModuleName(function_), stats_.count(),
      GetPrettyTime(absl::Nanoseconds(stats_.max_ns())),
      GetPrettyTime(absl::Nanoseconds(stats_.min_ns())),
      GetPrettyTime(absl::Nanoseconds(stats_.average_time_ns())));
}

std::string FrameTrack::GetBoxTooltip(PickingId id) const {
  const TextBox* text_box = time_graph_->GetBatcher().GetTextBox(id);
  if (!text_box) {
    return "";
  }
  // TODO(b/169554463): Support manual instrumentation.
  std::string function_name = FunctionUtils::GetDisplayName(function_);

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
      FunctionUtils::GetLoadedModuleName(function_), text_box->GetTimerInfo().user_data_key(),
      GetPrettyTime(
          TicksToDuration(text_box->GetTimerInfo().start(), text_box->GetTimerInfo().end())));
}

void FrameTrack::Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset) {
  TimerTrack::Draw(canvas, picking_mode, z_offset);

  const Color kWhiteColor(255, 255, 255, 255);
  const Color kBlackColor(0, 0, 0, 255);

  Batcher* batcher = &time_graph_->GetBatcher();
  float y = pos_[1] - GetMaximumBoxHeight() + GetAverageBoxHeight();
  float x = pos_[0];
  Vec2 from(x, y);
  Vec2 to(x + size_[0], y);
  float ui_z = GlCanvas::kZValueUi;

  const TimeGraphLayout& layout = time_graph_->GetLayout();
  std::string avg_time = GetPrettyTime(absl::Nanoseconds(stats_.average_time_ns()));
  std::string label = absl::StrFormat("Avg: %s", avg_time);
  uint32_t font_size = time_graph_->CalculateZoomedFontSize();
  float string_width = canvas->GetTextRenderer().GetStringWidth(label.c_str(), font_size);
  Vec2 white_text_box_size(string_width, layout.GetTextBoxHeight());
  Vec2 white_text_box_position(pos_[0] + layout.GetRightMargin(),
                               y - layout.GetTextBoxHeight() / 2.f);

  batcher->AddLine(from, from + Vec2(layout.GetRightMargin() / 2.f, 0), ui_z, kWhiteColor);
  batcher->AddLine(Vec2(white_text_box_position[0] + white_text_box_size[0], y), to, ui_z,
                   kWhiteColor);

  canvas->GetTextRenderer().AddText(label.c_str(), white_text_box_position[0],
                                    white_text_box_position[1] + layout.GetTextOffset(),
                                    GlCanvas::kZValueTextUi, kWhiteColor, font_size,
                                    white_text_box_size[0]);
}

void FrameTrack::UpdateBoxHeight() {
  box_height_ = kBoxHeightMultiplier * time_graph_->GetLayout().GetTextBoxHeight();
}

std::vector<std::shared_ptr<TimerChain>> FrameTrack::GetAllSerializableChains() {
  // Frametracks are just displaying existing data in a different way.
  // We don't want to write out all the timers of that track.
  // TODO(b/171026228): However, we should serialize them in some form.
  return {};
}
