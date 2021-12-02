// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_H_
#define ORBIT_GL_TRACK_H_

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "Batcher.h"
#include "CaptureViewElement.h"
#include "ClientData/CaptureData.h"
#include "ClientData/TimerChain.h"
#include "ClientData/TimerData.h"
#include "ClientProtos/capture_data.pb.h"
#include "Containers/BlockChain.h"
#include "CoreMath.h"
#include "GteVector.h"
#include "OrbitBase/Profiling.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"
#include "TimelineInfoInterface.h"
#include "TriangleToggle.h"
#include "Viewport.h"

class Track : public orbit_gl::CaptureViewElement, public std::enable_shared_from_this<Track> {
 public:
  enum class Type {
    kTimerTrack,
    kThreadTrack,
    kFrameTrack,
    kVariableTrack,
    kGpuTrack,
    kGraphTrack,
    kSchedulerTrack,
    kAsyncTrack,
    kMemoryTrack,
    kPageFaultsTrack,
    kUnknown,
  };

  static constexpr std::initializer_list<Type> kAllTrackTypes = {
      Type::kTimerTrack,  Type::kThreadTrack,     Type::kFrameTrack,     Type::kVariableTrack,
      Type::kGpuTrack,    Type::kGraphTrack,      Type::kSchedulerTrack, Type::kAsyncTrack,
      Type::kMemoryTrack, Type::kPageFaultsTrack, Type::kUnknown};

  explicit Track(CaptureViewElement* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
                 orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                 const orbit_client_data::CaptureData* capture_data);
  ~Track() override = default;

  void OnDrag(int x, int y) override;

  [[nodiscard]] virtual Type GetType() const = 0;

  [[nodiscard]] virtual uint64_t GetMinTime() const = 0;
  [[nodiscard]] virtual uint64_t GetMaxTime() const = 0;

  virtual void OnTimer(const orbit_client_protos::TimerInfo& /*timer_info*/) {}
  [[nodiscard]] bool IsPinned() const { return pinned_; }
  void SetPinned(bool value);

  [[nodiscard]] bool IsMoving() const { return picked_ && mouse_pos_last_click_ != mouse_pos_cur_; }
  [[nodiscard]] virtual std::string GetName() const = 0;
  [[nodiscard]] virtual std::string GetLabel() const { return GetName(); }
  [[nodiscard]] virtual int GetNumberOfPrioritizedTrailingCharacters() const { return 0; }

  [[nodiscard]] virtual Color GetTrackBackgroundColor() const;

  virtual void OnCollapseToggle(bool is_collapsed);
  [[nodiscard]] virtual bool IsCollapsible() const { return false; }
  TriangleToggle* GetTriangleToggle() const { return collapse_toggle_.get(); }
  [[nodiscard]] virtual uint32_t GetProcessId() const { return orbit_base::kInvalidProcessId; }
  [[nodiscard]] virtual bool IsEmpty() const = 0;
  [[nodiscard]] bool ShouldBeRendered() const override;

  [[nodiscard]] float DetermineZOffset() const override;

  [[nodiscard]] virtual bool IsTrackSelected() const { return false; }

  [[nodiscard]] virtual bool IsCollapsed() const { return collapse_toggle_->IsCollapsed(); }

  [[nodiscard]] bool GetHeadless() const { return headless_; }
  void SetHeadless(bool value);

  void SetIndentationLevel(uint32_t level);
  [[nodiscard]] uint32_t GetIndentationLevel() const { return indentation_level_; }

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override {
    return {collapse_toggle_.get()};
  }

  [[nodiscard]] virtual int GetVisiblePrimitiveCount() const { return 0; }

  // Must be overriden by child class for sensible behavior.
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const = 0;
  // Must be overriden by child class for sensible behavior.
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const = 0;
  // Must be overriden by child class for sensible behavior.
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const = 0;
  // Must be overriden by child class for sensible behavior.
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& /*timer_info*/) const = 0;

 protected:
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;
  void DoUpdateLayout() override;

  void DrawTriangleFan(Batcher& batcher, const std::vector<Vec2>& points, const Vec2& pos,
                       const Color& color, float rotation, float z);
  virtual void UpdatePositionOfSubtracks() {}
  void UpdatePositionOfCollapseToggle();

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

  bool draw_background_ = true;
  bool pinned_ = false;
  bool headless_ = false;
  uint32_t indentation_level_ = 0;
  Type type_ = Type::kUnknown;
  std::shared_ptr<TriangleToggle> collapse_toggle_;

  TimeGraphLayout* layout_;
  const orbit_gl::TimelineInfoInterface* timeline_info_;
  const orbit_client_data::CaptureData* capture_data_ = nullptr;
};

#endif
