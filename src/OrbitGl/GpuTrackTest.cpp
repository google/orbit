// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include "GpuTrack.h"

using orbit_gl::MapGpuTimelineToTrackLabel;

TEST(GpuTrack, MapGpuTimelineToTrackLabelMapsRegularQueuesCorrectly) {
  EXPECT_EQ("Graphics queue (gfx)", MapGpuTimelineToTrackLabel("gfx"));

  EXPECT_EQ("DMA queue (sdma0)", MapGpuTimelineToTrackLabel("sdma0"));
  EXPECT_EQ("DMA queue (sdma1)", MapGpuTimelineToTrackLabel("sdma1"));

  EXPECT_EQ("Compute queue (comp_1.0.0)", MapGpuTimelineToTrackLabel("comp_1.0.0"));
  EXPECT_EQ("Compute queue (comp_1.1.0)", MapGpuTimelineToTrackLabel("comp_1.1.0"));

  EXPECT_EQ("Video Coding Engine (vce0)", MapGpuTimelineToTrackLabel("vce0"));
  EXPECT_EQ("Video Coding Engine (vce1)", MapGpuTimelineToTrackLabel("vce1"));
}

TEST(GpuTrack, MapGpuTimelineToTrackLabelIgnoresUnknownTimelines) {
  EXPECT_EQ("unknown_timeline", MapGpuTimelineToTrackLabel("unknown_timeline"));
}