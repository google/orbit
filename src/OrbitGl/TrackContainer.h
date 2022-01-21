// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_CONTAINER_H_
#define ORBIT_GL_TRACK_CONTAINER_H_

#include <ClientData/CaptureData.h>

#include <optional>
#include <string>
#include <vector>

#include "CaptureViewElement.h"
#include "ClientData/CaptureData.h"
#include "ClientData/TimerChain.h"
#include "TimeGraphLayout.h"
#include "Track.h"
#include "TrackManager.h"
#include "Viewport.h"
#include "absl/container/flat_hash_map.h"

namespace orbit_gl {

// Represent the space where Tracks will be drawn.
class TrackContainer final : public CaptureViewElement {
 public:
  explicit TrackContainer(CaptureViewElement* parent, TimelineInfoInterface* timeline_info,
                          Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                          orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] float GetHeight() const override;

  [[nodiscard]] const orbit_client_data::CaptureData* GetCaptureData() const {
    return capture_data_;
  }
  [[nodiscard]] TrackManager* GetTrackManager() { return track_manager_.get(); }
  [[nodiscard]] orbit_client_data::ThreadTrackDataProvider* GetCaptureDataProvider() {
    return thread_track_data_provider_;
  }

  [[nodiscard]] float GetTextBoxHeight() const { return layout_.GetTextBoxHeight(); }

  void VerticalZoom(float real_ratio, float mouse_screen_y_position);
  void VerticallyMoveIntoView(const orbit_client_protos::TimerInfo& timer_info);
  void VerticallyMoveIntoView(Track& track);

  [[nodiscard]] const orbit_client_protos::TimerInfo* FindPreviousFunctionCall(
      uint64_t function_address, uint64_t current_time,
      std::optional<uint32_t> thread_id = std::nullopt) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindNextFunctionCall(
      uint64_t function_address, uint64_t current_time,
      std::optional<uint32_t> thread_id = std::nullopt) const;
  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetAllTimersForHookedFunction(
      uint64_t function_address) const;

  void SetThreadFilter(const std::string& filter);

  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetAllThreadTrackTimerChains()
      const;
  [[nodiscard]] int GetNumVisiblePrimitives() const;

  [[nodiscard]] const TimeGraphLayout& GetLayout() const { return layout_; }
  [[nodiscard]] TimeGraphLayout& GetLayout() { return layout_; }

  [[nodiscard]] const orbit_client_protos::TimerInfo* FindPrevious(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindNext(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindTop(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindDown(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] std::pair<const orbit_client_protos::TimerInfo*,
                          const orbit_client_protos::TimerInfo*>
  GetMinMaxTimerInfoForFunction(uint64_t function_id) const;

  void SetIteratorOverlayData(
      const absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*>&
          iterator_timer_info,
      const absl::flat_hash_map<uint64_t, uint64_t>& iterator_id_to_function_id) {
    iterator_timer_info_ = iterator_timer_info;
    iterator_id_to_function_id_ = iterator_id_to_function_id;
    RequestUpdate();
  }
  [[nodiscard]] float GetVerticalScrollingOffset() const { return vertical_scrolling_offset_; }
  void SetVerticalScrollingOffset(float value);

  [[nodiscard]] bool HasFrameTrack(uint64_t function_id) const;
  void RemoveFrameTrack(uint64_t function_id);

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override;
  [[nodiscard]] std::vector<CaptureViewElement*> GetNonHiddenChildren() const override;

 protected:
  void DoUpdateLayout() override;
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  void UpdateTracksPosition();

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;

 private:
  void DrawOverlay(Batcher& batcher, TextRenderer& text_renderer, PickingMode picking_mode);
  void DrawIteratorBox(Batcher& batcher, TextRenderer& text_renderer, Vec2 pos, Vec2 size,
                       const Color& color, const std::string& label, const std::string& time,
                       float text_box_y);
  void DrawIncompleteDataIntervals(Batcher& batcher, PickingMode picking_mode);

  // First member is id.
  absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*> iterator_timer_info_;
  absl::flat_hash_map<uint64_t, uint64_t> iterator_id_to_function_id_;

  float vertical_scrolling_offset_ = 0;

  TimeGraphLayout layout_;
  std::unique_ptr<TrackManager> track_manager_;

  const orbit_client_data::CaptureData* capture_data_ = nullptr;
  orbit_client_data::ThreadTrackDataProvider* thread_track_data_provider_ = nullptr;

  const TimelineInfoInterface* timeline_info_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TRACK_CONTAINER_H_
