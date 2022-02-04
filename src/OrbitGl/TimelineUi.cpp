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
  const float kLabelMarginLeft = 4;
  const float kLabelMarginRight = 2;
  const float kPixelMargin = 1;

  std::vector<uint64_t> all_major_ticks =
      timeline_ticks_.GetMajorTicks(min_timestamp_ns, max_timestamp_ns);
  // The label of the previous major tick could be also partially visible.
  std::optional<uint64_t> previous_major_tick =
      timeline_ticks_.GetPreviousMajorTick(min_timestamp_ns, max_timestamp_ns);
  if (previous_major_tick.has_value()) {
    all_major_ticks.insert(all_major_ticks.begin(), previous_major_tick.value());
  }

  int number_of_decimal_places_needed = 0;
  for (uint64_t tick : all_major_ticks) {
    number_of_decimal_places_needed = std::max(
        number_of_decimal_places_needed, timeline_ticks_.GetTimestampNumDigitsPrecision(tick));
  }
  float previous_label_end_x = std::numeric_limits<float>::lowest();
  for (uint64_t tick_ns : all_major_ticks) {
    std::string label;
    // TODO(http://b/170712621): Remove this flag when we decide which timestamp format we will use.
    if (absl::GetFlag(FLAGS_iso_timestamps)) {
      label = orbit_display_formats::GetDisplayISOTimestamp(
          absl::Nanoseconds(tick_ns), number_of_decimal_places_needed,
          absl::Nanoseconds(timeline_info_interface_->GetCaptureTimeSpanNs()));
    } else {
      label = orbit_display_formats::GetDisplayTime(absl::Nanoseconds(tick_ns));
    }
    float world_x = timeline_info_interface_->GetWorldFromUs(
        tick_ns / static_cast<double>(kNanosecondsPerMicrosecond));
    if (world_x > previous_label_end_x + kLabelMarginRight + kPixelMargin) {
      Vec2 pos, size;
      float label_middle_y = GetPos()[1] + GetHeightWithoutMargin() / 2;
      text_renderer.AddText(label.c_str(), world_x + kLabelMarginLeft, label_middle_y,
                            GlCanvas::kZValueTimeBar,
                            {layout_->GetFontSize(), Color(255, 255, 255, 255), -1.f,
                             TextRenderer::HAlign::Left, TextRenderer::VAlign::Middle},
                            &pos, &size);
      previous_label_end_x = pos[0] + size[0];

      // Box behind the label to hide the ticks behind it.
      const float kBackgroundBoxVerticalMargin = 4;
      size[0] += kLabelMarginRight;
      pos[1] = pos[1] - kBackgroundBoxVerticalMargin;
      size[1] = size[1] + 2 * kBackgroundBoxVerticalMargin;
      Box background_box(pos, size, GlCanvas::kZValueTimeBar);
      batcher.AddBox(background_box, GlCanvas::kTimeBarBackgroundColor);
    }
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