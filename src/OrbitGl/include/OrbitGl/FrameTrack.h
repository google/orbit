// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_FRAME_TRACK_H_
#define ORBIT_GL_FRAME_TRACK_H_

#include <absl/strings/str_format.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ScopeStats.h"
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

class OrbitApp;

class FrameTrack : public TimerTrack {
 public:
  explicit FrameTrack(CaptureViewElement* parent,
                      const orbit_gl::TimelineInfoInterface* timeline_info,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout, uint64_t function_id,
                      orbit_client_data::FunctionInfo function, OrbitApp* app,
                      const orbit_client_data::ModuleManager* module_manager,
                      const orbit_client_data::CaptureData* capture_data,
                      orbit_client_data::TimerData* timer_data);

  [[nodiscard]] std::string GetName() const override {
    return absl::StrFormat("Frame track based on %s", function_.pretty_name());
  }
  [[nodiscard]] Type GetType() const override { return Type::kFrameTrack; }
  [[nodiscard]] uint64_t GetFunctionId() const { return function_id_; }
  [[nodiscard]] bool IsCollapsible() const override {
    return GetCappedMaximumToAverageRatio() > 0.f;
  }

  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] float GetDefaultBoxHeight() const override;
  [[nodiscard]] float GetDynamicBoxHeight(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  [[nodiscard]] std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] std::string GetBoxTooltip(const orbit_gl::PrimitiveAssembler& primitive_assembler,
                                          PickingId id) const override;

 protected:
  void DoUpdatePrimitives(orbit_gl::PrimitiveAssembler& primitive_assembler,
                          orbit_gl::TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;

  void DoDraw(orbit_gl::PrimitiveAssembler& primitive_assembler,
              orbit_gl::TextRenderer& text_renderer, const DrawContext& draw_context) override;

  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected, bool is_highlighted,
                                    const internal::DrawData& draw_data) const override;
  [[nodiscard]] float GetHeight() const override;

 private:
  [[nodiscard]] float GetCappedMaximumToAverageRatio() const;
  [[nodiscard]] float GetMaximumBoxHeight() const;
  [[nodiscard]] float GetAverageBoxHeight() const;

  uint64_t function_id_;
  orbit_client_data::FunctionInfo function_;
  orbit_client_data::ScopeStats stats_;
};

#endif  // ORBIT_GL_FRAME_TRACK_H_
