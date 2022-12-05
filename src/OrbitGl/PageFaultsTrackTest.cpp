// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "OrbitGl/CaptureViewElementTester.h"
#include "OrbitGl/PageFaultsTrack.h"
#include "OrbitGl/TrackTestData.h"

namespace orbit_gl {

TEST(PageFaultsTrack, CaptureViewElementWorksAsIntended) {
  CaptureViewElementTester tester;
  std::unique_ptr<orbit_client_data::CaptureData> test_data =
      TrackTestData::GenerateTestCaptureData();
  PageFaultsTrack track = PageFaultsTrack(nullptr, nullptr, tester.GetViewport(),
                                          tester.GetLayout(), "", 100, nullptr, test_data.get());
  // Expect major pagefaults track, minor pagefaults track, and collapse toggle
  EXPECT_EQ(3ull, track.GetAllChildren().size());
  tester.RunTests(&track);
}

}  // namespace orbit_gl