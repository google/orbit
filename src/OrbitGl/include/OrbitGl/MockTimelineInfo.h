// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MOCK_TIMELINE_INFO_H_
#define ORBIT_GL_MOCK_TIMELINE_INFO_H_

#include <gmock/gmock.h>

#include "OrbitGl/TimelineInfoInterface.h"

namespace orbit_gl {

// MockTimelineInfo is mocking TimelineInfoInterface supposing that all the timestamps are 0-based,
// so the capture starts exactly at 0. The implementation is kind of similar than the one used in
// TimeGraph but a bit simplified.
class MockTimelineInfo : public TimelineInfoInterface {
 public:
  explicit MockTimelineInfo(double width = 0.f) : width_(width) {}
  [[nodiscard]] uint64_t GetCaptureTimeSpanNs() const override { return max_capture_ns_; }
  [[nodiscard]] double GetTimeWindowUs() const override {
    return (max_visible_ns_ - min_visible_ns_) / 1000.0f;
  }
  [[nodiscard]] uint64_t GetNsSinceStart(uint64_t time) const override { return time; }

  [[nodiscard]] float GetWorldFromTick(uint64_t time) const override {
    return GetWorldFromUs(GetUsFromTick(time));
  }
  [[nodiscard]] float GetWorldFromUs(double micros) const override {
    return static_cast<float>((micros - GetMinTimeUs()) * width_ / GetTimeWindowUs());
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
    max_capture_ns_ = std::max(max_capture_ns_, max_tick);
  }

  [[nodiscard]] std::pair<float, float> GetBoxPosXAndWidthFromTicks(
      uint64_t start_tick, uint64_t end_tick) const override {
    float start_x = GetWorldFromTick(start_tick);
    float end_x = GetWorldFromTick(end_tick);
    return {start_x, end_x - start_x};
  }
  MOCK_METHOD(void, ZoomTime, (int, double), (override));

 private:
  double width_ = 0.f;
  uint64_t min_visible_ns_ = 0;
  uint64_t max_visible_ns_ = 0;
  uint64_t max_capture_ns_ = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MOCK_TIMELINE_INFO_H_
