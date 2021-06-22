// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <QPushButton>
#include <optional>

#include "Connections.h"
#include "MetricsUploader/MetricsUploaderStub.h"
#include "OrbitSsh/Context.h"
#include "ProfilingTargetDialog.h"

namespace orbit_qt {

// TODO(b/189838600) Add tests that check correctness.

TEST(ProfilingTargetDialog, SmokeTest) {
  auto ssh_context = orbit_ssh::Context::Create();
  ASSERT_TRUE(ssh_context.has_value());

  DeploymentConfiguration config{NoDeployment{}};

  SshConnectionArtifacts ssh_artifacts{&ssh_context.value(), ServiceDeployManager::GrpcPort{0},
                                       &config};

  orbit_metrics_uploader::MetricsUploaderStub uploader{};

  ProfilingTargetDialog dialog{&ssh_artifacts, std::nullopt, &uploader};

  QApplication::processEvents();
  EXPECT_TRUE(dialog.isEnabled());

  QMetaObject::invokeMethod(
      &dialog, [&dialog]() { dialog.reject(); }, Qt::ConnectionType::QueuedConnection);

  std::optional<TargetConfiguration> result = dialog.Exec();
  EXPECT_EQ(result, std::nullopt);
}

}  // namespace orbit_qt