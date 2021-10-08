// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_TRACK_H_
#define ORBIT_GL_THREAD_TRACK_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <string>

#include "CallstackThreadBar.h"
#include "ClientData/ScopeTreeTimerData.h"
#include "Containers/ScopeTree.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "ThreadStateBar.h"
#include "TimerTrack.h"
#include "TracepointThreadBar.h"
#include "Track.h"
#include "Viewport.h"
#include "capture_data.pb.h"

class OrbitApp;

class ThreadTrack final : public TimerTrack {
 public:
  enum class ScopeTreeUpdateType { kAlways, kOnCaptureComplete, kNever };
  explicit ThreadTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout, uint32_t thread_id,
                       OrbitApp* app, const orbit_client_data::CaptureData* capture_data,
                       orbit_client_data::ScopeTreeTimerData* scope_tree_timer_data);

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetLabel() const override;
  [[nodiscard]] int GetNumberOfPrioritizedTrailingCharacters() const override;
  [[nodiscard]] Type GetType() const override { return Type::kThreadTrack; }
  [[nodiscard]] uint32_t GetProcessId() const override {
    return scope_tree_timer_data_->GetProcessId();
  }
  [[nodiscard]] size_t GetNumberOfTimers() const override {
    return scope_tree_timer_data_->GetNumberOfTimers();
  }
  [[nodiscard]] uint64_t GetMinTime() const override {
    return scope_tree_timer_data_->GetMinTime();
  }
  [[nodiscard]] uint64_t GetMaxTime() const override {
    return scope_tree_timer_data_->GetMaxTime();
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

  void OnPick(int x, int y) override;

  [[nodiscard]] bool IsEmpty() const override;

  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetChains() const {
    return scope_tree_timer_data_->GetChains();
  }

  [[nodiscard]] bool IsCollapsible() const override {
    return scope_tree_timer_data_->GetMaxDepth() > 1;
  }

  [[nodiscard]] std::vector<CaptureViewElement*> GetVisibleChildren() override;
  [[nodiscard]] std::vector<CaptureViewElement*> GetChildren() const override;

 protected:
  void DoUpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                          PickingMode picking_mode, float z_offset = 0) override;

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
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;

  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] float GetHeaderHeight() const override;

  void UpdatePositionOfSubtracks() override;

  int64_t thread_id_;

  std::shared_ptr<orbit_gl::ThreadStateBar> thread_state_bar_;
  std::shared_ptr<orbit_gl::CallstackThreadBar> event_bar_;
  std::shared_ptr<orbit_gl::TracepointThreadBar> tracepoint_bar_;

  orbit_client_data::ScopeTreeTimerData* scope_tree_timer_data_;
};

#endif  // ORBIT_GL_THREAD_TRACK_H_
