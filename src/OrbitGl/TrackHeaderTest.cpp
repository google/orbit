// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector4.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <string>

#include "OrbitGl/CaptureViewElementTester.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/TrackControlInterface.h"
#include "OrbitGl/TrackHeader.h"
#include "OrbitGl/TriangleToggle.h"

namespace orbit_gl {

using ::testing::Exactly;
using testing::Return;

class MockTrack : public TrackControlInterface {
 public:
  MOCK_METHOD(bool, IsPinned, (), (const override));
  MOCK_METHOD(void, SetPinned, (bool), (override));

  MOCK_METHOD(std ::string, GetLabel, (), (const override));
  MOCK_METHOD(std::string, GetName, (), (const override));
  MOCK_METHOD(int, GetNumberOfPrioritizedTrailingCharacters, (), (const override));
  MOCK_METHOD(Color, GetTrackBackgroundColor, (), (const override));
  MOCK_METHOD(uint32_t, GetIndentationLevel, (), (const override));

  MOCK_METHOD(bool, IsCollapsible, (), (const override));
  MOCK_METHOD(bool, Draggable, (), (override));

  MOCK_METHOD(bool, IsTrackSelected, (), (const override));
  MOCK_METHOD(void, SelectTrack, (), (override));

  MOCK_METHOD(void, DragBy, (float), (override));
};

TEST(TrackHeader, TrackHeaderDragsTheTrack) {
  CaptureViewElementTester tester;

  MockTrack track;
  constexpr int kDelta = 5;

  EXPECT_CALL(track, DragBy(kDelta)).Times(Exactly(2));
  EXPECT_CALL(track, Draggable()).Times(Exactly(2)).WillRepeatedly(Return(true));

  TrackHeader header(nullptr, tester.GetViewport(), tester.GetLayout(), &track);
  header.UpdateLayout();

  header.OnPick(0, 0);
  header.OnDrag(0, kDelta);
  header.OnDrag(0, kDelta);
  header.OnRelease();
}

TEST(TrackHeader, TrackHeaderDoesNotDragNonDraggableTracks) {
  CaptureViewElementTester tester;

  MockTrack track;
  constexpr int kDelta = 5;

  EXPECT_CALL(track, DragBy(kDelta)).Times(Exactly(0));
  EXPECT_CALL(track, Draggable()).Times(Exactly(2)).WillRepeatedly(Return(false));

  TrackHeader header(nullptr, tester.GetViewport(), tester.GetLayout(), &track);
  header.UpdateLayout();

  header.OnPick(0, 0);
  header.OnDrag(0, kDelta);
  header.OnDrag(0, kDelta);
  header.OnRelease();
}

TEST(TrackHeader, ClickingTrackHeadersSelectsTheTrack) {
  CaptureViewElementTester tester;

  MockTrack track;

  EXPECT_CALL(track, SelectTrack()).Times(Exactly(1));

  TrackHeader header(nullptr, tester.GetViewport(), tester.GetLayout(), &track);
  header.UpdateLayout();

  header.OnPick(0, 0);
  header.OnRelease();
}

TEST(TrackHeader, CollapseToggleWorksForCollapsibleTracks) {
  CaptureViewElementTester tester;

  MockTrack track;

  EXPECT_CALL(track, IsCollapsible()).WillRepeatedly(Return(true));

  TrackHeader header(nullptr, tester.GetViewport(), tester.GetLayout(), &track);
  header.UpdateLayout();

  TriangleToggle* toggle = header.GetCollapseToggle();
  EXPECT_TRUE(toggle->IsCollapsible());
  EXPECT_FALSE(toggle->IsCollapsed());

  toggle->OnPick(0, 0);
  toggle->OnRelease();

  EXPECT_TRUE(toggle->IsCollapsed());
}

TEST(TrackHeader, CollapseToggleDoesNotWorkForNonCollapsibleTracks) {
  CaptureViewElementTester tester;

  MockTrack track;

  EXPECT_CALL(track, IsCollapsible()).WillRepeatedly(Return(false));

  TrackHeader header(nullptr, tester.GetViewport(), tester.GetLayout(), &track);
  header.UpdateLayout();

  TriangleToggle* toggle = header.GetCollapseToggle();
  EXPECT_FALSE(toggle->IsCollapsible());
  EXPECT_FALSE(toggle->IsCollapsed());

  toggle->OnPick(0, 0);
  toggle->OnRelease();

  EXPECT_FALSE(toggle->IsCollapsed());
}

}  // namespace orbit_gl