// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_CLIENT_H_
#define ORBIT_GGP_CLIENT_H_

#include <QVector>
#include <functional>
#include <outcome.hpp>
#include <string>

#include "Instance.h"
#include "SshInfo.h"

namespace OrbitGgp {

class Client {
 public:
  static outcome::result<Client> Create();

  int GetNumberOfRequestsRunning() const { return number_of_requests_running_; }

  void GetInstancesAsync(
      const std::function<void(outcome::result<QVector<Instance>>)>& callback);
  void GetSshInformationAsync(
      const Instance& ggpInstance,
      const std::function<void(outcome::result<SshInfo>)>& callback);

 private:
  Client() = default;

  int number_of_requests_running_ = 0;
};

}  // namespace OrbitGgp

#endif  // ORBIT_GGP_CLIENT_H_
