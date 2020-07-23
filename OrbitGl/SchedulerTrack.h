// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SCHEDULER_TRACK_H_
#define ORBIT_GL_SCHEDULER_TRACK_H_

#include "ThreadTrack.h"

class SchedulerTrack : public ThreadTrack {
 public:
  explicit SchedulerTrack(TimeGraph* time_graph);
  ~SchedulerTrack() override = default;
  
  std::string GetTooltip() const override;

  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, bool picking) override;
  Type GetType() const override { return kSchedulerTrack; }
  float GetHeight() const override;
  bool HasEventTrack() const override { return false; }
  bool IsCollapsable() const override { return false; }

 protected:
  float GetYFromDepth(float track_y, uint32_t depth, bool collapsed) override;
  std::string GetBoxTooltip(PickingID id) const;
};

#endif  // ORBIT_GL_SCHEDULER_TRACK_H_
