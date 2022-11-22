// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_THROTTLE_H_
#define QT_UTILS_THROTTLE_H_

#include <QObject>
#include <QString>
#include <QTimer>
#include <chrono>
#include <optional>

namespace orbit_qt_utils {

// Throttle limits the number of events generated per time interval.
//
// Calls to `Fire` get throttled by emitting the `Triggered` signal at most once per `interval`. The
// Throttle also guarantees that each call of `Fire` results in at least one `Triggered` signal
// emission afterwards.
class Throttle : public QObject {
  Q_OBJECT

 public:
  explicit Throttle(std::chrono::milliseconds interval, QObject* parent = nullptr);
  void Fire();

 signals:
  void Triggered();

 private:
  void TriggerNow();

  std::chrono::milliseconds interval_;
  QTimer timer_;
  std::optional<std::chrono::steady_clock::time_point> last_time_executed_;
};

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_THROTTLE_H_
