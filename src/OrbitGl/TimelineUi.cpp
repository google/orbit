// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TimelineUi.h"

#include <GteVector.h>
#include <absl/flags/flag.h>
#include <absl/time/time.h>
#include <absl/types/span.h>

#include <algorithm>
#include <optional>

#include "ClientFlags/ClientFlags.h"
#include "DisplayFormats/DisplayFormats.h"
#include "OrbitGl/AccessibleCaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/TimelineTicks.h"

namespace orbit_gl {

constexpr float kLabelsPadding = 4.f;
constexpr float kPixelsBetweenMajorTicksAndLabels = 1.f;
const Color kBackgroundColorSpecialLabels(68, 67, 69, 255);

CaptureViewElement::EventResult TimelineUi::OnMouseWheel(const Vec2& mouse_pos, int delta,
                                                         const ModifierKeys& /*modifiers*/) {
  if (delta == 0) return EventResult::kIgnored;

  double mouse_ratio = (mouse_pos[0] - GetPos()[0]) / GetWidth();
  timeline_info_interface_->ZoomTime(delta, mouse_ratio);

  return EventResult::kHandled;
}

void TimelineUi::RenderLines(PrimitiveAssembler& primitive_assembler, uint64_t min_timestamp_ns,
                             uint64_t max_timestamp_ns) const {
  ClosedInterval<float> timeline_x_visible_range{GetPos()[0], GetPos()[0] + GetSize()[0]};
  for (auto& [tick_type, tick_ns] :
       timeline_ticks_.GetAllTicks(min_timestamp_ns, max_timestamp_ns)) {
    float world_x = timeline_info_interface_->GetWorldFromUs(
        tick_ns / static_cast<double>(kNanosecondsPerMicrosecond));
    if (timeline_x_visible_range.Contains(world_x)) {
      int screen_x = viewport_->WorldToScreen(Vec2(world_x, 0))[0];
      primitive_assembler.AddVerticalLine(
          Vec2(screen_x, GetPos()[1]), GetHeight(), GlCanvas::kZValueTimeBar,
          tick_type == TimelineTicks::TickType::kMajorTick ? kTimelineMajorTickColor
                                                           : kTimelineMinorTickColor);
    }
  }
}

void TimelineUi::RenderLabels(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                              uint64_t min_timestamp_ns, uint64_t max_timestamp_ns) const {
  std::vector<uint64_t> all_major_ticks =
      timeline_ticks_.GetMajorTicks(min_timestamp_ns, max_timestamp_ns);
  // The label of the previous major tick could be also partially visible.
  std::optional<uint64_t> previous_major_tick =
      timeline_ticks_.GetPreviousMajorTick(min_timestamp_ns, max_timestamp_ns);
  if (previous_major_tick.has_value()) {
    all_major_ticks.insert(all_major_ticks.begin(), previous_major_tick.value());
  }

  for (uint64_t tick_ns : GetTicksForNonOverlappingLabels(text_renderer, all_major_ticks)) {
    RenderLabel(primitive_assembler, text_renderer, tick_ns, GetNumDecimalsInLabels(),
                GlCanvas::kTimeBarBackgroundColor);
  }
}

void TimelineUi::RenderBackground(PrimitiveAssembler& primitive_assembler) const {
  Quad background_box = MakeBox(GetPos(), GetSize());
  primitive_assembler.AddBox(background_box, GlCanvas::kZValueTimeBar,
                             GlCanvas::kTimeBarBackgroundColor);
}

void TimelineUi::RenderLabel(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                             uint64_t tick_ns, uint32_t number_of_decimal_places,
                             const Color& background_color, bool is_mouse_label) const {
  float label_z =
      is_mouse_label ? GlCanvas::kZValueTimeBarMouseLabel : GlCanvas::kZValueTimeBarLabel;

  // We add a pixel separation between the label and the major ticks so they don't intersect.
  float label_extra_margin = is_mouse_label ? 0.f : kPixelsBetweenMajorTicksAndLabels;

  std::string label = GetLabel(tick_ns, number_of_decimal_places);
  float world_x = GetTickWorldXPos(tick_ns);
  float label_width = text_renderer.GetStringWidth(label.c_str(), layout_->GetFontSize());
  // Check that the label is visible or partially visible.
  if (ClosedInterval<float> label_x_interval{world_x, world_x + label_width};
      !label_x_interval.Intersects(ClosedInterval<float>{GetPos()[0], GetPos()[0] + GetWidth()})) {
    return;
  }

  float label_start_x = world_x + kLabelsPadding + label_extra_margin;

  // Flip mouse label if it doesn't fit on the right and rather it fits all the way on the left.
  if (is_mouse_label) {
    float right_margin_x = GetPos()[0] + GetWidth();
    if (label_start_x + label_width >= right_margin_x) {
      // If there is no enough space on the right, check whether there is enough space on the left
      if (float label_at_left_start_x = world_x - kLabelsPadding - label_extra_margin - label_width;
          label_at_left_start_x >= GetPos()[0]) {
        label_start_x = label_at_left_start_x;
      }
    }
  }

  Vec2 pos, size;
  float label_middle_y = GetPos()[1] + GetHeight() / 2.f;
  text_renderer.AddText(label.c_str(), label_start_x, label_middle_y, label_z, /*text_formatting=*/
                        {layout_->GetFontSize(), Color(255, 255, 255, 255), -1.f,
                         TextRenderer::HAlign::Left, TextRenderer::VAlign::Middle},
                        /*out_text_pos=*/&pos, /*out_text_size=*/&size);

  // Box behind the label to hide the ticks behind it.
  size[0] += 2.f * kLabelsPadding;
  size[1] += 2.f * kLabelsPadding;
  pos[0] -= kLabelsPadding;
  pos[1] -= kLabelsPadding;
  Quad background_box = MakeBox(pos, size);
  primitive_assembler.AddBox(background_box, label_z, background_color);
}

void TimelineUi::RenderMouseLabel(PrimitiveAssembler& primitive_assembler,
                                  TextRenderer& text_renderer, uint64_t mouse_tick_ns) const {
  // The label in mouse position has 2 more digits of precision than other labels.
  constexpr uint32_t kNumAdditionalDecimalDigits = 2;
  constexpr uint32_t kMaxNumberOfDecimalDigits = 9;
  uint32_t num_decimal_places_mouse_label =
      std::min(kMaxNumberOfDecimalDigits, GetNumDecimalsInLabels() + kNumAdditionalDecimalDigits);

  RenderLabel(primitive_assembler, text_renderer, mouse_tick_ns, num_decimal_places_mouse_label,
              kBackgroundColorSpecialLabels, /*is_mouse_label=*/true);
}

std::string TimelineUi::GetLabel(uint64_t tick_ns, uint32_t number_of_decimal_places) const {
  // TODO(http://b/170712621): Remove this flag when we decide which timestamp format we will use.
  if (absl::GetFlag(FLAGS_iso_timestamps)) {
    return orbit_display_formats::GetDisplayISOTimestamp(
        absl::Nanoseconds(tick_ns), number_of_decimal_places,
        absl::Nanoseconds(timeline_info_interface_->GetCaptureTimeSpanNs()));
  }
  return orbit_display_formats::GetDisplayTime(absl::Nanoseconds(tick_ns));
}

float TimelineUi::GetTickWorldXPos(uint64_t tick_ns) const {
  return timeline_info_interface_->GetWorldFromUs(tick_ns /
                                                  static_cast<double>(kNanosecondsPerMicrosecond));
}

std::vector<uint64_t> TimelineUi::GetTicksForNonOverlappingLabels(
    TextRenderer& text_renderer, absl::Span<const uint64_t> all_major_ticks) const {
  if (all_major_ticks.size() <= 1) return {all_major_ticks.begin(), all_major_ticks.end()};
  uint64_t ns_between_major_ticks = all_major_ticks[1] - all_major_ticks[0];

  // In general, all major tick labels will fit in screen. In extreme cases with long labels
  // and small screens, we will skip the same number of labels in between visible ones for
  // consistency.
  int num_consecutive_skipped_labels = 0;
  std::vector<uint64_t> visible_labels{all_major_ticks.begin(), all_major_ticks.end()};
  while (WillLabelsOverlap(text_renderer, visible_labels)) {
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
                                   absl::Span<const uint64_t> tick_list) const {
  if (tick_list.size() <= 1) return false;
  float distance_between_labels = GetTickWorldXPos(tick_list[1]) - GetTickWorldXPos(tick_list[0]);
  for (auto tick_ns : tick_list) {
    float label_width = text_renderer.GetStringWidth(
        GetLabel(tick_ns, GetNumDecimalsInLabels()).c_str(), layout_->GetFontSize());
    if (distance_between_labels <
        2.f * kLabelsPadding + kPixelsBetweenMajorTicksAndLabels + label_width) {
      return true;
    }
  }
  return false;
}

void TimelineUi::UpdateNumDecimalsInLabels(uint64_t min_timestamp_ns, uint64_t max_timestamp_ns) {
  constexpr uint32_t kMinDecimalsInLabels = 1;
  num_decimals_in_labels_ = kMinDecimalsInLabels;

  for (uint64_t tick : timeline_ticks_.GetMajorTicks(min_timestamp_ns, max_timestamp_ns)) {
    num_decimals_in_labels_ =
        std::max(num_decimals_in_labels_, timeline_ticks_.GetTimestampNumDigitsPrecision(tick));
  }
}

void TimelineUi::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                        const DrawContext& draw_context) {
  if (draw_context.current_mouse_tick.has_value()) {
    const uint64_t mouse_timestamp_ns =
        timeline_info_interface_->GetNsSinceStart(draw_context.current_mouse_tick.value());
    RenderMouseLabel(primitive_assembler, text_renderer, mouse_timestamp_ns);
  }
}

void TimelineUi::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                    TextRenderer& text_renderer, uint64_t min_tick,
                                    uint64_t max_tick, PickingMode picking_mode) {
  CaptureViewElement::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
                                         picking_mode);
  RenderBackground(primitive_assembler);
  if (timeline_info_interface_->GetTimeWindowUs() <= 0) return;

  uint64_t min_timestamp_ns = timeline_info_interface_->GetNsSinceStart(min_tick);
  uint64_t max_timestamp_ns = timeline_info_interface_->GetNsSinceStart(max_tick);

  // All labels will have the same number of decimals for consistency. We will store that number
  // because it is needed for the mouse label which is drawn independently.
  UpdateNumDecimalsInLabels(min_timestamp_ns, max_timestamp_ns);
  RenderLines(primitive_assembler, min_timestamp_ns, max_timestamp_ns);
  RenderLabels(primitive_assembler, text_renderer, min_timestamp_ns, max_timestamp_ns);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> TimelineUi::CreateAccessibleInterface() {
  return std::make_unique<AccessibleCaptureViewElement>(this, "Timeline");
}

}  // namespace orbit_gl