// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_CLIENT_H_
#define ORBIT_GGP_CLIENT_H_

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>
#include <chrono>
#include <functional>
#include <outcome.hpp>
#include <string>

#include "Instance.h"
#include "OrbitBase/Result.h"
#include "SshInfo.h"

namespace orbit_ggp {

constexpr const char* kDefaultGgpProgram{"ggp"};

class Client : public QObject {
  Q_OBJECT

 public:
  static ErrorMessageOr<QPointer<Client>> Create(
      QObject* parent, QString ggp_program = kDefaultGgpProgram,
      std::chrono::milliseconds timeout = GetDefaultTimeoutMs());

  void GetInstancesAsync(const std::function<void(outcome::result<QVector<Instance>>)>& callback,
                         int retry = 3);
  void GetSshInfoAsync(const Instance& ggp_instance,
                       const std::function<void(outcome::result<SshInfo>)>& callback);

 private:
  explicit Client(QObject* parent, QString ggp_program, std::chrono::milliseconds timeout)
      : QObject(parent), ggp_program_(std::move(ggp_program)), timeout_(timeout) {}
  static std::chrono::milliseconds GetDefaultTimeoutMs();

  const QString ggp_program_;
  const std::chrono::milliseconds timeout_;
};

}  // namespace orbit_ggp

#endif  // ORBIT_GGP_CLIENT_H_
