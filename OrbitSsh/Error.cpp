// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitSsh/Error.h>

namespace OrbitSsh {

std::string SftpErrorCategory::message(int condition) const {
  switch (static_cast<SftpError>(condition)) {
    case SftpError::kAlloc:
      return "An internal memory allocation call failed.";
    case SftpError::kSocketSend:
      return "Unable to send data on socket.";
    case SftpError::kSocketTimeout:
      return "Socket timed out.";
    case SftpError::kSftpProtocol:
      return "An invalid SFTP protocol response was received on the socket, or "
             "an SFTP operation caused an errorcode to be returned by the "
             "server.";
    case SftpError::kEagain:
      return "Marked for non-blocking I/O but the call would block.";
  }
}
}  // namespace OrbitSsh
