// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBITGGP_GGP_CLIENT_H_
#define ORBITGGP_GGP_CLIENT_H_

#include <QVector>
#include <functional>
#include <outcome.hpp>
#include <string>

#include "GgpInstance.h"

class GgpClient {
 public:
  // this policy means when a result is wrongly accessed, std::terminate() will
  // be called. (.value() is accessed even though it is an error, or vice versa)
  template <class T>
  using ResultOrQString =
      outcome::result<T, QString, outcome::policy::terminate>;

  static ResultOrQString<GgpClient> Create();

  const std::string& GetVersion() const { return version_; }
  int GetNumberOfRequestsRunning() const { return number_of_requests_running_; }

  void GetInstancesAsync(
      const std::function<void(ResultOrQString<QVector<GgpInstance>>)>&
          callback);

 private:
  GgpClient() = default;

  std::string version_;
  int number_of_requests_running_ = 0;
};

#endif  // ORBITGGP_GGP_CLIENT_H_
