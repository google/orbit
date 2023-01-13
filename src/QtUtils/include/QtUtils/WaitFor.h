// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_WAIT_FOR_H_
#define QT_UTILS_WAIT_FOR_H_

#include <OrbitBase/Future.h>
#include <qnamespace.h>

#include <QEventLoop>
#include <QMetaObject>
#include <tuple>

#include "OrbitBase/ImmediateExecutor.h"

namespace orbit_qt_utils {

// Blocks until `future` completes and then returns the stored value. While blocking this function
// spins up a Qt Event Loop and processes Qt events.
//
// If the future is already completed when this function is called, it is guaranteed that no event
// processing takes place and the function returns right away.
template <typename T>
T WaitFor(const orbit_base::Future<T>& future) {
  if (future.IsFinished()) return future.Get();

  QEventLoop event_loop{};
  std::optional<T> return_value;

  orbit_base::ImmediateExecutor immediate_executor{};
  std::ignore = future.Then(&immediate_executor, [&](const T& value) {
    // `return_value` might be written from a different thread, but since we won't access the
    // variable before the `event_loop` returns, this is fine. The `invokeMethod` call below is
    // thread-safe.
    return_value.emplace(value);
    QMetaObject::invokeMethod(&event_loop, &QEventLoop::quit, Qt::QueuedConnection);
  });
  event_loop.exec();
  return std::move(return_value.value());
}

// Overload of previous function for `Future<void>`. Check out the previous overload for details.
inline void WaitFor(const orbit_base::Future<void>& future) {
  if (future.IsFinished()) return;

  QEventLoop event_loop{};

  orbit_base::ImmediateExecutor immediate_executor{};
  std::ignore = future.Then(&immediate_executor, [&]() {
    // This lambda might be executed on a different thread which is no problem because the
    // `invokeMethod` call is thread-safe.
    QMetaObject::invokeMethod(&event_loop, &QEventLoop::quit, Qt::QueuedConnection);
  });
  event_loop.exec();
}
}  // namespace orbit_qt_utils

#endif  // QT_UTILS_WAIT_FOR_H_
