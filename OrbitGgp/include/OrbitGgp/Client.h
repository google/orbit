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
  // this policy means when a result is wrongly accessed, std::terminate() will
  // be called. (.value() is accessed even though it is an error, or vice versa)
  template <class T>
  using ResultOrQString =
      outcome::result<T, QString, outcome::policy::terminate>;

  static ResultOrQString<Client> Create();

  int GetNumberOfRequestsRunning() const { return number_of_requests_running_; }

  void GetInstancesAsync(
      const std::function<void(ResultOrQString<QVector<Instance>>)>& callback);
  void GetSshInformationAsync(
      const Instance& ggpInstance,
      const std::function<void(ResultOrQString<SshInfo>)>& callback);

 private:
  Client() = default;

  int number_of_requests_running_ = 0;
};

}  // namespace OrbitGgp

#endif  // ORBIT_GGP_CLIENT_H_
