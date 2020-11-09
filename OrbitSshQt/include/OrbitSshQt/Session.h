// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SESSION_H_
#define ORBIT_SSH_QT_SESSION_H_

#include <QObject>
#include <QSocketNotifier>
#include <optional>
#include <outcome.hpp>
#include <system_error>
#include <utility>

#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/Session.h"
#include "OrbitSshQt/StateMachineHelper.h"

namespace OrbitSshQt {
namespace details {
enum class SessionState {
  kInitial,
  kDisconnected,
  kSocketCreated,
  kSocketConnected,
  kSessionCreated,
  kHandshaked,
  kMatchedKnownHosts,
  kStarted,
  kConnected,
  kShutdown,
  kAboutToDisconnect,
  kDone,
  kError
};
}  // namespace details

/*
  Session represents a SSH session. The class takes care of the connecting TCP
  socket, handshaking, server authentication and user authentication.

  It's ready when the started() signal occurred. Otherwise the errorOccurred()
  will fire.
*/
class Session : public StateMachineHelper<Session, details::SessionState> {
  Q_OBJECT

  friend StateMachineHelper;

  using State = details::SessionState;

 public:
  explicit Session(const OrbitSsh::Context* context, QObject* parent = nullptr)
      : StateMachineHelper(parent), context_(context) {}

  void ConnectToServer(OrbitSsh::Credentials creds);
  void Disconnect();

  OrbitSsh::Session* GetRawSession() { return session_ ? &session_.value() : nullptr; }

  void HandleEagain();

 signals:
  void started();
  void stopped();
  void aboutToShutdown();
  void errorOccurred(std::error_code);

  void dataEvent();

 private:
  const OrbitSsh::Context* context_;
  OrbitSsh::Credentials credentials_;

  std::optional<OrbitSsh::Socket> socket_;
  std::optional<OrbitSsh::Session> session_;

  struct NotifierSet {
    QSocketNotifier read;
    QSocketNotifier write;

    explicit NotifierSet(qintptr socket, QObject* parent = nullptr)
        : read(socket, QSocketNotifier::Read, parent),
          write(socket, QSocketNotifier::Write, parent) {}
  };

  std::optional<NotifierSet> notifiers_;

  outcome::result<void> startup();
  outcome::result<void> shutdown();
  outcome::result<void> run();

  void SetError(std::error_code);
};

}  // namespace OrbitSshQt
#endif  // ORBIT_SSH_QT_SESSION_H_
