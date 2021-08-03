// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MINOR_PAGE_FAULTS_TRACK_H_
#define ORBIT_GL_MINOR_PAGE_FAULTS_TRACK_H_

#include <string>
#include <utility>

#include "BasicPageFaultsTrack.h"

namespace orbit_gl {

class MinorPageFaultsTrack final : public BasicPageFaultsTrack {
 public:
  explicit MinorPageFaultsTrack(Track* parent, TimeGraph* time_graph, orbit_gl::Viewport* viewport,
                                TimeGraphLayout* layout, const std::string& cgroup_name,
                                uint64_t memory_sampling_period_ms,
                                const orbit_client_data::CaptureData* capture_data)
      : BasicPageFaultsTrack(parent, time_graph, viewport, layout, "Page Faults: Minor",
                             cgroup_name, memory_sampling_period_ms, capture_data) {}

  [[nodiscard]] std::string GetTooltip() const override;

 private:
  [[nodiscard]] std::string GetLegendTooltips(size_t legend_index) const override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MINOR_PAGE_FAULTS_TRACK_H_
