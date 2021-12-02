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
  [[nodiscard]] virtual double GetTimeWindowUs() const = 0;

  [[nodiscard]] virtual double GetMinTimeUs() const = 0;
  [[nodiscard]] virtual double GetMaxTimeUs() const = 0;
};

}  // namespace orbit_gl

#endif
