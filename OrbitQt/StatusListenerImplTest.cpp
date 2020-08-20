// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>

#include "StatusListenerImpl.h"
#include "gtest/gtest.h"

static int argc = 0;

TEST(StatusListenerImpl, ShowAndClearOneMessage) {
  QApplication app(argc, nullptr);
  auto status_bar = std::make_unique<QStatusBar>(nullptr);
  auto status_listener = StatusListenerImpl::Create(status_bar.get());

  EXPECT_EQ(status_bar->currentMessage(), "");
  auto id = status_listener->AddStatus("message 1");
  EXPECT_EQ(status_bar->currentMessage(), "message 1");
  status_listener->ClearStatus(id);
  EXPECT_EQ(status_bar->currentMessage(), "");
}

TEST(StatusListenerImpl, ShowAndUpdate) {
  constexpr const char* message1 = "message 1";
  constexpr const char* message2 = "message 2";
  constexpr const char* message3 = "message 3";
  constexpr const char* updated_message = "updated message";

  QApplication app(argc, nullptr);
  auto status_bar = std::make_unique<QStatusBar>(nullptr);
  auto status_listener = StatusListenerImpl::Create(status_bar.get());

  EXPECT_EQ(status_bar->currentMessage(), "");
  auto id1 = status_listener->AddStatus(message1);
  EXPECT_EQ(status_bar->currentMessage(), message1);
  auto id2 = status_listener->AddStatus(message2);
  EXPECT_EQ(status_bar->currentMessage(), message2);
  auto id3 = status_listener->AddStatus(message3);
  EXPECT_EQ(status_bar->currentMessage(), message3);

  status_listener->UpdateStatus(id2, updated_message);
  EXPECT_EQ(status_bar->currentMessage(), updated_message);

  status_listener->ClearStatus(id3);
  // Check that nothing changes.
  EXPECT_EQ(status_bar->currentMessage(), updated_message);

  // now we should see 1st message.
  status_listener->ClearStatus(id2);
  EXPECT_EQ(status_bar->currentMessage(), message1);
  status_listener->ClearStatus(id1);

  // Nothing left - status bar is empty.
  EXPECT_EQ(status_bar->currentMessage(), "");
}

TEST(StatusListenerImpl, CheckOrder) {
  constexpr const char* message1 = "message 1";
  constexpr const char* message2 = "message 2";
  constexpr const char* message3 = "message 3";
  constexpr const char* message4 = "message 4";
  constexpr const char* message5 = "message 5";

  QApplication app(argc, nullptr);
  auto status_bar = std::make_unique<QStatusBar>(nullptr);
  auto status_listener = StatusListenerImpl::Create(status_bar.get());

  EXPECT_EQ(status_bar->currentMessage(), "");

  auto id2 = status_listener->AddStatus(message2);
  EXPECT_EQ(status_bar->currentMessage(), message2);
  auto id4 = status_listener->AddStatus(message4);
  EXPECT_EQ(status_bar->currentMessage(), message4);
  auto id1 = status_listener->AddStatus(message1);
  EXPECT_EQ(status_bar->currentMessage(), message1);
  auto id5 = status_listener->AddStatus(message5);
  EXPECT_EQ(status_bar->currentMessage(), message5);
  auto id3 = status_listener->AddStatus(message3);
  EXPECT_EQ(status_bar->currentMessage(), message3);

  // Now update them in order
  status_listener->UpdateStatus(id1, message1);
  EXPECT_EQ(status_bar->currentMessage(), message1);
  status_listener->UpdateStatus(id2, message2);
  EXPECT_EQ(status_bar->currentMessage(), message2);
  status_listener->UpdateStatus(id3, message3);
  EXPECT_EQ(status_bar->currentMessage(), message3);
  status_listener->UpdateStatus(id4, message4);
  EXPECT_EQ(status_bar->currentMessage(), message4);
  status_listener->UpdateStatus(id5, message5);
  EXPECT_EQ(status_bar->currentMessage(), message5);

  // Remove from last to first - check that most recently
  // updated message is on the top.

  status_listener->ClearStatus(id5);
  EXPECT_EQ(status_bar->currentMessage(), message4);
  status_listener->ClearStatus(id4);
  EXPECT_EQ(status_bar->currentMessage(), message3);
  status_listener->ClearStatus(id3);
  EXPECT_EQ(status_bar->currentMessage(), message2);
  status_listener->ClearStatus(id2);
  EXPECT_EQ(status_bar->currentMessage(), message1);
  status_listener->ClearStatus(id1);
  // Nothing left - status bar is empty.
  EXPECT_EQ(status_bar->currentMessage(), "");
}

TEST(StatusListenerImpl, UpdateStatusInvalidId) {
  EXPECT_DEATH(
      {
        QApplication app(argc, nullptr);
        auto status_bar = std::make_unique<QStatusBar>(nullptr);
        auto status_listener = StatusListenerImpl::Create(status_bar.get());

        status_listener->UpdateStatus(10, "no message");
      },
      "");
}

TEST(StatusListenerImpl, ClearStatusInvalidId) {
  EXPECT_DEATH(
      {
        QApplication app(argc, nullptr);
        auto status_bar = std::make_unique<QStatusBar>(nullptr);
        auto status_listener = StatusListenerImpl::Create(status_bar.get());

        status_listener->ClearStatus(1);
      },
      "");
}