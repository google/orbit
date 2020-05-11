// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRIANGLE_TOGGLE_H_
#define ORBIT_GL_TRIANGLE_TOGGLE_H_

#include "PickingManager.h"

#include <functional>

class TimeGraph;
class GlCanvas;

class TriangleToggle : public Pickable {
 public:
  using StateChangeHandler = std::function<void(bool)>;
  explicit TriangleToggle(bool initial_state, StateChangeHandler handler,
                          TimeGraph* time_graph);
  virtual ~TriangleToggle() = default;

  TriangleToggle() = delete;
  TriangleToggle(const TriangleToggle&) = delete;
  TriangleToggle& operator=(const TriangleToggle&) = delete;
  TriangleToggle(TriangleToggle&&) = delete;
  TriangleToggle& operator=(TriangleToggle&&) = delete;

  // Pickable
  void Draw(GlCanvas* canvas, bool picking) override;
  void OnPick(int x, int y) override;
  void OnRelease() override;

  bool GetActive() const { return is_active_; }
  void SetActive(bool active) { is_active_ = active; }
  Vec2 GetPos() const { return pos_; }
  void SetPos(const Vec2& pos) { pos_ = pos; }

 private:
  bool is_active_;
  StateChangeHandler handler_;
  TimeGraph* time_graph_;
  Vec2 pos_ = {0, 0};
  float size_ = 10.f;
};

#endif
