// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_DEPLOYMENT_CONFIGURATIONS_H_
#define SESSION_SETUP_DEPLOYMENT_CONFIGURATIONS_H_

#include <filesystem>
#include <string>
#include <utility>
#include <variant>

namespace orbit_session_setup {

struct SignedDebianPackageDeployment {
  SignedDebianPackageDeployment();
  SignedDebianPackageDeployment(std::filesystem::path path_to_package,
                                std::filesystem::path path_to_signature)
      : path_to_package(std::move(path_to_package)),
        path_to_signature(std::move(path_to_signature)) {}

  std::filesystem::path path_to_package;
  std::filesystem::path path_to_signature;
};

struct BareExecutableAndRootPasswordDeployment {
  std::filesystem::path path_to_executable;
  std::string root_password;
};

struct NoDeployment {};

using DeploymentConfiguration = std::variant<SignedDebianPackageDeployment,
                                             BareExecutableAndRootPasswordDeployment, NoDeployment>;

DeploymentConfiguration FigureOutDeploymentConfiguration();

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_DEPLOYMENT_CONFIGURATIONS_H_