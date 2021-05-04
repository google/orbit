// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRIANGLE_TOGGLE_H_
#define ORBIT_GL_TRIANGLE_TOGGLE_H_

#include <functional>
#include <memory>

#include "CaptureViewElement.h"
#include "CoreMath.h"
#include "Viewport.h"

class Track;

class TriangleToggle : public orbit_gl::CaptureViewElement,
                       public std::enable_shared_from_this<TriangleToggle> {
 public:
  enum class State { kInactive, kExpanded, kCollapsed };
  enum class InitialStateUpdate { kKeepInitialState, kReplaceInitialState };

  using StateChangeHandler = std::function<void(TriangleToggle::State)>;
  explicit TriangleToggle(State initial_state, StateChangeHandler handler, TimeGraph* time_graph,
                          orbit_gl::Viewport* viewport, TimeGraphLayout* layout, Track* track,
                          float size);
  ~TriangleToggle() override = default;

  TriangleToggle() = delete;
  TriangleToggle(const TriangleToggle&) = delete;
  TriangleToggle& operator=(const TriangleToggle&) = delete;
  TriangleToggle(TriangleToggle&&) = delete;
  TriangleToggle& operator=(TriangleToggle&&) = delete;

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset = 0) override;

  // Pickable
  void OnRelease() override;

  State GetState() const { return state_; }
  void SetState(State state,
                InitialStateUpdate update_initial_state = InitialStateUpdate::kKeepInitialState);
  void ResetToInitialState() { state_ = initial_state_; }
  bool IsCollapsed() const { return state_ == State::kCollapsed; }
  bool IsExpanded() const { return state_ == State::kExpanded; }
  bool IsInactive() const { return state_ == State::kInactive; }

 protected:
  std::unique_ptr<orbit_accessibility::AccessibleInterface> CreateAccessibleInterface() override;

 private:
  // Ideally this should be `CaptureViewElement* parent`, but as the track is not the parent, but
  // the virtual `TrackTab`, which is a child of the track itself, we stick to track here.
  // We require explicit knowledge about the parent.
  Track* track_;

  State state_ = State::kInactive;
  State initial_state_ = State::kInactive;
  StateChangeHandler handler_;
};

#endif  // ORBIT_GL_TRIANGLE_TOGGLE_H_
