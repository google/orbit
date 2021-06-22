// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

#include "DoubleClickableLabel.h"

namespace orbit_qt {

TEST(DoubleClickableLabel, DoubleClickedSignalledOnDoubleClick) {
  DoubleClickableLabel label;
  QSignalSpy spy{&label, &DoubleClickableLabel::DoubleClicked};
  EXPECT_EQ(spy.count(), 0);
  QTest::mouseDClick(&label, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
  EXPECT_EQ(spy.count(), 1);
}

TEST(DoubleClickableLabel, DoubleClickedNotSignalledOnSingleClick) {
  DoubleClickableLabel label;
  QSignalSpy spy{&label, &DoubleClickableLabel::DoubleClicked};
  EXPECT_EQ(spy.count(), 0);
  QTest::mouseClick(&label, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
  EXPECT_EQ(spy.count(), 0);
}

}  // namespace orbit_qt