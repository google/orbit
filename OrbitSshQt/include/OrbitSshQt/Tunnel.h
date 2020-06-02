// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_TUNNEL_H_
#define ORBIT_SSH_QT_TUNNEL_H_

#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <deque>
#include <outcome.hpp>

#include "OrbitSsh/Channel.h"
#include "OrbitSshQt/Error.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace OrbitSshQt {
namespace details {
enum class TunnelState {
  kInitial,
  kNoChannel,
  kChannelInitialized,
  kStarted,
  kServerListening,
  kShutdown,
  kFlushing,
  kSendEOF,
  kClosingChannel,
  kWaitRemoteClosed,
  kDone,
  kError
};
}  // namespace details

/*
  Tunnel encapsulates SSH's TCP-tunneling feature and locally connects a
  listening TCP server.

  The remote port and host needs to be specified by the user. The local port
  will be chosen randomly (by the system) and reported by to the caller via the
  tunnelOpen(uint16_t) signal or the GetListenPort() member function.

  Tunnel needs a open and running Session to work.
*/
class Tunnel : public StateMachineHelper<Tunnel, details::TunnelState> {
  Q_OBJECT
  friend StateMachineHelper;

 public:
  explicit Tunnel(Session* session, std::string remote_host,
                  uint16_t remote_port);

  void Start();
  void Stop();

  uint16_t GetListenPort() const {
    return local_server_ ? local_server_->serverPort() : 0;
  }

 signals:
  void tunnelOpened(uint16_t listen_port);
  void started();
  void stopped();
  void errorOccurred(std::error_code);
  void aboutToShutdown();

 private:
  QPointer<Session> session_;

  std::string remote_host_;
  uint16_t remote_port_ = 0;

  outcome::result<void> startup();
  outcome::result<void> shutdown();
  outcome::result<void> run();
  outcome::result<void> readFromChannel();
  outcome::result<void> writeToChannel();
  void HandleSessionShutdown();
  void HandleIncomingDataLocalSocket();
  void HandleEagain();

  using StateMachineHelper::SetError;
  void SetError(std::error_code);

  std::optional<OrbitSsh::Channel> channel_;
  std::optional<QTcpServer> local_server_;
  QPointer<QTcpSocket> local_socket_;
  std::string write_buffer_;
  std::string read_buffer_;

  std::optional<ScopedConnection> data_event_connection_;
  std::optional<ScopedConnection> about_to_shutdown_connection_;
};

}  // namespace OrbitSshQt

#endif  // ORBIT_SSH_QT_TUNNEL_H_