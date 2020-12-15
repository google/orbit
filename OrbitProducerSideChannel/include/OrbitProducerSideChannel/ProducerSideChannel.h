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

// This is the default path of the Unix domain socket used for the communication
// between producers of CaptureEvents and OrbitService.
constexpr std::string_view kProducerSideUnixDomainSocketPath = "/tmp/orbit-producer-side-socket";

// This function returns a gRPC channel that uses a Unix domain socket,
// by default the one specified by kProducerSideUnixDomainSocketPath.
inline std::shared_ptr<grpc::Channel> CreateProducerSideChannel(
    std::string_view unix_domain_socket_path = kProducerSideUnixDomainSocketPath) {
  std::string server_address = absl::StrFormat("unix:%s", unix_domain_socket_path);
  return grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
}

}  // namespace orbit_producer_side_channel

#endif  // ORBIT_PRODUCER_SIDE_CHANNEL_PRODUCER_SIDE_CHANNEL_H_
