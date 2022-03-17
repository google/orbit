// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimelineUi.h"

#include <absl/flags/flag.h>

#include "AccessibleTimeline.h"
#include "ClientFlags/ClientFlags.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GlCanvas.h"
#include "TimelineTicks.h"

namespace orbit_gl {

constexpr float kLabelMarginRight = 2.f;
constexpr float kLabelMarginLeft = 4.f;

void TimelineUi::RenderLines(Batcher& batcher, uint64_t min_timestamp_ns,
                             uint64_t max_timestamp_ns) const {
  const Color kMajorTickColor(255, 254, 253, 255);
  const Color kMinorTickColor(63, 62, 63, 255);

  for (auto& [tick_type, tick_ns] :
       timeline_ticks_.GetAllTicks(min_timestamp_ns, max_timestamp_ns)) {
    float world_x = timeline_info_interface_->GetWorldFromUs(
        tick_ns / static_cast<double>(kNanosecondsPerMicrosecond));
    int screen_x = viewport_->WorldToScreen(Vec2(world_x, 0))[0];
    batcher.AddVerticalLine(
        Vec2(screen_x, GetPos()[1]), GetHeightWithoutMargin(), GlCanvas::kZValueTimeBarBg,
        tick_type == TimelineTicks::TickType::kMajorTick ? kMajorTickColor : kMinorTickColor);
  }
}

void TimelineUi::RenderLabels(Batcher& batcher, TextRenderer& text_renderer,
                              uint64_t min_timestamp_ns, uint64_t max_timestamp_ns) const {
  const float kPixelMargin = 1;

  std::vector<uint64_t> all_major_ticks =
      timeline_ticks_.GetMajorTicks(min_timestamp_ns, max_timestamp_ns);
  // The label of the previous major tick could be also partially visible.
  std::optional<uint64_t> previous_major_tick =
      timeline_ticks_.GetPreviousMajorTick(min_timestamp_ns, max_timestamp_ns);
  if (previous_major_tick.has_value()) {
    all_major_ticks.insert(all_major_ticks.begin(), previous_major_tick.value());
  }

  uint32_t number_of_decimal_places_needed = 1;
  for (uint64_t tick : all_major_ticks) {
    number_of_decimal_places_needed = std::max(
        number_of_decimal_places_needed, timeline_ticks_.GetTimestampNumDigitsPrecision(tick));
  }

  for (uint64_t tick_ns : GetTicksForNonOverlappingLabels(
           text_renderer, all_major_ticks, kLabelMarginLeft + kLabelMarginRight + kPixelMargin,
           number_of_decimal_places_needed)) {
    RenderLabel(batcher, text_renderer, tick_ns, number_of_decimal_places_needed,
                GlCanvas::kZValueTimeBar, GlCanvas::kTimeBarBackgroundColor);
  }
}

void TimelineUi::RenderMargin(Batcher& batcher) const {
  Vec2 margin_pos = Vec2(GetPos()[0], GetPos()[1] + GetHeightWithoutMargin());
  Vec2 margin_size = Vec2(GetSize()[0], GetMarginHeight());
  batcher.AddBox(Box(margin_pos, margin_size, GlCanvas::kZValueOverlay),
                 GlCanvas::kBackgroundColor);
}

void TimelineUi::RenderBackground(Batcher& batcher) const {
  Box background_box(GetPos(), Vec2(GetWidth(), GetHeightWithoutMargin()),
                     GlCanvas::kZValueTimeBarBg);
  batcher.AddBox(background_box, GlCanvas::kTimeBarBackgroundColor);
}

void TimelineUi::RenderLabel(Batcher& batcher, TextRenderer& text_renderer, uint64_t tick_ns,
                             uint32_t number_of_decimal_places_needed, float label_z,
                             const Color background_color) const {
  std::string label = GetLabel(tick_ns, number_of_decimal_places_needed);
  float world_x = GetTickWorldXPos(tick_ns);
  Vec2 pos, size;
  float label_middle_y = GetPos()[1] + GetHeightWithoutMargin() / 2.f;
  text_renderer.AddText(label.c_str(), world_x + kLabelMarginLeft, label_middle_y, label_z,
                        /*text_formatting=*/
                        {layout_->GetFontSize(), Color(255, 255, 255, 255), -1.f,
                         TextRenderer::HAlign::Left, TextRenderer::VAlign::Middle},
                        /*out_text_pos=*/&pos, /*out_text_size=*/&size);

  // Box behind the label to hide the ticks behind it.
  const float kBackgroundBoxVerticalMargin = 4.f;
  size[0] += kLabelMarginRight;
  pos[1] = pos[1] - kBackgroundBoxVerticalMargin;
  size[1] = size[1] + 2 * kBackgroundBoxVerticalMargin;
  Box background_box(pos, size, label_z);
  batcher.AddBox(background_box, background_color);
}

std::string TimelineUi::GetLabel(uint64_t tick_ns, uint32_t number_of_decimal_places_needed) const {
  // TODO(http://b/170712621): Remove this flag when we decide which timestamp format we will use.
  if (absl::GetFlag(FLAGS_iso_timestamps)) {
    return orbit_display_formats::GetDisplayISOTimestamp(
        absl::Nanoseconds(tick_ns), number_of_decimal_places_needed,
        absl::Nanoseconds(timeline_info_interface_->GetCaptureTimeSpanNs()));
  }
  return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(tick_ns));
}

