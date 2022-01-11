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

  [[nodiscard]] float GetHeight() const override { return layout_->GetTimeBarHeight(); }

 private:
  void DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;
  void RenderLines(Batcher& batcher, uint64_t min_timestamp_ns, uint64_t max_timestamp_ns) const;
  void RenderLabels(TextRenderer& text_renderer, uint64_t min_timestamp_ns,
                    uint64_t max_timestamp_ns) const;
  void RenderBackground(Batcher& batcher) const;

  const TimelineInfoInterface* timeline_info_interface_;
  TimelineTicks timeline_ticks_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TIMELINE_UI_H_
