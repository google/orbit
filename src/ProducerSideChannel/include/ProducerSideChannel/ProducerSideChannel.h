// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_SIDE_CHANNEL_PRODUCER_SIDE_CHANNEL_H_
#define ORBIT_PRODUCER_SIDE_CHANNEL_PRODUCER_SIDE_CHANNEL_H_

#include <absl/strings/str_format.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

namespace orbit_producer_side_channel {

// This is the default server address used for the communication between producers of CaptureEvents
// and OrbitService on Windows.
constexpr const char* kProducerSideWindowsServerAddress = "localhost:1789";

// This is the default path of the Unix domain socket used for the communication
// between producers of CaptureEvents and OrbitService.
constexpr const char* kProducerSideUnixDomainSocketPath = "/tmp/orbit-producer-side-socket";

// This function returns a gRPC channel that uses a Unix domain socket,
// by default the one specified by kProducerSideUnixDomainSocketPath.
inline std::shared_ptr<grpc::Channel> CreateProducerSideChannel(
    std::string_view unix_domain_socket_path = kProducerSideUnixDomainSocketPath) {
#ifdef WIN32
  std::string server_address = kProducerSideWindowsServerAddress;
#else
  std::string server_address = absl::StrFormat("unix:%s", unix_domain_socket_path);
#endif

  grpc::ChannelArguments channel_arguments;
  // Significantly reduce the gRPC channel's reconnection backoff time. Defaults for min and max
  // would be 20 seconds and 2 minutes. That's too much for us, as we want a producer to quickly
  // connect to OrbitService after Orbit is started, so that when starting a capture the producer
  // can already send data.
  constexpr int kMinReconnectBackoffMs = 1000;
  constexpr int kMaxReconnectBackoffMs = 1000;
  channel_arguments.SetInt(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, kMinReconnectBackoffMs);
  channel_arguments.SetInt(GRPC_ARG_MAX_RECONNECT_BACKOFF_MS, kMaxReconnectBackoffMs);

  return grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(),
                                   channel_arguments);
}

}  // namespace orbit_producer_side_channel

#endif  // ORBIT_PRODUCER_SIDE_CHANNEL_PRODUCER_SIDE_CHANNEL_H_
