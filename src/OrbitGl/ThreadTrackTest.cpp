// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include "CaptureViewElementTester.h"
#include "ThreadTrack.h"
#include "TrackTestData.h"

namespace orbit_gl {

TEST(ThreadTrack, CaptureViewElementWorksAsIntended) {
  CaptureViewElementTester tester;
  std::unique_ptr<orbit_client_data::CaptureData> test_data =
      TrackTestData::GenerateTestCaptureData();
  ThreadTrack track = ThreadTrack(nullptr, nullptr, tester.GetViewport(), tester.GetLayout(), -1,
                                  nullptr, test_data.get(), nullptr);
  EXPECT_EQ(3ull, track.GetChildren().size());
  tester.RunTests(&track);
}

}  // namespace orbit_gl