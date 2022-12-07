// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_TRACK_H_
#define ORBIT_GL_TIMER_TRACK_H_

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ScopeId.h"
#include "ClientData/TimerData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"
#include "Statistics/Histogram.h"

class OrbitApp;

namespace internal {
struct DrawData {
  uint64_t min_tick;
  uint64_t max_tick;
  std::optional<orbit_client_data::ScopeId> highlighted_scope_id;
  uint64_t highlighted_group_id;
  double ns_per_pixel;
  uint64_t min_timegraph_tick;
  orbit_gl::PrimitiveAssembler* primitive_assembler;
  const orbit_gl::Viewport* viewport;
  const orbit_client_protos::TimerInfo* selected_timer;
  double inv_time_window;
  float track_start_x;
  float track_width;
  float z;
  bool is_collapsed;
  std::optional<orbit_statistics::HistogramSelectionRange> histogram_selection_range;
};
}  // namespace internal

class TimerTrack : public Track {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  explicit TimerTrack(CaptureViewElement* parent,
                      const orbit_gl::TimelineInfoInterface* timeline_info,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                      const orbit_client_data::ModuleManager* module_manager,
                      const orbit_client_data::CaptureData* capture_data,
                      orbit_client_data::TimerData* timer_data);
  ~TimerTrack() override = default;

  // Pickable
  virtual void OnTimer(const orbit_client_protos::TimerInfo& timer_info);
  [[nodiscard]] std::string GetTooltip() const override;

  // Track
  [[nodiscard]] Type GetType() const override { return Type::kTimerTrack; }

  [[nodiscard]] uint32_t GetProcessId() const override { return timer_data_->GetProcessId(); }
  [[nodiscard]] static std::string GetExtraInfo(const orbit_client_protos::TimerInfo& timer);

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  [[nodiscard]] bool IsEmpty() const override;

  [[nodiscard]] virtual float GetDefaultBoxHeight() const { return layout_->GetTextBoxHeight(); }
  [[nodiscard]] virtual float GetDynamicBoxHeight(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return GetDefaultBoxHeight();
  }

  [[nodiscard]] virtual float GetYFromTimer(const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] virtual float GetYFromDepth(uint32_t depth) const;

  [[nodiscard]] virtual float GetHeightAboveTimers() const;

  [[nodiscard]] int GetVisiblePrimitiveCount() const override { return visible_timer_count_; }

  [[nodiscard]] virtual uint32_t GetDepth() const { return timer_data_->GetDepth(); }

  [[nodiscard]] virtual size_t GetNumberOfTimers() const;
  [[nodiscard]] uint64_t GetMinTime() const override;
  [[nodiscard]] uint64_t GetMaxTime() const override;

 protected:
  void DoUpdatePrimitives(orbit_gl::PrimitiveAssembler& primitive_assembler,
                          orbit_gl::TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode /*picking_mode*/) override;

  // TODO(b/179225487): Filtering is implemented only in `ThreadTrack`, that is, only the timers
  // corresponding to dynamically instrumented functions synchonous manual
  // instrumentation can be filtered.
  [[nodiscard]] virtual bool IsTimerActive(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return true;
  }

  [[nodiscard]] virtual Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                            bool is_selected, bool is_highlighted,
                                            const internal::DrawData& draw_data) const = 0;
  [[nodiscard]] virtual bool TimerFilter(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const {
    return true;
  }

  [[nodiscard]] bool DrawTimer(orbit_gl::TextRenderer& text_renderer,
                               const orbit_client_protos::TimerInfo* prev_timer_info,
                               const orbit_client_protos::TimerInfo* next_timer_info,
                               const internal::DrawData& draw_data,
                               const orbit_client_protos::TimerInfo* current_timer_info,
                               uint64_t* min_ignore, uint64_t* max_ignore);

  [[nodiscard]] virtual std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& /*timer*/) const {
    return "";
  }
  [[nodiscard]] static std::string GetDisplayTime(const orbit_client_protos::TimerInfo&);

  void DrawTimesliceText(orbit_gl::TextRenderer& text_renderer,
                         const orbit_client_protos::TimerInfo& timer, float min_x, Vec2 box_pos,
                         Vec2 box_size);

  [[nodiscard]] static internal::DrawData GetDrawData(
      uint64_t min_tick, uint64_t max_tick, float track_pos_x, float track_width,
      orbit_gl::PrimitiveAssembler* primitive_assembler,
      const orbit_gl::TimelineInfoInterface* timeline_info, const orbit_gl::Viewport* viewport,
      bool is_collapsed, const orbit_client_protos::TimerInfo* selected_timer,
      std::optional<ScopeId> highlighted_scope_id, uint64_t highlighted_group_id,
      std::optional<orbit_statistics::HistogramSelectionRange> histogram_selection_range);

  [[nodiscard]] virtual std::string GetBoxTooltip(
      const orbit_gl::PrimitiveAssembler& primitive_assembler, PickingId id) const;
  [[nodiscard]] std::unique_ptr<orbit_gl::PickingUserData> CreatePickingUserData(
      const orbit_gl::PrimitiveAssembler& primitive_assembler,
      const orbit_client_protos::TimerInfo& timer_info) {
    return std::make_unique<orbit_gl::PickingUserData>(
        &timer_info, [this, &primitive_assembler](PickingId id) {
          return this->GetBoxTooltip(primitive_assembler, id);
        });
  }

  [[nodiscard]] inline bool BoxHasRoomForText(orbit_gl::TextRenderer& text_renderer,
                                              const float width) {
    return text_renderer.GetStringWidth("w", layout_->GetFontSize()) < width;
  }

  [[nodiscard]] bool ShouldHaveBorder(
      const orbit_client_protos::TimerInfo* timer,
      const std::optional<orbit_statistics::HistogramSelectionRange>& range, float width) const;

  static const Color kHighlightColor;
  static const Color kBoxBorderColor;

  int visible_timer_count_ = 0;
  OrbitApp* app_ = nullptr;

  orbit_client_data::TimerData* timer_data_;
};

#endif  // ORBIT_GL_TIMER_TRACK_H_
