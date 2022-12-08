// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMELINE_UI_H_
#define ORBIT_GL_TIMELINE_UI_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/TimelineTicks.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

inline const Color kTimelineMajorTickColor(255, 254, 253, 255);
inline const Color kTimelineMinorTickColor(255, 254, 253, 63);

// TimelineUi is the class which takes care of drawing the timeline in the CaptureWindows.
class TimelineUi : public CaptureViewElement {
 public:
  explicit TimelineUi(CaptureViewElement* parent, TimelineInfoInterface* timeline_info,
                      Viewport* viewport, TimeGraphLayout* layout)
      : CaptureViewElement(parent, viewport, layout), timeline_info_interface_(timeline_info) {}

  [[nodiscard]] float GetHeight() const override { return layout_->GetTimeBarHeight(); }

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 protected:
  [[nodiscard]] EventResult OnMouseWheel(
      const Vec2& mouse_pos, int delta,
      const orbit_gl::ModifierKeys& modifiers = orbit_gl::ModifierKeys()) override;

 private:
  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;
  void DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                          uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) override;
  void RenderLines(PrimitiveAssembler& primitive_assembler, uint64_t min_timestamp_ns,
                   uint64_t max_timestamp_ns) const;
  void RenderLabels(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                    uint64_t min_timestamp_ns, uint64_t max_timestamp_ns) const;
  void RenderBackground(PrimitiveAssembler& primitive_assembler) const;
  void RenderLabel(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                   uint64_t tick_ns, uint32_t number_of_decimal_places,
                   const Color& background_color, bool is_mouse_label = false) const;
  void RenderMouseLabel(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                        uint64_t mouse_tick_ns) const;
  [[nodiscard]] std::string GetLabel(uint64_t tick_ns, uint32_t number_of_decimal_places) const;
  [[nodiscard]] std::vector<uint64_t> GetTicksForNonOverlappingLabels(
      TextRenderer& text_renderer, absl::Span<const uint64_t> all_major_ticks) const;
  [[nodiscard]] bool WillLabelsOverlap(TextRenderer& text_renderer,
                                       absl::Span<const uint64_t> tick_list) const;
  [[nodiscard]] float GetTickWorldXPos(uint64_t tick_ns) const;
  [[nodiscard]] uint32_t GetNumDecimalsInLabels() const { return num_decimals_in_labels_; }
  void UpdateNumDecimalsInLabels(uint64_t min_timestamp_ns, uint64_t max_timestamp_ns);

  TimelineInfoInterface* timeline_info_interface_;
  TimelineTicks timeline_ticks_;
  uint32_t num_decimals_in_labels_ = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TIMELINE_UI_H_