float TimelineUi::GetTickWorldXPos(uint64_t tick_ns) const {
  return timeline_info_interface_->GetWorldFromUs(tick_ns /
                                                  static_cast<double>(kNanosecondsPerMicrosecond));
}

std::vector<uint64_t> TimelineUi::GetTicksForNonOverlappingLabels(
    TextRenderer& text_renderer, const std::vector<uint64_t>& all_major_ticks,
    float horizontal_margin, uint32_t number_of_decimal_places) const {
  if (all_major_ticks.size() <= 1) return all_major_ticks;
  uint64_t ns_between_major_ticks = all_major_ticks[1] - all_major_ticks[0];

  // In general, all major tick labels will fit in screen. In extreme cases with long labels
  // and small screens, we will skip the same number of labels in between visible ones for
  // consistency.
  int num_consecutive_skipped_labels = 0;
  std::vector<uint64_t> visible_labels = all_major_ticks;
  while (WillLabelsOverlap(text_renderer, visible_labels, horizontal_margin,
                           number_of_decimal_places)) {
    visible_labels.clear();
    num_consecutive_skipped_labels++;
    for (uint64_t tick : all_major_ticks) {
      // There is an invariant that all timestamps will be divisible by the space between them.
      // We will choose the visible labels without breaking this invariant. In this way, visible
      // labels won't change after horizontal scrolling.
      if (tick % ((num_consecutive_skipped_labels + 1) * ns_between_major_ticks) == 0) {
        visible_labels.push_back(tick);
      }
    }
  }
  return visible_labels;
}

bool TimelineUi::WillLabelsOverlap(TextRenderer& text_renderer,
                                   const std::vector<uint64_t>& tick_list, float horizontal_margin,
                                   uint32_t number_of_decimal_places) const {
  if (tick_list.size() <= 1) return false;
  float distance_between_labels = GetTickWorldXPos(tick_list[1]) - GetTickWorldXPos(tick_list[0]);
  for (auto tick_ns : tick_list) {
    float label_width = text_renderer.GetStringWidth(
        GetLabel(tick_ns, number_of_decimal_places).c_str(), layout_->GetFontSize());
    if (distance_between_labels < horizontal_margin + label_width) {
      return true;
    }
  }
  return false;
}

void TimelineUi::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                    uint64_t min_tick, uint64_t max_tick,
                                    PickingMode picking_mode) {
  CaptureViewElement::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);
  RenderBackground(batcher);
  if (timeline_info_interface_->GetTimeWindowUs() <= 0) return;

  uint64_t min_timestamp_ns = timeline_info_interface_->GetNsSinceStart(min_tick);
  uint64_t max_timestamp_ns = timeline_info_interface_->GetNsSinceStart(max_tick);
  RenderLines(batcher, min_timestamp_ns, max_timestamp_ns);
  RenderLabels(batcher, text_renderer, min_timestamp_ns, max_timestamp_ns);
  // TODO(http://b/217719000): Hack needed to not draw tracks on timeline's margin.
  RenderMargin(batcher);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> TimelineUi::CreateAccessibleInterface() {
  return std::make_unique<AccessibleTimeline>(this);
}

}  // namespace orbit_gl