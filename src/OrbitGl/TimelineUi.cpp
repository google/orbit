// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimelineUi.h"

#include "AccessibleTimeline.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GlCanvas.h"
#include "TimelineTicks.h"

namespace orbit_gl {

void TimelineUi::RenderLines(Batcher& batcher, uint64_t min_timestamp_ns,
                             uint64_t max_timestamp_ns) const {
  const Color kMajorTickColor(255, 255, 255, 255);
  const Color kMinorTickColor(150, 150, 150, 255);
  constexpr int kPixelMargin = 1;
  const float ticks_height = GetHeight() - kPixelMargin;

  for (auto& [tick_type, tick_ns] :
       timeline_ticks_.GetAllTicks(min_timestamp_ns, max_timestamp_ns)) {
    float world_x = timeline_info_interface_->GetWorldFromUs(
        tick_ns / static_cast<double>(kNanosecondsPerMicrosecond));
    int screen_x = viewport_->WorldToScreen(Vec2(world_x, 0))[0];
    // TODO(b/208447247): Assure no overlap between minor ticks and labels.
    batcher.AddVerticalLine(
        Vec2(screen_x, GetPos()[1]), ticks_height, GlCanvas::kZValueTimeBar,
        tick_type == TimelineTicks::TickType::kMajorTick ? kMajorTickColor : kMinorTickColor);
  }
}

void TimelineUi::RenderLabels(TextRenderer& text_renderer, uint64_t min_timestamp_ns,
                              uint64_t max_timestamp_ns) const {
  const float kLabelMarginLeft = 4;
  const float kLabelMarginBottom = 2;
  for (uint64_t tick_ns : timeline_ticks_.GetMajorTicks(min_timestamp_ns, max_timestamp_ns)) {
    // TODO (b/170712621): Test the new format (ISO 8601) to draw timestamps.
    std::string text = orbit_display_formats::GetDisplayTime(absl::Nanoseconds(tick_ns));

    float world_x = timeline_info_interface_->GetWorldFromUs(
        tick_ns / static_cast<double>(kNanosecondsPerMicrosecond));
    float label_bottom_y = GetPos()[1] + GetHeight() - kLabelMarginBottom;
    text_renderer.AddText(text.c_str(), world_x + kLabelMarginLeft, label_bottom_y,
                          GlCanvas::kZValueTimeBar,
                          {layout_->GetFontSize(), Color(255, 255, 255, 255), -1.f,
                           TextRenderer::HAlign::Left, TextRenderer::VAlign::Bottom});
  }
}

void TimelineUi::RenderBackground(Batcher& batcher) const {
  Vec2 pos = GetPos();
  Vec2 size = GetSize();
  Box background_box(GetPos(), GetSize(), GlCanvas::kZValueTimeBarBg);
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
  RenderLabels(text_renderer, min_timestamp_ns, max_timestamp_ns);
}

std::unique_ptr<orbit_accessibility::AccessibleInterface> TimelineUi::CreateAccessibleInterface() {
  return std::make_unique<AccessibleTimeline>(this);
}

}  // namespace orbit_gl