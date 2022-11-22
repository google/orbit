// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_HEADER_H_
#define ORBIT_GL_TRACK_HEADER_H_

#include <GteVector.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TrackControlInterface.h"
#include "OrbitGl/TriangleToggle.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

// "Tab" element of the track. This encapsulates:
// * Display of track title
// * Collapse functionality
// * Click-to-drag behavior
class TrackHeader : public CaptureViewElement, public std::enable_shared_from_this<TrackHeader> {
 public:
  explicit TrackHeader(CaptureViewElement* parent, const Viewport* viewport,
                       const TimeGraphLayout* layout, TrackControlInterface* track);

  [[nodiscard]] const TriangleToggle* GetCollapseToggle() const { return collapse_toggle_.get(); }
  [[nodiscard]] TriangleToggle* GetCollapseToggle() { return collapse_toggle_.get(); }

  [[nodiscard]] float GetHeight() const override { return layout_->GetTrackTabHeight(); }
  [[nodiscard]] uint32_t GetLayoutFlags() const override { return 0; }

  void OnPick(int x, int y) override;

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override {
    return {collapse_toggle_.get()};
  }

  [[nodiscard]] std::string GetTooltip() const override { return GetParent()->GetTooltip(); }

  void OnDrag(int x, int y) override;
  [[nodiscard]] bool Draggable() override;

  [[nodiscard]] bool IsBeingDragged() {
    return picked_ && mouse_pos_last_click_[1] != mouse_pos_cur_[1];
  }

 protected:
  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;
  void DoUpdateLayout() override;

  void UpdateCollapseToggle();

  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 private:
  std::shared_ptr<TriangleToggle> collapse_toggle_;
  TrackControlInterface* track_;
};

}  // namespace orbit_gl

#endif