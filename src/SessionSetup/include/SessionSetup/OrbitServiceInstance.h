// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_ORBIT_SERVICE_INSTANCE_H_
#define SESSION_SETUP_ORBIT_SERVICE_INSTANCE_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <filesystem>
#include <memory>

#include "OrbitBase/Result.h"

namespace orbit_session_setup {

// Abstracts a continuously running QProcess for using OrbitService on the client.
class OrbitServiceInstance : public QObject {
  Q_OBJECT;

 public:
  virtual ~OrbitServiceInstance() = default;
  [[nodiscard]] virtual bool IsRunning() const = 0;
  // Sends EOF to OrbitService and blocks until OrbitService ended.
  [[nodiscard]] virtual ErrorMessageOr<void> Shutdown() = 0;

  // Starts an OrbitService that is located next to client executable via pkexec. Blocks until
  // pkexec has started.
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> CreatePrivileged();
  // Creates and starts `program` with `arguments`. Used for testing. Blocks until `program` has
  // started.
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> Create(
      const QString& program, const QStringList& arguments);

 signals:
  // ErrorOccurred is emitted when an error with the process occurred _or_ when the process ends
  // unexpected.
  void ErrorOccurred(QString message);
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_ORBIT_SERVICE_INSTANCE_H_
