// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CALLSTACK_THREAD_BAR_H_
#define ORBIT_GL_CALLSTACK_THREAD_BAR_H_

#include <GteVector.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "ClientModel/CaptureData.h"
#include "CoreMath.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/CallstackTypes.h"
#include "ThreadBar.h"

class OrbitApp;

namespace orbit_gl {

class CallstackThreadBar : public ThreadBar {
 public:
  explicit CallstackThreadBar(CaptureViewElement* parent, OrbitApp* app, TimeGraph* time_graph,
                              TimeGraphLayout* layout,
                              const orbit_client_model::CaptureData* capture_data,
                              ThreadID thread_id);

  std::string GetTooltip() const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;

  void OnPick(int x, int y) override;
  void OnRelease() override;

  [[nodiscard]] bool IsEmpty() const override;

  void SetColor(const Color& color) { color_ = color; }

 protected:
  void SelectCallstacks();
  [[nodiscard]] std::string SafeGetFormattedFunctionName(uint64_t addr, int max_line_length) const;
  [[nodiscard]] std::string FormatCallstackForTooltip(const CallStack& callstack,
                                                      int max_line_length = 80, int max_lines = 20,
                                                      int bottom_n_lines = 5) const;

  [[nodiscard]] std::string GetSampleTooltip(const Batcher& batcher, PickingId id) const;

  Color color_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_EVENT_TRACK_H_
