// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MEMORY_TRACK_H_
#define ORBIT_GL_MEMORY_TRACK_H_

#include <string>
#include <utility>

#include "AnnotationTrack.h"
#include "CaptureViewElement.h"
#include "GraphTrack.h"
#include "Track.h"
#include "Viewport.h"

namespace orbit_gl {

template <size_t Dimension>
class MemoryTrack : public GraphTrack<Dimension>, public AnnotationTrack {
 public:
  explicit MemoryTrack(CaptureViewElement* parent,
                       const orbit_gl::TimelineInfoInterface* timeline_info,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                       std::array<std::string, Dimension> series_names,
                       uint8_t series_value_decimal_digits, std::string series_value_units,
                       const orbit_client_data::CaptureData* capture_data)
      : GraphTrack<Dimension>(parent, timeline_info, viewport, layout, series_names,
                              series_value_decimal_digits, std::move(series_value_units),
                              capture_data),
        AnnotationTrack() {
    // Memory tracks are collapsed by default.
    this->collapse_toggle_->SetCollapsed(true);
  }
  ~MemoryTrack() override = default;

  [[nodiscard]] Track::Type GetType() const override { return Track::Type::kMemoryTrack; }

  void TrySetValueUpperBound(const std::string& pretty_label, double raw_value);
  void TrySetValueLowerBound(const std::string& pretty_label, double raw_value);

 protected:
  void DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const CaptureViewElement::DrawContext& draw_context) override;

  [[nodiscard]] double GetGraphMaxValue() const override;
  [[nodiscard]] double GetGraphMinValue() const override;

 private:
  [[nodiscard]] float GetAnnotatedTrackContentHeight() const override {
    return this->GetGraphContentHeight();
  }
  [[nodiscard]] Vec2 GetAnnotatedTrackPosition() const override { return this->GetPos(); };
  [[nodiscard]] Vec2 GetAnnotatedTrackSize() const override { return this->GetSize(); };
  [[nodiscard]] uint32_t GetAnnotationFontSize(int indentation_level) const override {
    return this->GetLegendFontSize(indentation_level);
  }
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MEMORY_TRACK_H_
