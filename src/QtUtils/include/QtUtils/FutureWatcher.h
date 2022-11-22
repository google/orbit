// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_FUTURE_WATCHER_H_
#define QT_UTILS_FUTURE_WATCHER_H_

#include <absl/types/span.h>

#include <QObject>
#include <QString>
#include <chrono>
#include <optional>

#include "OrbitBase/Future.h"

namespace orbit_qt_utils {

// FutureWatcher is a helper class to work in conjunction with ThreadPool and orbit_base::Future.
// It can wait for a future to complete while still running a QEventLoop that can process other
// events happening, for example drawing requests, when the user moves the window.
//
// You probably don't want to use this class directly and rather use MainThreadExecutor::WaitFor and
// WaitForAll.
//
// The methods WaitFor and WaitForAll are stateless, nevertheless this needs to be class,
// because in most cases we want to connect a Qt-signal to the Abort-slot. For example when
// the user closes the window, we want to abort waiting for a completion.
//
// Currently only orbit_base::Future<void> is supported, since MainThreadExecutor can't be
// templated. Support for arbitrary return types can be added when we allow Qt-linking in OrbitGl or
// move the code up.
//
// Usage example:
// orbit_base::Future<void> future = thread_pool_->Schedule([]() { foo(); });
// FutureWatcher watcher{};
// QObject::connect(&orbit_main_window, &OrbitMainWindow::CloseRequested,
//                  &watcher, &FutureWatcher::Abort);
//
// const FutureWatcher::Reason result = watcher.WaitFor(future);
// if (result != FutureWatcher::Reason::kFutureCompleted) return;
//
// /* Continue do my main thread stuff */
//
class FutureWatcher : public QObject {
  Q_OBJECT
 public:
  explicit FutureWatcher(QObject* parent = nullptr);

  enum class Reason { kFutureCompleted, kTimeout, kAbortRequested };

  [[nodiscard]] Reason WaitFor(const orbit_base::Future<void>& future,
                               std::optional<std::chrono::milliseconds> timeout) const;
  [[nodiscard]] Reason WaitForAll(absl::Span<orbit_base::Future<void>> futures,
                                  std::optional<std::chrono::milliseconds> timeout) const;

  void Abort();

 signals:
  void AbortRequested();
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_FUTURE_WATCHER_H_
