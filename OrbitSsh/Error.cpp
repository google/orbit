// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Error.h"

namespace orbit_ssh {

std::string ErrorCategory::message(int condition) const {
  switch (static_cast<Error>(condition)) {
    case Error::kChannelFailure:
      return "Channel failure.";
    case Error::kBannerRecv:
      return "Banner receive.";
    case Error::kBannerSend:
      return "Unable to send banner to remote host.";
    case Error::kInvalidMac:
      return "Invalid MAC address.";
    case Error::kKexFailure:
      return "Encryption key exchange with the remote host failed.";
    case Error::kAlloc:
      return "An internal memory allocation call failed.";
    case Error::kSocketSend:
      return "Unable to send data on socket.";
    case Error::kExchangeFailure:
      return "Exchange failure.";
    case Error::kTimeout:
      return "Timeout";
    case Error::kHostkeyInit:
      return "Host key init.";
    case Error::kHostkeySign:
      return "Host key sign.";
    case Error::kDecrypt:
      return "Decrypt.";
    case Error::kSocketDisconnect:
      return "The socket was disconnected.";
    case Error::kProto:
      return "An invalid SSH protocol response was received on the socket.";
    case Error::kPasswordExpired:
      return "Password expired.";
    case Error::kFile:
      return "File error.";
    case Error::kMethodNone:
      return "Method none.";
    case Error::kAuthenticationFailed:
      return "Authentication failed.";
    case Error::kPublickeyUnverified:
      return "Public key unverified.";
    case Error::kChannelOutOfOrder:
      return "Channel out of order.";
    case Error::kChannelRequestDenied:
      return "Channel request denied.";
    case Error::kChannelUnknown:
      return "Channel unknown.";
    case Error::kChannelWindowExceeded:
      return "Channel window exceeded.";
    case Error::kChannelPacketExceeded:
      return "Channel packet exceeded.";
    case Error::kChannelClosed:
      return "The channel has been closed.";
    case Error::kChannelEofSent:
      return "The channel has been requested to be closed.";
    case Error::kScpProtocol:
      return "SCP protocol error.";
    case Error::kZlib:
      return "zlib-error";
    case Error::kSocketTimeout:
      return "Socket timed out.";
    case Error::kSftpProtocol:
      return "An invalid SFTP protocol response was received on the socket, or "
             "an SFTP operation caused an errorcode to be returned by the "
             "server.";
    case Error::kRequestDenied:
      return "The remote server refused the request.";
    case Error::kMethodNotSupported:
      return "Method not supported.";
    case Error::kInvalid:
      return "Invalid";
    case Error::kInvalidPollType:
      return "Invalid poll type.";
    case Error::kPublicKeyProtocol:
      return "Public key protocol error.";
    case Error::kEagain:
      return "Marked for non-blocking I/O but the call would block.";
    case Error::kBufferToSmall:
      return "Buffer too small.";
    case Error::kBadUse:
      return "Bad use.";
    case Error::kCompress:
      return "Compression error.";
    case Error::kOutOfBoundary:
      return "Out of boundary.";
    case Error::kAgentProtocol:
      return "Agent protocol error.";
    case Error::kSocketRecv:
      return "Socket receive error.";
    case Error::kEncrypt:
      return "Encryption error.";
    case Error::kBadSocket:
      return "Bad socket.";
    case Error::kKnownHosts:
      return "Known hosts error.";
    case Error::kChannelWindowFull:
      return "Channel window full.";
    case Error::kKeyfileAuthFailed:
      return "Key file authentication failed.";
    case Error::kSocketNone:
      return "The socket is invalid.";
    case Error::kInvalidIP:
      return "Unable to parse IP address";
    case Error::kUnknown:
      return "Unknown error code.";
    case Error::kFailedCreatingSession:
      return "Failed to create a session object.";
    case Error::kTaskUsedAfterFinish:
      return "Task invoked after it finished";
  }

  return "Unknown error code.";
}
}  // namespace orbit_ssh
