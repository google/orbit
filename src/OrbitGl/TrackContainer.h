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
                          const orbit_client_data::ModuleManager* module_manager,
                          orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] float GetHeight() const override { return height_; };
  void SetHeight(float height) { height_ = height; }
  [[nodiscard]] float GetVisibleTracksTotalHeight() const;

  [[nodiscard]] TrackManager* GetTrackManager() { return track_manager_.get(); }

  void VerticalZoom(float real_ratio, float mouse_screen_y_position);
  void VerticallyMoveIntoView(const orbit_client_protos::TimerInfo& timer_info);
  void VerticallyMoveIntoView(const Track& track);

  void SetThreadFilter(const std::string& filter);

  [[nodiscard]] int GetNumVisiblePrimitives() const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* FindPrevious(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindNext(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindTop(
      const orbit_client_protos::TimerInfo& from);
  [[nodiscard]] const orbit_client_protos::TimerInfo* FindDown(
      const orbit_client_protos::TimerInfo& from);

  void SetIteratorOverlayData(
      const absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*>&
          iterator_timer_info,
      const absl::flat_hash_map<uint64_t, uint64_t>& iterator_id_to_function_id);
  void UpdateVerticalScrollUsingRatio(float ratio);
  [[nodiscard]] float GetVerticalScrollingOffset() const { return vertical_scrolling_offset_; }
  void SetVerticalScrollingOffset(float value);

  void IncrementVerticalScroll(float ratio);
  [[nodiscard]] bool HasFrameTrack(uint64_t function_id) const;
  void RemoveFrameTrack(uint64_t function_id);

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override;
  [[nodiscard]] std::vector<CaptureViewElement*> GetNonHiddenChildren() const override;

 protected:
  void DoUpdateLayout() override;
  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  void UpdateTracksPosition();

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;

 private:
  void DrawOverlay(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                   PickingMode picking_mode);
  void DrawThreadDependency(PrimitiveAssembler& primitive_assembler, PickingMode picking_mode);
  void DrawIteratorBox(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                       Vec2 pos, Vec2 size, const Color& color, const std::string& label,
                       const std::string& time, float text_box_y);
  void DrawIncompleteDataIntervals(PrimitiveAssembler& primitive_assembler,
                                   PickingMode picking_mode);

  // First member is id.
  absl::flat_hash_map<uint64_t, const orbit_client_protos::TimerInfo*> iterator_timer_info_;
  absl::flat_hash_map<uint64_t, uint64_t> iterator_id_to_function_id_;

  float vertical_scrolling_offset_ = 0;
  float height_ = 0;

  std::unique_ptr<TrackManager> track_manager_;

  const orbit_client_data::CaptureData* capture_data_ = nullptr;

  const TimelineInfoInterface* timeline_info_;

  OrbitApp* app_ = nullptr;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TRACK_CONTAINER_H_
