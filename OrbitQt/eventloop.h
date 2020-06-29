// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_EVENT_LOOP_H_
#define ORBIT_QT_EVENT_LOOP_H_

#include <QEventLoop>
#include <optional>
#include <outcome.hpp>

namespace OrbitQt {

/**
 * A wrapper around QEventLoop to allow returning a std::error_code instead of
 * just a plain integer return code.
 *
 * The function names don't follow our usual style guide but maintain API
 * compatibility to QEventLoop.
 *
 * Behavioural change against QEventLoop:
 * This event loop can queue an error before it's even running. The consecutive
 * call of exec() will then immediately return the queued error.
 * */
class EventLoop : public QObject {
  Q_OBJECT

 public:
  using QObject::QObject;
  using ProcessEventsFlag = QEventLoop::ProcessEventsFlag;
  using ProcessEventsFlags = QEventLoop::ProcessEventsFlags;

  outcome::result<int> exec(
      ProcessEventsFlags flags = ProcessEventsFlag::AllEvents) {
    if (error_) {
      auto error = error_.value();
      error_ = std::nullopt;
      return outcome::failure(error);
    }

    const int return_code = loop_.exec(flags);

    if (error_) {
      return outcome::failure(error_.value());
    } else {
      return outcome::success(return_code);
    }
  }

  void error(std::error_code e) {
    error_ = e;
    loop_.quit();
  }

  void quit() { return loop_.quit(); }
  void exit(int return_code) { return loop_.exit(return_code); }
  bool isRunning() const { return loop_.isRunning(); }
  void wakeUp() { return loop_.wakeUp(); }
  bool event(QEvent* event) { return loop_.event(event); }

  bool processEvents(ProcessEventsFlags flags = ProcessEventsFlag::AllEvents) {
    return loop_.processEvents(flags);
  }
  void processEvents(ProcessEventsFlags flags, int maxTime) {
    loop_.processEvents(flags, maxTime);
  }

 private:
  std::optional<std::error_code> error_;
  QEventLoop loop_;
};

}  // namespace OrbitQt

#endif  // ORBIT_QT_EVENT_LOOP_H_