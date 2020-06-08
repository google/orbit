// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Error.h"

#include <absl/strings/str_format.h>

namespace OrbitQt {

std::string ErrorCategory::message(int condition) const {
  switch (static_cast<Error>(condition)) {
    case Error::kCouldNotConnectToServer:
      return "Could not connect to remote server.";
    case Error::kCouldNotUploadPackage:
      return "Could not upload OrbitService package to remote. Please make "
             "sure the .deb package is located in the `collector` folder.";
    case Error::kCouldNotUploadSignature:
      return "Could not upload OrbitService signature to remote. Please make "
             "sure the .deb.asc signature is located in the `collector` "
             "folder.";
    case Error::kCouldNotInstallPackage:
      return "Could not install OrbitService on remote.";
    case Error::kCouldNotStartTunnel:
      return "Could not start tunnel to remote.";
    case Error::kUserCanceledServiceDeployment:
      return "User canceled the deployment.";
    case Error::kUserClosedStartUpWindow:
      return "User closed window.";
  }

  return absl::StrFormat("Unknown error condition: %i.", condition);
}
}  // namespace OrbitQt