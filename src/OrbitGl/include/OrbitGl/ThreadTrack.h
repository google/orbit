// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_TRACK_H_
#define ORBIT_GL_THREAD_TRACK_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ThreadTrackDataProvider.h"
#include "ClientData/TimerChain.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/CallstackThreadBar.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/ThreadStateBar.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/TimerTrack.h"
#include "OrbitGl/TracepointThreadBar.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"

class OrbitApp;

class ThreadTrack final : public TimerTrack {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  enum class ScopeTreeUpdateType { kAlways, kOnCaptureComplete, kNever };
  explicit ThreadTrack(CaptureViewElement* parent,
                       const orbit_gl::TimelineInfoInterface* timeline_info,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout, uint32_t thread_id,
                       OrbitApp* app, const orbit_client_data::ModuleManager* module_manager,
                       const orbit_client_data::CaptureData* capture_data,
                       orbit_client_data::ThreadTrackDataProvider* thread_track_data_provider);

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetLabel() const override;
  [[nodiscard]] int GetNumberOfPrioritizedTrailingCharacters() const override;
  [[nodiscard]] Type GetType() const override { return Type::kThreadTrack; }

  [[nodiscard]] uint32_t GetDepth() const override {
    return thread_track_data_provider_->GetDepth(thread_id_);
  }
  [[nodiscard]] uint32_t GetProcessId() const override {
    return thread_track_data_provider_->GetProcessId(thread_id_);
  }
  [[nodiscard]] size_t GetNumberOfTimers() const override {
    return thread_track_data_provider_->GetNumberOfTimers(thread_id_);
  }
  [[nodiscard]] uint64_t GetMinTime() const override {
    return thread_track_data_provider_->GetMinTime(thread_id_);
  }
  [[nodiscard]] uint64_t GetMaxTime() const override {
    return thread_track_data_provider_->GetMaxTime(thread_id_);
  }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  [[nodiscard]] float GetYFromDepth(uint32_t depth) const override;

  void SelectTrack() override;

  [[nodiscard]] bool IsEmpty() const override;

  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetChains() const {
    return thread_track_data_provider_->GetChains(thread_id_);
  }

  [[nodiscard]] bool IsCollapsible() const override { return GetDepth() > 1; }

  [[nodiscard]] Vec2 GetThreadStateBarPos() const { return thread_state_bar_->GetPos(); }
  [[nodiscard]] float GetThreadStateBarHeight() const { return thread_state_bar_->GetHeight(); }

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override;

 protected:
  void DoUpdatePrimitives(orbit_gl::PrimitiveAssembler& primitive_assembler,
                          orbit_gl::TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;

  [[nodiscard]] int64_t GetThreadId() const { return thread_id_; }
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] bool IsTrackSelected() const override;

  [[nodiscard]] float GetDefaultBoxHeight() const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer, bool is_selected,
                                    bool is_highlighted,
                                    const internal::DrawData& draw_data) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    const internal::DrawData& draw_data);
  [[nodiscard]] std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] std::string GetBoxTooltip(const orbit_gl::PrimitiveAssembler& primitive_assembler,
                                          PickingId id) const override;

  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] float GetHeightAboveTimers() const override;

  void UpdatePositionOfSubtracks() override;

  int64_t thread_id_;

  std::shared_ptr<orbit_gl::ThreadStateBar> thread_state_bar_;
  std::shared_ptr<orbit_gl::CallstackThreadBar> event_bar_;
  std::shared_ptr<orbit_gl::TracepointThreadBar> tracepoint_bar_;

  orbit_client_data::ThreadTrackDataProvider* thread_track_data_provider_;
};

#endif  // ORBIT_GL_THREAD_TRACK_H_
