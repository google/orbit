// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include "CaptureViewElementTester.h"
#include "PageFaultsTrack.h"
#include "TrackTestData.h"

namespace orbit_gl {

TEST(PageFaultsTrack, CaptureViewElementWorksAsIntended) {
  CaptureViewElementTester tester;
  std::unique_ptr<orbit_client_data::CaptureData> test_data =
      TrackTestData::GenerateTestCaptureData();
  PageFaultsTrack track = PageFaultsTrack(nullptr, nullptr, tester.GetViewport(),
                                          tester.GetLayout(), "", 100, test_data.get());
  EXPECT_EQ(2ull, track.GetChildren().size());
  tester.RunTests(&track);
}

}  // namespace orbit_gl