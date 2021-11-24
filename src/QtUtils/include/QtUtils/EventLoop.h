// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_EVENT_LOOP_H_
#define QT_UTILS_EVENT_LOOP_H_

#include <QEventLoop>
#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_qt_utils {

/**
 * A wrapper around QEventLoop to allow returning a std::error_code instead of just a plain integer
 * return code.
 *
 * The function names don't follow our usual style guide but maintain API compatibility to
 * QEventLoop.
 *
 * Behavioural change against QEventLoop:
 * This event loop can queue an result (error or return code) before it's even running. The
 * consecutive call of exec() will then immediately return the queued result.
 * */
class EventLoop : public QObject {
  Q_OBJECT

 public:
  explicit EventLoop(QObject* parent = nullptr) : QObject{parent}, loop_{this} {}

  using ProcessEventsFlag = QEventLoop::ProcessEventsFlag;
  using ProcessEventsFlags = QEventLoop::ProcessEventsFlags;

  outcome::result<int> exec(ProcessEventsFlags flags = ProcessEventsFlag::AllEvents) {
    if (result_ == std::nullopt) {
      (void)loop_.exec(flags);
    }

    CHECK(result_ != std::nullopt);

    auto result = result_.value();
    result_ = std::nullopt;
    return result;
  }

  void error(std::error_code e) {
    result_ = outcome::failure(e);
    loop_.quit();
  }

  void quit() {
    result_ = outcome::success(0);
    return loop_.quit();
  }
  void exit(int return_code) {
    result_ = outcome::success(return_code);
    return loop_.exit(return_code);
  }
  bool isRunning() const { return loop_.isRunning(); }
  void wakeUp() { return loop_.wakeUp(); }
  bool event(QEvent* event) { return loop_.event(event); }

  bool processEvents(ProcessEventsFlags flags = ProcessEventsFlag::AllEvents) {
    return loop_.processEvents(flags);
  }
  void processEvents(ProcessEventsFlags flags, int maxTime) { loop_.processEvents(flags, maxTime); }

 private:
  std::optional<outcome::result<int>> result_;
  QEventLoop loop_;
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_EVENT_LOOP_H_