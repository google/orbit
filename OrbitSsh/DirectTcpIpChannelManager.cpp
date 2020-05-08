// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/DirectTcpIpChannelManager.h"

#include <memory>
#include <optional>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Channel.h"
#include "OrbitSsh/ResultType.h"

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
DirectTcpIpChannelManager::State DirectTcpIpChannelManager::Tick() {
  switch (state_) {
    case State::kNotInitialized: {
      if (Channel::CreateDirectTcpIp(session_ptr_, third_party_host_,
                                     third_party_port_,
                                     &channel_) == ResultType::kSuccess) {
        state_ = State::kRunning;
        LOG("Direct Tcp Channel Connected");
      }
      break;
    }
    case State::kRunning: {
      break;
    }
    case State::kSentEOFToRemote: {
      if (channel_->WaitRemoteEOF() != ResultType::kAgain) {
        state_ = State::kRemoteSentEOFBack;
      }
      break;
    }
    case State::kRemoteSentEOFBack: {
      if (channel_->Close() != ResultType::kAgain) {
        state_ = State::kWaitRemoteClosed;
      }
      break;
    }
    case State::kWaitRemoteClosed: {
      if (channel_->WaitClosed() != ResultType::kAgain) {
        channel_ = std::nullopt;
        state_ = State::kNotInitialized;
      }
      break;
    }
  }

  return state_;
}

ResultType DirectTcpIpChannelManager::WriteBlocking(const std::string& text) {
  if (state_ == State::kRunning) return channel_->WriteBlocking(text);

  ERROR("Unable to write to DirectTcpIpChannelManager, state not running");
  return ResultType::kError;
}

ResultType DirectTcpIpChannelManager::Read(std::string* result) {
  if (state_ != State::kRunning) {
    ERROR("Unable to write to DirectTcpIpChannelManager, state not running");
    return ResultType::kError;
  }

  ResultType read_result = channel_->Read(result);

  if (channel_->GetRemoteEOF()) {
    state_ = State::kRemoteSentEOFBack;
    LOG("Direct Tcp Channel Disconnected");
    return ResultType::kError;
  }

  return read_result;
}

// This function closes the channel gracefully. This includes sending
// close (EOF) messages to the remote and receiving corresponding close
// messages. Therefore this function might return kAgain to for example indicate
// its waiting for the close message. This function should be called
// periodically until it returns kSuccess.
ResultType DirectTcpIpChannelManager::Close() {
  ResultType result;
  switch (state_) {
    // send eof
    case State::kRunning:
      result = channel_->SendEOF();
      if (result != ResultType::kSuccess) return result;
      state_ = State::kSentEOFToRemote;

    // wait remote eof
    case State::kSentEOFToRemote:
      result = channel_->WaitRemoteEOF();
      if (result != ResultType::kSuccess) return result;
      state_ = State::kRemoteSentEOFBack;

    // close channel
    case State::kRemoteSentEOFBack:
      result = channel_->Close();
      if (result != ResultType::kSuccess) return result;
      state_ = State::kWaitRemoteClosed;

    // wait closed
    case State::kWaitRemoteClosed:
      result = channel_->WaitClosed();
      if (result != ResultType::kSuccess) return result;
      channel_ = std::nullopt;
      state_ = State::kNotInitialized;

    // do nothing
    case State::kNotInitialized:
      break;
  }

  return ResultType::kSuccess;
}

}  // namespace OrbitSsh