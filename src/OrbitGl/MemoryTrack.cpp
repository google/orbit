// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTrack.h"

#include <algorithm>

#include "GlCanvas.h"
#include "OrbitBase/Logging.h"

namespace orbit_gl {

template <size_t Dimension>
void MemoryTrack<Dimension>::Draw(Batcher& batcher, TextRenderer& text_renderer,
                                  uint64_t current_mouse_time_ns, PickingMode picking_mode,
                                  float z_offset) {
  GraphTrack<Dimension>::Draw(batcher, text_renderer, current_mouse_time_ns, picking_mode,
                              z_offset);

  if (picking_mode != PickingMode::kNone || this->collapse_toggle_->IsCollapsed()) return;
  AnnotationTrack::DrawAnnotation(batcher, text_renderer, this->layout_,
                                  GlCanvas::kZValueTrackText + z_offset);
}

template <size_t Dimension>
void MemoryTrack<Dimension>::TrySetValueUpperBound(const std::string& pretty_label,
                                                   double raw_value) {
  double max_series_value = GraphTrack<Dimension>::GetGraphMaxValue();
  if (raw_value < max_series_value) {
    LOG("Fail to set MemoryTrack value upper bound: input value %f < maximum series value %f",
        raw_value, max_series_value);
    return;
  }
  this->SetValueUpperBound(pretty_label, raw_value);
}

template <size_t Dimension>
void MemoryTrack<Dimension>::TrySetValueLowerBound(const std::string& pretty_label,
                                                   double raw_value) {
  double min_series_value = GraphTrack<Dimension>::GetGraphMinValue();
  if (raw_value > min_series_value) {
    LOG("Fail to set MemoryTrack value lower bound: input value %f > minimum series value %f",
        raw_value, min_series_value);
    return;
  }
  this->SetValueLowerBound(pretty_label, raw_value);
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