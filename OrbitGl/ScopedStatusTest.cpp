// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "MainThreadExecutor.h"
#include "ScopedStatus.h"

namespace {

class MockStatusListener : public StatusListener {
 public:
  MOCK_METHOD(uint64_t, AddStatus, (std::string message), (override));
  MOCK_METHOD(void, UpdateStatus, (uint64_t status_id, std::string message), (override));
  MOCK_METHOD(void, ClearStatus, (uint64_t status_id), (override));
};

class MockMainThreadExecutor : public MainThreadExecutor {
 public:
  MOCK_METHOD(void, Schedule, (std::unique_ptr<Action> action), (override));
};

}  // namespace

using ::testing::_;

TEST(ScopedStatus, Smoke) {
  MockStatusListener status_listener{};
  MockMainThreadExecutor main_thread_executor{};
  EXPECT_CALL(status_listener, AddStatus("Initial message")).Times(1);
  EXPECT_CALL(status_listener, UpdateStatus(_, "Updated message")).Times(1);
  EXPECT_CALL(status_listener, ClearStatus).Times(1);
  EXPECT_CALL(main_thread_executor, Schedule).Times(0);

  {
    ScopedStatus status(&main_thread_executor, &status_listener, "Initial message");
    status.UpdateMessage("Updated message");
  }
}

TEST(ScopedStatus, UpdateInAnotherThread) {
  MockStatusListener status_listener{};
  MockMainThreadExecutor main_thread_executor{};
  EXPECT_CALL(status_listener, AddStatus("Initial message")).Times(1);
  EXPECT_CALL(status_listener, ClearStatus).Times(1);
  EXPECT_CALL(main_thread_executor, Schedule).Times(1);

  {
    ScopedStatus status(&main_thread_executor, &status_listener, "Initial message");
    std::thread thread([&status] { status.UpdateMessage("Updated message"); });

    thread.join();
  }
}

TEST(ScopedStatus, DestroyInAnotherThread) {
  MockStatusListener status_listener{};
  MockMainThreadExecutor main_thread_executor{};
  EXPECT_CALL(status_listener, AddStatus("Initial message")).Times(1);
  EXPECT_CALL(status_listener, UpdateStatus(_, "Updated message")).Times(1);
  EXPECT_CALL(main_thread_executor, Schedule).Times(1);

  {
    ScopedStatus status(&main_thread_executor, &status_listener, "Initial message");
    status.UpdateMessage("Updated message");
    std::thread thread([status = std::move(status)]() {
      // Do nothing
    });

    thread.join();
  }
}

TEST(ScopedStatus, MoveAssignment) {
  MockStatusListener status_listener{};
  MockMainThreadExecutor main_thread_executor{};
  EXPECT_CALL(status_listener, AddStatus("Initial message 1")).Times(1);
  EXPECT_CALL(status_listener, AddStatus("Initial message 2")).Times(1);
  EXPECT_CALL(status_listener, UpdateStatus(_, "Updated message")).Times(1);
  EXPECT_CALL(status_listener, ClearStatus).Times(2);

  {
    ScopedStatus status1(&main_thread_executor, &status_listener, "Initial message 1");
    ScopedStatus status2(&main_thread_executor, &status_listener, "Initial message 2");
    status1.UpdateMessage("Updated message");
    status1 = std::move(status2);
  }
}

TEST(ScopedStatus, SelfMoveAssign) {
  MockStatusListener status_listener{};
  MockMainThreadExecutor main_thread_executor{};
  EXPECT_CALL(status_listener, AddStatus("Initial message")).Times(1);
  EXPECT_CALL(status_listener, UpdateStatus(_, "Updated message")).Times(1);
  EXPECT_CALL(status_listener, ClearStatus).Times(1);

  {
    ScopedStatus status1(&main_thread_executor, &status_listener, "Initial message");
    status1.UpdateMessage("Updated message");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
    status1 = std::move(status1);
#pragma GCC diagnostic pop
  }
}

TEST(ScopedStatus, Unitialized) { ScopedStatus status{}; }

TEST(ScopedStatus, UpdateUnutialized) {
  ScopedStatus status{};
  EXPECT_DEATH(status.UpdateMessage("Updated message"), "");
}
