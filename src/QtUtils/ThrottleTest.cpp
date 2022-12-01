// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QTimer>
#include <chrono>
#include <memory>
#include <ratio>

#include "QtUtils/Throttle.h"

namespace orbit_qt_utils {

// This Wait function uses QTimer for waiting and therefore relies on the OS to wake the thread back
// up when the timer expired. This ensures that `Wait` doesn't return earlier than the internal
// QTimer used in `Throttle`.
static void Wait(std::chrono::milliseconds interval) {
  QTimer timer{};
  timer.setSingleShot(true);
  QEventLoop loop{};
  QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

  // QTimer only guarantees an accuracy of 1 millisecond, which means the timer could
  // potentially time out 1 millisecond earlier than the requested delay. By waiting
  // 1 millisecond longer we avoid potential test failures.
  timer.start(interval + std::chrono::milliseconds{1});
  loop.exec();
}

class MockReceiver : public QObject {
 public:
  MOCK_METHOD(void, Execute, (), (const));
};

constexpr std::chrono::milliseconds kStandardDelay{10};

TEST(Throttle, TriggersImmediatelyOnFirstFire) {
  Throttle throttle{kStandardDelay};

  MockReceiver receiver{};
  QObject::connect(&throttle, &Throttle::Triggered, &receiver, &MockReceiver::Execute);

  // The first call leads to an immediate trigger.
  EXPECT_CALL(receiver, Execute()).Times(1);
  throttle.Fire();
}

TEST(Throttle, SecondImmediateFireLeadsToDelayedTrigger) {
  Throttle throttle{kStandardDelay};

  MockReceiver receiver{};
  QObject::connect(&throttle, &Throttle::Triggered, &receiver, &MockReceiver::Execute);

  // The first call leads to an immediate trigger.
  EXPECT_CALL(receiver, Execute()).Times(1);
  throttle.Fire();

  // The second call will start the timer. The trigger will be expected after the interval has
  // passed.
  throttle.Fire();

  EXPECT_CALL(receiver, Execute()).Times(1);
  Wait(kStandardDelay);
  QCoreApplication::processEvents();
}

TEST(Throttle, SecondDelayedFireLeadsToImmediateTrigger) {
// TODO(https://github.com/google/orbit/issues/4503): Enable test again.
#ifdef _WIN32
  GTEST_SKIP();
#endif
  Throttle throttle{kStandardDelay};

  MockReceiver receiver{};
  QObject::connect(&throttle, &Throttle::Triggered, &receiver, &MockReceiver::Execute);

  // The first call leads to an immediate trigger.
  EXPECT_CALL(receiver, Execute()).Times(1);
  throttle.Fire();

  Wait(kStandardDelay);
  QCoreApplication::processEvents();

  // The interval has passed, so the second call also leads to an immediate trigger.
  EXPECT_CALL(receiver, Execute()).Times(1);
  throttle.Fire();
}

TEST(Throttle, ThirdImmediateFireGetsConsumed) {
  Throttle throttle{kStandardDelay};

  MockReceiver receiver{};
  QObject::connect(&throttle, &Throttle::Triggered, &receiver, &MockReceiver::Execute);

  // We fire 3 times. The first calls leads to an immediate trigger. The second starts the timer.
  // The third gets consumed and combined with the second.
  EXPECT_CALL(receiver, Execute()).Times(1);
  throttle.Fire();
  throttle.Fire();
  throttle.Fire();

  EXPECT_CALL(receiver, Execute()).Times(1);
  Wait(kStandardDelay);
  QCoreApplication::processEvents();
}

TEST(Throttle, ThirdSlightlyDelayedFireGetsConsumed) {
  Throttle throttle{kStandardDelay};

  MockReceiver receiver{};
  QObject::connect(&throttle, &Throttle::Triggered, &receiver, &MockReceiver::Execute);

  // We fire two times. The first triggers immediately, the second starts the timer.
  EXPECT_CALL(receiver, Execute()).Times(1);
  throttle.Fire();
  throttle.Fire();

  // We call processEvents here to ensure that it does NOT call MockReceiver::Execute.
  QCoreApplication::processEvents();
  throttle.Fire();

  // After waiting for the interval, we expect the throttle to have triggered.
  EXPECT_CALL(receiver, Execute()).Times(1);
  Wait(kStandardDelay);
  QCoreApplication::processEvents();
}
}  // namespace orbit_qt_utils