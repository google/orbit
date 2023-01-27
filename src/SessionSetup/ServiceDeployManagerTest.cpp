// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <QMetaType>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <variant>
#include <vector>

#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "OrbitSsh/Context.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/Task.h"
#include "QtTestUtils/WaitForWithTimeout.h"
#include "SessionSetup/DeploymentConfigurations.h"
#include "SessionSetup/ServiceDeployManager.h"
#include "SshQtTestUtils/KillProcessListeningOnTcpPort.h"
#include "SshQtTestUtils/ParsePortNumberFromSocatOutput.h"
#include "SshQtTestUtils/SshSessionTest.h"
#include "Test/Path.h"
#include "TestUtils/TemporaryDirectory.h"
#include "TestUtils/TestUtils.h"

namespace orbit_session_setup {
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_qt_test_utils::YieldsResult;
using orbit_ssh_qt_test_utils::KillProcessListeningOnTcpPort;
using orbit_test_utils::HasNoError;

using ServiceDeployManagerTest = orbit_ssh_qt_test_utils::SshSessionTest;

struct ServiceDeployManagerSigningTest : orbit_ssh_qt_test_utils::SshSessionTest {
  constexpr static std::string_view kSigningSshServerEnvironmentVariableName =
      "ORBIT_TESTING_SSH_SERVER_SIGNING_ADDRESS";
  ServiceDeployManagerSigningTest()
      : orbit_ssh_qt_test_utils::SshSessionTest{
            std::string{kSigningSshServerEnvironmentVariableName}} {}
};

TEST_F(ServiceDeployManagerTest, NoDeployment) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_THAT(context, orbit_test_utils::HasValue());

  orbit_ssh_qt::Session helper_session{&context.value()};
  EXPECT_THAT(WaitForWithTimeout(helper_session.ConnectToServer(GetCredentials())),
              YieldsResult(HasNoError()));

  // This is emulating OrbitService - It's important that we don't hard code any ports because all
  // tests usually share the same network namespace and they might run concurrently.
  orbit_ssh_qt::Task orbit_service_task{&helper_session,
                                        "socat -dd TCP-LISTEN:0,fork exec:'/bin/cat'"};
  std::string socat_output{};
  orbit_ssh_qt::ScopedConnection output_connection{
      QObject::connect(&orbit_service_task, &orbit_ssh_qt::Task::readyReadStdErr,
                       &orbit_service_task, [&orbit_service_task, &socat_output]() {
                         socat_output.append(orbit_service_task.ReadStdErr());
                       })};
  EXPECT_THAT(WaitForWithTimeout(orbit_service_task.Start()), YieldsResult(HasNoError()));

  std::optional<ErrorMessageOr<int>> port_or_error{};
  std::ignore = QTest::qWaitFor([&]() {
    port_or_error = orbit_ssh_qt_test_utils::ParsePortNumberFromSocatOutput(socat_output);
    return port_or_error.has_value();
  });
  ASSERT_TRUE(port_or_error.has_value()) << "The socat output was: " << socat_output;
  ASSERT_THAT(port_or_error.value(), HasNoError());
  const ServiceDeployManager::GrpcPort grpc_port{static_cast<uint16_t>(port_or_error->value())};

  // We need the signal spy have longer lifetime than the ServiceDeployManager because we wanna
  // ensure that the ServiceDeployManager's destructor does not emit any signals.
  qRegisterMetaType<std::error_code>("std::error_code");
  std::optional<QSignalSpy> socket_error_signal{};
  {
    DeploymentConfiguration deployment_config{NoDeployment{}};
    ServiceDeployManager sdm{&deployment_config, &context.value(), GetCredentials(), grpc_port};
    socket_error_signal.emplace(&sdm, &ServiceDeployManager::socketErrorOccurred);
    EXPECT_THAT(sdm.Exec(), orbit_test_utils::HasNoError());
  }
  EXPECT_THAT(*socket_error_signal, testing::IsEmpty());

  EXPECT_THAT(KillProcessListeningOnTcpPort(&helper_session, grpc_port.grpc_port), HasNoError());
  EXPECT_THAT(WaitForWithTimeout(orbit_service_task.Stop()), YieldsResult(HasNoError()));
  EXPECT_THAT(WaitForWithTimeout(helper_session.Disconnect()), YieldsResult(HasNoError()));
}

TEST_F(ServiceDeployManagerTest, BareExecutableAndRootPassword) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_THAT(context, orbit_test_utils::HasValue());

  orbit_ssh_qt::Session helper_session{&context.value()};
  EXPECT_THAT(WaitForWithTimeout(helper_session.ConnectToServer(GetCredentials())),
              YieldsResult(HasNoError()));

  // We need the signal spy have longer lifetime than the ServiceDeployManager because we wanna
  // ensure that the ServiceDeployManager's destructor does not emit any signals.
  qRegisterMetaType<std::error_code>("std::error_code");
  std::optional<QSignalSpy> socket_error_signal{};
  {
    DeploymentConfiguration deployment_config{BareExecutableAndRootPasswordDeployment{
        orbit_test::GetTestdataDir() / "deployments" / "BareExecutableAndRootPassword" / "bin" /
            "emulate_orbit_service.sh",
        "loginpassword"}};
    ServiceDeployManager sdm{&deployment_config, &context.value(), GetCredentials(),
                             ServiceDeployManager::GrpcPort{44765}};
    socket_error_signal.emplace(&sdm, &ServiceDeployManager::socketErrorOccurred);
    EXPECT_THAT(sdm.Exec(), orbit_test_utils::HasNoError());
  }
  EXPECT_THAT(*socket_error_signal, testing::IsEmpty());

  EXPECT_THAT(WaitForWithTimeout(helper_session.Disconnect()), YieldsResult(HasNoError()));
}

