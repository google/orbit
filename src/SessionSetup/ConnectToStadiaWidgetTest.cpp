// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QPushButton>
#include <QRadioButton>
#include <QTableView>

#include "SessionSetup/ConnectToStadiaWidget.h"

namespace orbit_session_setup {

TEST(ConnectToStadiaWidget, IsSetActive) {
  ConnectToStadiaWidget widget{};

  // This is the radio button of the ConnectToStadiaWidget, which is one of three (with local
  // profiling enabled) in SessionSetupDialog. This radio button should always be enabled, even when
  // the widget is inactive. This has to be the case so the user can re-activate the widget.
  auto* radio_button = widget.findChild<QRadioButton*>("radioButton");
  ASSERT_NE(radio_button, nullptr);

  widget.show();

  // default
  EXPECT_TRUE(widget.IsActive());
  EXPECT_TRUE(radio_button->isEnabled());

  // set active false
  widget.SetActive(false);
  EXPECT_FALSE(widget.IsActive());
  EXPECT_TRUE(radio_button->isEnabled());

  // set active true
  widget.SetActive(true);
  EXPECT_TRUE(widget.IsActive());
  EXPECT_TRUE(radio_button->isEnabled());
}

}  // namespace orbit_session_setup