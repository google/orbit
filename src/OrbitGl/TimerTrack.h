// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_TRACK_H_
#define ORBIT_GL_TIMER_TRACK_H_

#include <stdint.h>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "BlockChain.h"
#include "CallstackThreadBar.h"
#include "CaptureViewElement.h"
#include "ClientData/CallstackTypes.h"
#include "ClientData/TimerChain.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "TextRenderer.h"
#include "TracepointThreadBar.h"
#include "Track.h"
#include "Viewport.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"

class OrbitApp;
using orbit_client_protos::TimerInfo;

namespace internal {
struct DrawData {
  uint64_t min_tick;
  uint64_t max_tick;
  uint64_t highlighted_function_id;
  uint64_t highlighted_group_id;
  uint64_t ns_per_pixel;
  uint64_t min_timegraph_tick;
  Batcher* batcher;
  orbit_gl::Viewport* viewport;
  const orbit_client_protos::TimerInfo* selected_timer;
  double inv_time_window;
  float track_start_x;
  float track_width;
  float z_offset;
  float z;
  bool is_collapsed;
};
}  // namespace internal

class TimerTrack : public Track {
 public:
  explicit TimerTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                      const orbit_client_data::CaptureData* capture_data,
                      orbit_client_data::TrackData* track_data);
  ~TimerTrack() override = default;

  // Pickable
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  [[nodiscard]] std::string GetTooltip() const override;

  // Track
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode /*picking_mode*/, float z_offset = 0) override;
  [[nodiscard]] Type GetType() const override { return Type::kTimerTrack; }

  [[nodiscard]] std::string GetExtraInfo(const orbit_client_protos::TimerInfo& timer) const;

  [[nodiscard]] const TimerInfo* GetFirstAfterTime(uint64_t time, uint32_t depth) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetFirstBeforeTime(uint64_t time,
                                                                         uint32_t depth) const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetScopesInRange(
      uint64_t start_ns, uint64_t end_ns) const;
  [[nodiscard]] bool IsEmpty() const override;

  [[nodiscard]] bool IsCollapsible() const override { return track_data_->GetMaxDepth() > 1; }

  [[nodiscard]] virtual float GetDefaultBoxHeight() const { return layout_->GetTextBoxHeight(); }
  [[nodiscard]] virtual float GetDynamicBoxHeight(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return GetDefaultBoxHeight();
  }
  [[nodiscard]] virtual float GetYFromTimer(const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] virtual float GetYFromDepth(uint32_t depth) const;

  [[nodiscard]] virtual float GetHeaderHeight() const;

  [[nodiscard]] int GetVisiblePrimitiveCount() const override { return visible_timer_count_; }

  float GetHeight() const override;

  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetChains() const {
    return track_data_->GetChains();
  }

  [[nodiscard]] size_t GetNumberOfTimers() const;
  [[nodiscard]] uint64_t GetMinTime() const override;
  [[nodiscard]] uint64_t GetMaxTime() const override;

 protected:
  [[nodiscard]] virtual bool IsTimerActive(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return true;
  }
  [[nodiscard]] virtual Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                            bool is_selected, bool is_highlighted) const = 0;
  [[nodiscard]] virtual bool TimerFilter(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return true;
  }

  [[nodiscard]] bool DrawTimer(const orbit_client_protos::TimerInfo* prev_timer_info,
                               const orbit_client_protos::TimerInfo* next_timer_info,
                               const internal::DrawData& draw_data,
                               const orbit_client_protos::TimerInfo* current_timer_info,
                               uint64_t* min_ignore, uint64_t* max_ignore);

  [[nodiscard]] virtual std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& /*timer*/) const {
    return "";
  }
  [[nodiscard]] std::string GetDisplayTime(const orbit_client_protos::TimerInfo&) const;

  void DrawTimesliceText(const orbit_client_protos::TimerInfo& timer, float min_x, float z_offset,
                         Vec2 box_pos, Vec2 box_size);

  [[nodiscard]] static internal::DrawData GetDrawData(
      uint64_t min_tick, uint64_t max_tick, float track_width, float z_offset, Batcher* batcher,
      TimeGraph* time_graph, orbit_gl::Viewport* viewport, bool is_collapsed,
      const orbit_client_protos::TimerInfo* selected_timer, uint64_t highlighted_function_id,
      uint64_t highlighted_group_id);

  [[nodiscard]] virtual std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const;
  [[nodiscard]] std::unique_ptr<PickingUserData> CreatePickingUserData(
      const Batcher& batcher, const orbit_client_protos::TimerInfo& timer_info) {
    return std::make_unique<PickingUserData>(
        &timer_info, [this, &batcher](PickingId id) { return this->GetBoxTooltip(batcher, id); });
  }

  static const Color kHighlightColor;
  TextRenderer* text_renderer_ = nullptr;
  int visible_timer_count_ = 0;
  OrbitApp* app_ = nullptr;

  orbit_client_data::TrackData* track_data_;
};

#endif  // ORBIT_GL_TIMER_TRACK_H_
