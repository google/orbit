// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ASYNC_TRACK_H_
#define ORBIT_GL_ASYNC_TRACK_H_

#include <absl/container/flat_hash_map.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TimerData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/TimerTrack.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"
#include "absl/container/flat_hash_map.h"

class OrbitApp;

class AsyncTrack final : public TimerTrack {
 public:
  explicit AsyncTrack(CaptureViewElement* parent,
                      const orbit_gl::TimelineInfoInterface* timeline_info,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                      OrbitApp* app, const orbit_client_data::ModuleManager* module_manager,
                      const orbit_client_data::CaptureData* capture_data,
                      orbit_client_data::TimerData* timer_data);

  [[nodiscard]] std::string GetName() const override { return name_; };
  [[nodiscard]] Type GetType() const override { return Type::kAsyncTrack; };
  [[nodiscard]] std::string GetBoxTooltip(const orbit_gl::PrimitiveAssembler& primitive_assembler,
                                          PickingId id) const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  [[nodiscard]] float GetHeight() const override;

 protected:
  void DoUpdatePrimitives(orbit_gl::PrimitiveAssembler& primitive_assembler,
                          orbit_gl::TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;
  [[nodiscard]] float GetDefaultBoxHeight() const override;
  [[nodiscard]] std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected, bool is_highlighted,
                                    const internal::DrawData& draw_data) const override;

  std::string name_;
  // Used for determining what row can receive a new timer with no overlap.
  absl::flat_hash_map<uint32_t, uint64_t> max_span_time_by_depth_;
};

#endif  // ORBIT_GL_ASYNC_TRACK_H_
