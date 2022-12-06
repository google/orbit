// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_H_
#define ORBIT_GL_TIMER_H_

#include "OrbitBase/Profiling.h"

class Timer {
 public:
  explicit Timer() { Restart(); }

  void Stop() { end_ = Now(); }
  void Restart() {
    start_ = Now();
    end_ = 0;
  }

  [[nodiscard]] uint64_t ElapsedNanos() const { return EndOrNow() - start_; }
  [[nodiscard]] double ElapsedMicros() const {
    return static_cast<double>(ElapsedNanos()) / 1000.0;
  }
  [[nodiscard]] double ElapsedMillis() const { return ElapsedMicros() / 1000.0; }

 private:
  [[nodiscard]] uint64_t EndOrNow() const { return end_ == 0 ? Now() : end_; }
  [[nodiscard]] static uint64_t Now() { return orbit_base::CaptureTimestampNs(); }

  uint64_t start_ = 0;
  uint64_t end_ = 0;
};

#endif  // ORBIT_GL_TIMER_H_
