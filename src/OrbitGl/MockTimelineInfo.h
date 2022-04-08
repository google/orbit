// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MOCK_TIMELINE_INFO_H_
#define ORBIT_GL_MOCK_TIMELINE_INFO_H_

#include "TimelineInfoInterface.h"

namespace orbit_gl {

// MockTimelineInfo is mocking TimelineInfoInterface supposing that all the timestamps are 0-based,
// so the capture starts exactly at 0. The implementation is kind of similar than the used in
// TimeGraph but a bit simplified.
class MockTimelineInfo : public TimelineInfoInterface {
 public:
  MockTimelineInfo(double width = 0.f) : width_(width) {}
  [[nodiscard]] uint64_t GetCaptureTimeSpanNs() const override {
    return max_capture_ns - min_capture_ns;
  }
  [[nodiscard]] double GetTimeWindowUs() const override {
    return (max_visible_ns_ - min_visible_ns_) / 1000.0f;
  }
  [[nodiscard]] uint64_t GetNsSinceStart(uint64_t time) const override { return time; }

  [[nodiscard]] float GetWorldFromTick(uint64_t time) const override {
    return GetWorldFromUs(GetUsFromTick(time));
  }
  [[nodiscard]] float GetWorldFromUs(double micros) const override {
    return (micros - GetMinTimeUs()) * width_ / GetTimeWindowUs();
  }
  [[nodiscard]] uint64_t GetTickFromWorld(float world_x) const override {
    double ratio = world_x / width_;
    return static_cast<uint64_t>(ratio * max_visible_ns_ + (1 - ratio) * min_visible_ns_);
  }

  [[nodiscard]] uint64_t GetTickFromUs(double micros) const override {
    return GetNsSinceStart(0) + static_cast<uint64_t>(std::floor(micros * 1000.0));
  }
  [[nodiscard]] double GetUsFromTick(uint64_t time) const override {
    return GetNsSinceStart(time) / 1000.0f;
  }

  [[nodiscard]] double GetMinTimeUs() const override { return min_visible_ns_ / 1000.0; }
  [[nodiscard]] double GetMaxTimeUs() const override { return max_visible_ns_ / 1000.0; }

  void SetWorldWidth(float width) { width_ = width; }
  void SetMinMax(uint64_t min_tick, uint64_t max_tick) {
    min_visible_ns_ = min_tick;
    max_visible_ns_ = max_tick;
    max_capture_ns = std::max(max_capture_ns, max_tick);
  }

 private:
  double width_ = 0.f;
  uint64_t min_visible_ns_ = 0;
  uint64_t max_visible_ns_ = 0;
  const uint64_t min_capture_ns = 0;
  uint64_t max_capture_ns = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MOCK_TIMELINE_INFO_H_
