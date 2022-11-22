// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "ClientData/CaptureData.h"
#include "OrbitGl/CaptureViewElementTester.h"
#include "OrbitGl/ThreadTrack.h"
#include "OrbitGl/TrackTestData.h"

namespace orbit_gl {

TEST(ThreadTrack, CaptureViewElementWorksAsIntended) {
  CaptureViewElementTester tester;
  std::unique_ptr<orbit_client_data::CaptureData> test_data =
      TrackTestData::GenerateTestCaptureData();
  ThreadTrack track(nullptr, nullptr, tester.GetViewport(), tester.GetLayout(), -1, nullptr,
                    nullptr, test_data.get(), nullptr);
  // Expect thread states, samples, tracepoints, and collapse toggle
  EXPECT_EQ(4ull, track.GetAllChildren().size());
  tester.RunTests(&track);
}

}  // namespace orbit_gl