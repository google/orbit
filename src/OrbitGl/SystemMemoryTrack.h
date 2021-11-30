// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SYSTEM_MEMORY_TRACK_H_
#define ORBIT_GL_SYSTEM_MEMORY_TRACK_H_

#include <string>
#include <utility>

#include "MemoryTrack.h"

namespace orbit_gl {

constexpr size_t kSystemMemoryTrackDimension = 3;

class SystemMemoryTrack final : public MemoryTrack<kSystemMemoryTrackDimension> {
 public:
  explicit SystemMemoryTrack(CaptureViewElement* parent,
                             const orbit_gl::TimelineInfoInterface* timeline_info,
                             orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                             const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetTooltip() const override;

  void TrySetValueUpperBound(double total_mb);
  void SetWarningThreshold(double warning_threshold_mb);

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  enum class SeriesIndex { kUsedMb = 0, kBuffersOrCachedMb = 1, kUnusedMb = 2 };

 private:
  [[nodiscard]] std::string GetLegendTooltips(size_t legend_index) const override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_SYSTEM_MEMORY_TRACK_H_
