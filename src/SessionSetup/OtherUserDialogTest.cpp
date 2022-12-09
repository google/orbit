// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QCoreApplication>
#include <QMetaObject>
#include <QSettings>
#include <Qt>
#include <memory>

#include "SessionSetup/OtherUserDialog.h"
#include "TestUtils/TestUtils.h"

namespace {
constexpr const char* kOrganizationName = "The Orbit Authors";
constexpr const char* kRememberKey = "OtherUserDialog.RememberKey";
}  // namespace

namespace orbit_session_setup {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(OtherUserDialog, ExecAccept) {
  QCoreApplication::setOrganizationDomain(kOrganizationName);
  QCoreApplication::setApplicationName("OtherUserDialog.ExecAccept");
  QSettings settings;
  settings.clear();

  OtherUserDialog dialog{"username"};

  QMetaObject::invokeMethod(
      &dialog, [&]() { dialog.accept(); }, Qt::QueuedConnection);

  auto result = dialog.Exec();
  EXPECT_THAT(result, HasValue());
}

TEST(OtherUserDialog, ExecReject) {
  QCoreApplication::setOrganizationDomain(kOrganizationName);
  QCoreApplication::setApplicationName("OtherUserDialog.ExecReject");
  QSettings settings;
  settings.clear();
  OtherUserDialog dialog{"username"};

  QMetaObject::invokeMethod(
      &dialog, [&]() { dialog.reject(); }, Qt::QueuedConnection);

  auto result = dialog.Exec();
  EXPECT_THAT(result, HasError("user rejected"));
}

TEST(OtherUserDialog, Remember) {
  QCoreApplication::setOrganizationDomain(kOrganizationName);
  QCoreApplication::setApplicationName("OtherUserDialog.WillRemember");
  QSettings settings;
  settings.clear();
  {
    OtherUserDialog dialog{"username"};

    QMetaObject::invokeMethod(
        &dialog,
        [&]() {
          auto* check_box = dialog.findChild<QCheckBox*>();
          ASSERT_TRUE(check_box != nullptr);
          check_box->setChecked(true);
          dialog.accept();
        },
        Qt::QueuedConnection);

    auto result = dialog.Exec();
    EXPECT_THAT(result, HasValue());
    EXPECT_TRUE(settings.contains(kRememberKey));
  }

  {
    OtherUserDialog dialog{"username"};
    auto result = dialog.Exec();
    EXPECT_THAT(result, HasValue());
  }
}

}  // namespace orbit_session_setup