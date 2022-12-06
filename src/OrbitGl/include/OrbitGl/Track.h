// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_H_
#define ORBIT_GL_TRACK_H_

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TimerChain.h"
#include "ClientData/TimerData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "Containers/BlockChain.h"
#include "GteVector.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/TrackControlInterface.h"
#include "OrbitGl/TrackHeader.h"
#include "OrbitGl/TriangleToggle.h"
#include "OrbitGl/Viewport.h"

class Track : public orbit_gl::CaptureViewElement,
              public orbit_gl::TrackControlInterface,
              public std::enable_shared_from_this<Track> {
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
                 const orbit_client_data::ModuleManager* module_manager,
                 const orbit_client_data::CaptureData* capture_data);
  ~Track() override = default;

  void OnPick(int x, int y) override;

  [[nodiscard]] virtual Type GetType() const = 0;

  [[nodiscard]] virtual uint64_t GetMinTime() const = 0;
  [[nodiscard]] virtual uint64_t GetMaxTime() const = 0;

  [[nodiscard]] bool IsPinned() const override { return pinned_; }
  void SetPinned(bool value) override;

  [[nodiscard]] bool IsMoving() const { return header_->IsBeingDragged(); }
  void DragBy(float delta_y) override;
  [[nodiscard]] bool Draggable() override { return true; }

  [[nodiscard]] std::string GetLabel() const override { return GetName(); }
  [[nodiscard]] int GetNumberOfPrioritizedTrailingCharacters() const override { return 0; }

  [[nodiscard]] Color GetTrackBackgroundColor() const override;

  [[nodiscard]] bool IsCollapsible() const override { return false; }
  [[nodiscard]] virtual uint32_t GetProcessId() const { return orbit_base::kInvalidProcessId; }
  [[nodiscard]] virtual bool IsEmpty() const = 0;
  [[nodiscard]] bool ShouldBeRendered() const override;

  [[nodiscard]] float DetermineZOffset() const override;

  [[nodiscard]] bool IsTrackSelected() const override { return false; }
  void SelectTrack() override{};

  [[nodiscard]] virtual bool IsCollapsed() const {
    return header_->GetCollapseToggle()->IsCollapsed();
  }

  void SetCollapsed(bool collapsed);

  void SetHeadless(bool value);

  void SetIndentationLevel(uint32_t level);
  [[nodiscard]] uint32_t GetIndentationLevel() const override { return indentation_level_; }

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override {
    return {header_.get()};
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
  void DoDraw(orbit_gl::PrimitiveAssembler& primitive_assembler,
              orbit_gl::TextRenderer& text_renderer, const DrawContext& draw_context) override;
  void DoUpdateLayout() override;

  virtual void UpdatePositionOfSubtracks() {}

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

  bool pinned_ = false;
  bool headless_ = false;
  uint32_t indentation_level_ = 0;
  Type type_ = Type::kUnknown;

  std::shared_ptr<orbit_gl::TrackHeader> header_;

  const orbit_gl::TimelineInfoInterface* timeline_info_;
  const orbit_client_data::ModuleManager* module_manager_ = nullptr;
  const orbit_client_data::CaptureData* capture_data_ = nullptr;
};

#endif
