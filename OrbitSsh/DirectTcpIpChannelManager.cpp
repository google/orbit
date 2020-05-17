// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/DirectTcpIpChannelManager.h"

#include <memory>
#include <optional>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Channel.h"

namespace OrbitSsh {

DirectTcpIpChannelManager::DirectTcpIpChannelManager(
    Session* session_ptr, std::string third_party_host, int third_party_port)
    : session_ptr_(session_ptr),
      third_party_host_(third_party_host),
      third_party_port_(third_party_port) {}

// Tick() progresses the state when appropriate. The return values have the
// following meaning
// * kNotInitialized: Trying to establish a tcp connection to the third_party
// * kRunning: Channel to the third_party is open
// * kSentEOFToRemote: A close message has been sent to the remote server and a
// corresponding close message is awaited.
// * kRemoteSentEOFBack: A close message from the remote server arrived, the
// channel will be closed
// * kWaitRemoteClosed: Waiting for the remote server to close the channel. Once
// it is closed the state jumps back to kNotInitialized and tries to establish a
// new connection.
outcome::result<DirectTcpIpChannelManager::State>
DirectTcpIpChannelManager::Tick() {
  if (state_ == State::kNotInitialized) {
    OUTCOME_TRY(Initialize());
  }
  if (state_ != State::kRunning) {
    OUTCOME_TRY(Close());
  }
  return state_;
}
outcome::result<void> DirectTcpIpChannelManager::Initialize() {
  if (state_ == State::kNotInitialized) {
    OUTCOME_TRY(channel,
                Channel::OpenTcpIpTunnel(session_ptr_, third_party_host_,
                                         third_party_port_));
    channel_ = std::move(channel);
    state_ = State::kRunning;
    LOG("Direct Tcp Channel Connected");
  }
  return outcome::success();
}

outcome::result<void> DirectTcpIpChannelManager::WriteBlocking(
    std::string_view data) {
  CHECK(state_ == State::kRunning);
  return channel_->WriteBlocking(data);
}

outcome::result<std::string> DirectTcpIpChannelManager::Read() {
  CHECK(state_ == State::kRunning);
  return channel_->Read();
}

// This function closes the channel gracefully. This includes sending
// close (EOF) messages to the remote and receiving corresponding close
// messages. Therefore this function might return kAgain to for example indicate
// its waiting for the close message. This function should be called
// periodically until it returns kSuccess.
outcome::result<void> DirectTcpIpChannelManager::Close() {
  switch (state_) {
    // send eof
    case State::kRunning: {
      OUTCOME_TRY(channel_->SendEOF());
      state_ = State::kSentEOFToRemote;
    }

    // wait remote eof
    case State::kSentEOFToRemote: {
      OUTCOME_TRY(channel_->WaitRemoteEOF());
      state_ = State::kRemoteSentEOFBack;
    }

    // close channel
    case State::kRemoteSentEOFBack: {
      OUTCOME_TRY(channel_->Close());
      state_ = State::kWaitRemoteClosed;
    }

    // wait closed
    case State::kWaitRemoteClosed: {
      OUTCOME_TRY(channel_->WaitClosed());
      channel_ = std::nullopt;
      state_ = State::kNotInitialized;
    }

    // do nothing
    case State::kNotInitialized:
      break;
  }

  return outcome::success();
}

}  // namespace OrbitSsh
