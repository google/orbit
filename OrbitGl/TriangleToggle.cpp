// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TriangleToggle.h"

#include "GlCanvas.h"
#include "OpenGl.h"

TriangleToggle::TriangleToggle(bool initial_state, StateChangeHandler handler,
                               TimeGraph* time_graph)
    : is_active_(initial_state), handler_(handler), time_graph_(time_graph) {}

void TriangleToggle::Draw(GlCanvas* canvas, bool picking) {
  const Color kWhite(255, 255, 255, 255);
  Color color = kWhite;

  if (picking) {
    PickingManager& picking_manager = canvas->GetPickingManager();
    PickingID id = picking_manager.CreatePickableId(this);
    color = picking_manager.ColorFromPickingID(id);
  }

  // Draw triangle.
  static float half_sqrt_three = 0.5f * sqrtf(3.f);
  float half_w = 0.5f * size_;
  float half_h = half_sqrt_three * half_w;
  glPushMatrix();
  glTranslatef(pos_[0], pos_[1], 0);
  if (!is_active_) {
    glRotatef(90.f, 0, 0, 1.f);
  }
  glColor4ubv(&color[0]);
  glBegin(GL_TRIANGLES);
  glVertex3f(half_w, half_h, 0);
  glVertex3f(-half_w, half_h, 0);
  glVertex3f(0, -half_w, 0);
  glEnd();
  glPopMatrix();
}

void TriangleToggle::OnPick(int /*x*/, int /*y*/) {}

void TriangleToggle::OnRelease() {
  is_active_ = !is_active_;
  handler_(is_active_);
  time_graph_->NeedsUpdate();
}
