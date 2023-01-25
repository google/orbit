// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/MemoryTrack.h"

#include <algorithm>
#include <optional>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/GlCanvas.h"

namespace orbit_gl {

void MemoryTrack::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                     TextRenderer& text_renderer, uint64_t min_tick,
                                     uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("MemoryTrack::DoUpdatePrimitives", kOrbitColorGrey);
  GraphTrack::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
                                 picking_mode);
}

void MemoryTrack::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                         const CaptureViewElement::DrawContext& draw_context) {
  GraphTrack::DoDraw(primitive_assembler, text_renderer, draw_context);

  if (IsCollapsed()) return;

  AnnotationTrack::DrawAnnotation(primitive_assembler, text_renderer, layout_, indentation_level_,
                                  GlCanvas::kZValueTrackText);
}

void MemoryTrack::TrySetValueUpperBound(std::string pretty_label, double raw_value) {
  double max_series_value = GraphTrack::GetGraphMaxValue();
  if (raw_value < max_series_value) {
    ORBIT_LOG("Fail to set MemoryTrack value upper bound: input value %f < maximum series value %f",
              raw_value, max_series_value);
    return;
  }
  SetValueUpperBound(std::move(pretty_label), raw_value);
}

void MemoryTrack::TrySetValueLowerBound(std::string pretty_label, double raw_value) {
  double min_series_value = GraphTrack::GetGraphMinValue();
  if (raw_value > min_series_value) {
    ORBIT_LOG("Fail to set MemoryTrack value lower bound: input value %f > minimum series value %f",
              raw_value, min_series_value);
    return;
  }
  SetValueLowerBound(std::move(pretty_label), raw_value);
}

double MemoryTrack::GetGraphMaxValue() const {
  if (!GetValueUpperBound().has_value()) return GraphTrack::GetGraphMaxValue();

  return std::max(GraphTrack::GetGraphMaxValue(), GetValueUpperBound().value().second);
}

double MemoryTrack::GetGraphMinValue() const {
  if (!GetValueLowerBound().has_value()) return GraphTrack::GetGraphMinValue();

  return std::min(GraphTrack::GetGraphMinValue(), GetValueLowerBound().value().second);
}

}  // namespace orbit_gl
