// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_EVENT_TRACK_H_
#define ORBIT_GL_EVENT_TRACK_H_

#include <GteVector.h>
#include <stdint.h>

#include <string>

#include "CoreMath.h"
#include "OrbitClientData/CallstackTypes.h"
#include "PickingManager.h"
#include "Track.h"

class CallStack;
class GlCanvas;
class OrbitApp;
class TimeGraph;

class EventTrack : public Track {
 public:
  explicit EventTrack(TimeGraph* time_graph, OrbitApp* app, CaptureData* capture_data);
  Type GetType() const override { return kEventTrack; }

  std::string GetTooltip() const override;

  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode,
                        float z_offset = 0) override;

  void OnPick(int x, int y) override;
  void OnRelease() override;
  void OnDrag(int x, int y) override;
  bool Draggable() override { return true; }
  float GetHeight() const override { return size_[1]; }

  void SetThreadId(ThreadID thread_id) { thread_id_ = thread_id; }
  void SetPos(float x, float y) { pos_ = Vec2(x, y); }
  bool IsEmpty() const override;
  [[nodiscard]] uint64_t GetMinTime() const override;
  [[nodiscard]] uint64_t GetMaxTime() const override;

 protected:
  void SelectEvents();
  [[nodiscard]] std::string GetSampleTooltip(PickingId id) const;
  [[nodiscard]] std::string SafeGetFormattedFunctionName(uint64_t addr, int max_line_length) const;
  [[nodiscard]] std::string FormatCallstackForTooltip(const CallStack& callstack,
                                                      int max_line_length = 80, int max_lines = 20,
                                                      int bottom_n_lines = 5) const;

 private:
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_EVENT_TRACK_H_