TEST_F(ServiceDeployManagerSigningTest, SignedDebianPackageDeployment) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_THAT(context, orbit_test_utils::HasValue());

  orbit_ssh_qt::Session helper_session{&context.value()};
  EXPECT_THAT(WaitForWithTimeout(helper_session.ConnectToServer(GetCredentials())),
              YieldsResult(HasNoError()));

  // We need the signal spy have longer lifetime than the ServiceDeployManager because we wanna
  // ensure that the ServiceDeployManager's destructor does not emit any signals.
  qRegisterMetaType<std::error_code>("std::error_code");
  std::optional<QSignalSpy> socket_error_signal{};
  {
    DeploymentConfiguration deployment_config{SignedDebianPackageDeployment{
        orbit_test::GetTestdataDir() / "deployments" / "SignedDebianPackage" / "OrbitService",
        orbit_test::GetTestdataDir() / "deployments" / "SignedDebianPackage" / "OrbitService.asc"}};
    ServiceDeployManager sdm{&deployment_config, &context.value(), GetCredentials(),
                             ServiceDeployManager::GrpcPort{44765}};
    socket_error_signal.emplace(&sdm, &ServiceDeployManager::socketErrorOccurred);
    EXPECT_THAT(sdm.Exec(), orbit_test_utils::HasNoError());
  }
  EXPECT_THAT(*socket_error_signal, testing::IsEmpty());

  EXPECT_THAT(WaitForWithTimeout(helper_session.Disconnect()), YieldsResult(HasNoError()));
}

TEST_F(ServiceDeployManagerTest, CopyFileToLocal) {
  auto context = orbit_ssh::Context::Create();
  ASSERT_THAT(context, orbit_test_utils::HasValue());

  orbit_ssh_qt::Session helper_session{&context.value()};
  EXPECT_THAT(WaitForWithTimeout(helper_session.ConnectToServer(GetCredentials())),
              YieldsResult(HasNoError()));

  // This is emulating OrbitService - It's important that we don't hard code any ports because all
  // tests usually share the same network namespace and they might run concurrently.
  orbit_ssh_qt::Task orbit_service_task{&helper_session,
                                        "socat -dd TCP-LISTEN:0,fork exec:'/bin/cat'"};
  std::string socat_output{};
  orbit_ssh_qt::ScopedConnection output_connection{
      QObject::connect(&orbit_service_task, &orbit_ssh_qt::Task::readyReadStdErr,
                       &orbit_service_task, [&orbit_service_task, &socat_output]() {
                         socat_output.append(orbit_service_task.ReadStdErr());
                       })};
  EXPECT_THAT(WaitForWithTimeout(orbit_service_task.Start()), YieldsResult(HasNoError()));

  std::optional<ErrorMessageOr<int>> port_or_error{};
  std::ignore = QTest::qWaitFor([&]() {
    port_or_error = orbit_ssh_qt_test_utils::ParsePortNumberFromSocatOutput(socat_output);
    return port_or_error.has_value();
  });
  ASSERT_TRUE(port_or_error.has_value()) << "The socat output was: " << socat_output;
  ASSERT_THAT(port_or_error.value(), HasNoError());
  const ServiceDeployManager::GrpcPort grpc_port{static_cast<uint16_t>(port_or_error->value())};

  {
    DeploymentConfiguration deployment_config{NoDeployment{}};
    ServiceDeployManager sdm{&deployment_config, &context.value(), GetCredentials(), grpc_port};
    EXPECT_THAT(sdm.Exec(), orbit_test_utils::HasNoError());

    auto temp_dir = orbit_test_utils::TemporaryDirectory::Create();
    ASSERT_THAT(temp_dir, orbit_test_utils::HasValue());
    orbit_base::StopSource stop_source{};
    auto future = sdm.CopyFileToLocal("/home/loginuser/plain.txt",
                                      temp_dir.value().GetDirectoryPath() / "plain.txt",
                                      stop_source.GetStopToken());
    auto result = WaitForWithTimeout(future);
    ASSERT_THAT(result, YieldsResult(HasValue(orbit_test_utils::HasNotBeenCanceled())));
  }

  EXPECT_THAT(KillProcessListeningOnTcpPort(&helper_session, grpc_port.grpc_port), HasNoError());
  EXPECT_THAT(WaitForWithTimeout(orbit_service_task.Stop()), YieldsResult(HasNoError()));
  EXPECT_THAT(WaitForWithTimeout(helper_session.Disconnect()), YieldsResult(HasNoError()));
}
}  // namespace orbit_session_setup
