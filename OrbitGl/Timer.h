// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_H_
#define ORBIT_GL_TIMER_H_

#include "OrbitBase/Profiling.h"

class Timer {
 public:
  explicit Timer() { Start(); }

  void Start() { Reset(); }
  void Stop() { end_ = Now(); }
  void Reset() {
    start_ = Now();
    end_ = 0;
  }

  [[nodiscard]] double ElapsedNanos() const { return static_cast<double>(EndOrNow() - start_); }
  [[nodiscard]] double ElapsedMicros() const { return ElapsedNanos() * 0.001; }
  [[nodiscard]] double ElapsedMillis() const { return ElapsedNanos() * 0.000001; }
  [[nodiscard]] double ElapsedSeconds() const { return ElapsedNanos() * 0.000000001; }

 private:
  [[nodiscard]] uint64_t EndOrNow() const { return end_ == 0 ? Now() : end_; }
  [[nodiscard]] uint64_t Now() const { return MonotonicTimestampNs(); }

  uint64_t start_ = 0;
  uint64_t end_ = 0;
};

#endif  // ORBIT_GL_TIMER_H_
