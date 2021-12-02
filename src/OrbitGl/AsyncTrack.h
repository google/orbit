// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ASYNC_TRACK_H_
#define ORBIT_GL_ASYNC_TRACK_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CallstackThreadBar.h"
#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "TimerTrack.h"
#include "Track.h"
#include "Viewport.h"
#include "absl/container/flat_hash_map.h"

class OrbitApp;

class AsyncTrack final : public TimerTrack {
 public:
  explicit AsyncTrack(CaptureViewElement* parent,
                      const orbit_gl::TimelineInfoInterface* timeline_info,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                      OrbitApp* app, const orbit_client_data::CaptureData* capture_data,
                      orbit_client_data::TimerData* timer_data);

  [[nodiscard]] std::string GetName() const override { return name_; };
  [[nodiscard]] Type GetType() const override { return Type::kAsyncTrack; };
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

 protected:
  void DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_tick,
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
