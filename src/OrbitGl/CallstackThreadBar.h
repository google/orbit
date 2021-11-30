// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CALLSTACK_THREAD_BAR_H_
#define ORBIT_GL_CALLSTACK_THREAD_BAR_H_

#include <GteVector.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "ClientData/CallstackTypes.h"
#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "CoreMath.h"
#include "ThreadBar.h"
#include "Viewport.h"

class OrbitApp;

namespace orbit_gl {

class CallstackThreadBar : public ThreadBar {
 public:
  explicit CallstackThreadBar(CaptureViewElement* parent, OrbitApp* app,
                              const orbit_gl::TimelineInfoInterface* timeline_info,
                              orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                              const orbit_client_data::CaptureData* capture_data,
                              orbit_client_data::ThreadID thread_id, const Color& color);

  std::string GetTooltip() const override;

  [[nodiscard]] float GetHeight() const override {
    return layout_->GetEventTrackHeightFromTid(GetThreadId());
  }

  void OnPick(int x, int y) override;
  void OnRelease() override;

  [[nodiscard]] bool IsEmpty() const override;

 protected:
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;
  void DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;

 private:
  void SelectCallstacks();
  [[nodiscard]] std::string SafeGetFormattedFunctionName(
      const orbit_client_protos::CallstackInfo& callstack, int frame_index,
      int max_line_length) const;
  [[nodiscard]] std::string FormatCallstackForTooltip(
      const orbit_client_protos::CallstackInfo& callstack, int max_line_length = 80,
      int max_lines = 20, int bottom_n_lines = 5) const;

  [[nodiscard]] std::string GetSampleTooltip(const Batcher& batcher, PickingId id) const;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_EVENT_TRACK_H_
