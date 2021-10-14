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
#include <optional>
#include <string>

#include "Instance.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Project.h"
#include "SshInfo.h"

namespace orbit_ggp {

constexpr const char* kDefaultGgpProgram{"ggp"};

class Client {
 public:
  Client() = default;
  virtual ~Client() = default;
  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<QVector<Instance>>> GetInstancesAsync(
      bool all_reserved, std::optional<Project> project) = 0;
  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<QVector<Instance>>> GetInstancesAsync(
      bool all_reserved, std::optional<Project> project, int retry) = 0;
  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<SshInfo>> GetSshInfoAsync(
      const Instance& ggp_instance, std::optional<Project> project) = 0;
  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<QVector<Project>>> GetProjectsAsync() = 0;
};

[[nodiscard]] std::chrono::milliseconds GetClientDefaultTimeoutInMs();
ErrorMessageOr<std::unique_ptr<Client>> CreateClient(
    QString ggp_programm = kDefaultGgpProgram,
    std::chrono::milliseconds timeout = GetClientDefaultTimeoutInMs());

}  // namespace orbit_ggp

#endif  // ORBIT_GGP_CLIENT_H_
