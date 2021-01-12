// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SCOPED_CONNECTION_H_
#define ORBIT_SSH_QT_SCOPED_CONNECTION_H_

#include <QMetaObject>
#include <QObject>

namespace orbit_ssh_qt {

/*
  Disconnects a Qt-signal-slot-connection when this object gets out-of-scope.
  Just construct it from the return type of QObject::connect(...);
*/
class ScopedConnection {
  QMetaObject::Connection connection_;

 public:
  ScopedConnection() = default;
  explicit ScopedConnection(QMetaObject::Connection&& conn) : connection_(std::move(conn)) {}
  ScopedConnection(const ScopedConnection&) = delete;
  ScopedConnection& operator=(const ScopedConnection&) = delete;
  ScopedConnection(ScopedConnection&& other) : connection_(std::move(other.connection_)) {
    other.connection_ = QMetaObject::Connection{};
  }
  ScopedConnection& operator=(ScopedConnection&& other) {
    if (this != &other) {
      connection_ = std::move(other.connection_);
      other.connection_ = QMetaObject::Connection{};
    }
    return *this;
  }
  ~ScopedConnection() { QObject::disconnect(connection_); }
};

}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_SCOPED_CONNECTION_H_