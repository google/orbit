// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_CLIENT_H_
#define ORBIT_GGP_CLIENT_H_

#include <QObject>
#include <QVector>
#include <functional>
#include <outcome.hpp>
#include <string>

#include "Instance.h"
#include "SshInfo.h"

namespace OrbitGgp {

class Client : public QObject {
  Q_OBJECT

 public:
  static outcome::result<QPointer<Client>> Create(QObject* parent);

  void GetInstancesAsync(const std::function<void(outcome::result<QVector<Instance>>)>& callback);
  void GetSshInfoAsync(const Instance& ggp_instance,
                       const std::function<void(outcome::result<SshInfo>)>& callback);

 private:
  explicit Client(QObject* parent) : QObject(parent) {}
};

}  // namespace OrbitGgp

#endif  // ORBIT_GGP_CLIENT_H_
