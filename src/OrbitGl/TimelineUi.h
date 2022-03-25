// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMELINE_UI_H_
#define ORBIT_GL_TIMELINE_UI_H_

#include "CaptureViewElement.h"
#include "TimelineInfoInterface.h"
#include "TimelineTicks.h"

namespace orbit_gl {

// TimelineUi is the class which takes care of drawing the timeline in the CaptureWindows.
class TimelineUi : public CaptureViewElement {
 public:
  explicit TimelineUi(CaptureViewElement* parent, const TimelineInfoInterface* timeline_info,
                      Viewport* viewport, TimeGraphLayout* layout)
      : CaptureViewElement(parent, viewport, layout), timeline_info_interface_(timeline_info) {}

  [[nodiscard]] float GetHeight() const override {
    return GetHeightWithoutMargin() + GetMarginHeight();
  }

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 private:
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;
  void DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;
  void RenderLines(Batcher& batcher, uint64_t min_timestamp_ns, uint64_t max_timestamp_ns) const;
  void RenderLabels(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_timestamp_ns,
                    uint64_t max_timestamp_ns) const;
  void RenderMargin(Batcher& batcher) const;
  void RenderBackground(Batcher& batcher) const;
  void RenderLabel(Batcher& batcher, TextRenderer& text_renderer, uint64_t tick_ns,
                   uint32_t number_of_decimal_places, float label_z, Color background_color) const;
  void RenderMouseLabel(Batcher& batcher, TextRenderer& text_renderer,
                        uint64_t mouse_tick_ns) const;
  [[nodiscard]] std::string GetLabel(uint64_t tick_ns, uint32_t number_of_decimal_places) const;
  [[nodiscard]] std::vector<uint64_t> GetTicksForNonOverlappingLabels(
      TextRenderer& text_renderer, const std::vector<uint64_t>& all_major_ticks) const;
  [[nodiscard]] bool WillLabelsOverlap(TextRenderer& text_renderer,
                                       const std::vector<uint64_t>& tick_list) const;
  [[nodiscard]] float GetTickWorldXPos(uint64_t tick_ns) const;
  [[nodiscard]] float GetHeightWithoutMargin() const { return layout_->GetTimeBarHeight(); }
  [[nodiscard]] float GetMarginHeight() const { return layout_->GetTimeBarMargin(); }
  [[nodiscard]] uint32_t GetNumDecimalsInLabels() const { return num_decimals_in_labels_; }
  void UpdateNumDecimalsInLabels(uint64_t min_timestamp_ns, uint64_t max_timestamp_ns);

  const TimelineInfoInterface* timeline_info_interface_;
  TimelineTicks timeline_ticks_;
  uint32_t num_decimals_in_labels_ = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TIMELINE_UI_H_
