// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FRAME_TRACK_H_
#define ORBIT_GL_FRAME_TRACK_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "CallstackThreadBar.h"
#include "ClientData/TimerChain.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "TimerTrack.h"
#include "Track.h"
#include "Viewport.h"
#include "capture_data.pb.h"

class OrbitApp;

class FrameTrack : public TimerTrack {
 public:
  explicit FrameTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                      orbit_grpc_protos::InstrumentedFunction function, OrbitApp* app,
                      const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override {
    return absl::StrFormat("Frame track based on %s", function_.function_name());
  }
  [[nodiscard]] Type GetType() const override { return Type::kFrameTrack; }
  [[nodiscard]] uint64_t GetFunctionId() const { return function_.function_id(); }
  [[nodiscard]] bool IsCollapsible() const override {
    return GetCappedMaximumToAverageRatio() > 0.f;
  }

  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] float GetDefaultBoxHeight() const override;
  [[nodiscard]] float GetDynamicBoxHeight(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] float GetHeaderHeight() const override;

  [[nodiscard]] std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;

  void Draw(Batcher& batcher, TextRenderer& text_renderer,
            const DrawContext& draw_context) override;

 protected:
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected, bool is_highlighted) const override;
  [[nodiscard]] float GetHeight() const override;

 private:
  [[nodiscard]] float GetCappedMaximumToAverageRatio() const;
  [[nodiscard]] float GetMaximumBoxHeight() const;
  [[nodiscard]] float GetAverageBoxHeight() const;

  orbit_grpc_protos::InstrumentedFunction function_;
  orbit_client_protos::FunctionStats stats_;
};

#endif  // ORBIT_GL_FRAME_TRACK_H_
