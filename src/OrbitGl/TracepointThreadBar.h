// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACEPOINT_THREAD_BAR_H_
#define ORBIT_GL_TRACEPOINT_THREAD_BAR_H_

#include <stdint.h>

#include <string>

#include "CaptureViewElement.h"
#include "ThreadBar.h"

namespace orbit_gl {

class TracepointThreadBar : public ThreadBar {
 public:
  explicit TracepointThreadBar(CaptureViewElement* parent, OrbitApp* app, TimeGraph* time_graph,
                               TimeGraphLayout* layout,
                               const orbit_client_model::CaptureData* capture_data,
                               int32_t thread_id);

  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;

  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;

  [[nodiscard]] bool IsEmpty() const override;

  void SetColor(const Color& color) { color_ = color; }

 private:
  std::string GetTracepointTooltip(Batcher* batcher, PickingId id) const;

  Color color_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TRACEPOINT_TRACK_H_
