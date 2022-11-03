// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMELINE_INFO_INTERFACE_H_
#define ORBIT_GL_TIMELINE_INFO_INTERFACE_H_

#include <stdint.h>

namespace orbit_gl {

class TimelineInfoInterface {
 public:
  virtual ~TimelineInfoInterface() = default;

  [[nodiscard]] virtual float GetWorldFromTick(uint64_t time) const = 0;
  [[nodiscard]] virtual float GetWorldFromUs(double micros) const = 0;
  [[nodiscard]] virtual uint64_t GetTickFromWorld(float world_x) const = 0;
  [[nodiscard]] virtual uint64_t GetTickFromUs(double micros) const = 0;
  [[nodiscard]] virtual double GetUsFromTick(uint64_t time) const = 0;
  [[nodiscard]] virtual uint64_t GetNsSinceStart(uint64_t tick) const = 0;

  [[nodiscard]] virtual double GetTimeWindowUs() const = 0;
  [[nodiscard]] virtual double GetMinTimeUs() const = 0;
  [[nodiscard]] virtual double GetMaxTimeUs() const = 0;

  virtual void ZoomTime(int zoom_delta, double center_time_ratio) = 0;
  // Translation from start and end ticks to box position and width, to be consistently used across
  // CaptureViewElements. It will extend boxes to the border of the pixels.
  [[nodiscard]] virtual std::pair<float, float> GetBoxPosXAndWidthFromTicks(
      uint64_t start_tick, uint64_t end_tick) const = 0;

  [[nodiscard]] virtual uint64_t GetCaptureTimeSpanNs() const = 0;
};

}  // namespace orbit_gl

#endif
