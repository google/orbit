// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/Error.h"

#include <absl/strings/str_format.h>

namespace orbit_ssh_qt {

std::string ErrorCategory::message(int condition) const {
  switch (static_cast<Error>(condition)) {
    case Error::kNotConnected:
      return "Not connected";
    case Error::kUncleanSessionShutdown:
      return "The session was shut down while channels were still active.";
    case Error::kUncleanChannelShutdown:
      return "The channel was shut down while operations were still active.";
    case Error::kCouldNotListen:
      return "Could not set up a listening socket.";
    case Error::kRemoteSocketClosed:
      return "The socket was closed on the remote side.";
    case Error::kLocalSocketClosed:
      return "The local socket was closed.";
    case Error::kCouldNotOpenFile:
      return "Could not open file.";
    case Error::kOrbitServiceShutdownTimedout:
      return "Shut down of OrbitService timed out.";
  }

  return absl::StrFormat("Unkown error condition: %i.", condition);
}
}  // namespace orbit_ssh_qt