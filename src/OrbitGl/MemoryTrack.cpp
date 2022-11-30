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

template <size_t Dimension>
void MemoryTrack<Dimension>::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                                TextRenderer& text_renderer, uint64_t min_tick,
                                                uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("MemoryTrack<Dimension>::DoUpdatePrimitives", kOrbitColorGrey);
  GraphTrack<Dimension>::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick,
                                            picking_mode);
}

template <size_t Dimension>
void MemoryTrack<Dimension>::DoDraw(PrimitiveAssembler& primitive_assembler,
                                    TextRenderer& text_renderer,
                                    const CaptureViewElement::DrawContext& draw_context) {
  GraphTrack<Dimension>::DoDraw(primitive_assembler, text_renderer, draw_context);

  if (this->IsCollapsed()) return;
  AnnotationTrack::DrawAnnotation(primitive_assembler, text_renderer, this->layout_,
                                  this->indentation_level_, GlCanvas::kZValueTrackText);
}

template <size_t Dimension>
void MemoryTrack<Dimension>::TrySetValueUpperBound(std::string pretty_label, double raw_value) {
  double max_series_value = GraphTrack<Dimension>::GetGraphMaxValue();
  if (raw_value < max_series_value) {
    ORBIT_LOG("Fail to set MemoryTrack value upper bound: input value %f < maximum series value %f",
              raw_value, max_series_value);
    return;
  }
  this->SetValueUpperBound(std::move(pretty_label), raw_value);
}

template <size_t Dimension>
void MemoryTrack<Dimension>::TrySetValueLowerBound(std::string pretty_label, double raw_value) {
  double min_series_value = GraphTrack<Dimension>::GetGraphMinValue();
  if (raw_value > min_series_value) {
    ORBIT_LOG("Fail to set MemoryTrack value lower bound: input value %f > minimum series value %f",
              raw_value, min_series_value);
    return;
  }
  this->SetValueLowerBound(std::move(pretty_label), raw_value);
}

template <size_t Dimension>
double MemoryTrack<Dimension>::GetGraphMaxValue() const {
  if (!this->GetValueUpperBound().has_value()) {
    return GraphTrack<Dimension>::GetGraphMaxValue();
  }

  return std::max(GraphTrack<Dimension>::GetGraphMaxValue(),
                  this->GetValueUpperBound().value().second);
}

template <size_t Dimension>
double MemoryTrack<Dimension>::GetGraphMinValue() const {
  if (!this->GetValueLowerBound().has_value()) {
    return GraphTrack<Dimension>::GetGraphMinValue();
  }

  return std::min(GraphTrack<Dimension>::GetGraphMinValue(),
                  this->GetValueLowerBound().value().second);
}

template class MemoryTrack<1>;
template class MemoryTrack<2>;
template class MemoryTrack<3>;
template class MemoryTrack<4>;

}  // namespace orbit_gl
