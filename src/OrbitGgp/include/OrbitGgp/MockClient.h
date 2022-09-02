// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_MOCK_CLIENT_H_
#define ORBIT_GGP_MOCK_CLIENT_H_

#include <gmock/gmock.h>

#include <QString>
#include <QVector>
#include <optional>

#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"

namespace orbit_ggp {
class MockClient : public Client {
 public:
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<QVector<Instance>>>, GetInstancesAsync,
              (Client::InstanceListScope /*scope*/, std::optional<Project> /*project*/),
              (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<QVector<Instance>>>, GetInstancesAsync,
              (Client::InstanceListScope /*scope*/, std::optional<Project> /*project*/,
               int /*retry*/),
              (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<SshInfo>>, GetSshInfoAsync,
              (const QString& /*instance_id*/, std::optional<Project> /*project*/), (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<SshInfo>>, GetSshInfoAsync,
              (const QString& /*instance_id*/, std::optional<Project> /*project*/, int /*retry*/),
              (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<QVector<Project>>>, GetProjectsAsync, (),
              (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<Project>>, GetDefaultProjectAsync, (), (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<Instance>>, DescribeInstanceAsync,
              (const QString& /*instance_id*/), (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<Account>>, GetDefaultAccountAsync, (), (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<std::vector<SymbolDownloadInfo>>>,
              GetSymbolDownloadInfoAsync, ((const std::vector<Client::SymbolDownloadQuery>&)),
              (override));
};
}  // namespace orbit_ggp

#endif  // ORBIT_GGP_MOCK_CLIENT_H_