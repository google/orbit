// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerSideServer.h"

#include <absl/strings/str_format.h>
#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_impl.h>
#include <sys/stat.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_service {

bool ProducerSideServer::BuildAndStart(std::string_view unix_domain_socket_path) {
  CHECK(server_ == nullptr);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(absl::StrFormat("unix:%s", unix_domain_socket_path),
                           grpc::InsecureServerCredentials());

  builder.RegisterService(&producer_side_service_);

  server_ = builder.BuildAndStart();
  if (server_ == nullptr) {
    return false;
  }

  // When OrbitService runs as root, also allow non-root producers
  // (e.g., the game) to communicate over the Unix domain socket.
  if (chmod(std::string{unix_domain_socket_path}.c_str(), 0777) != 0) {
    ERROR("Changing mode bits to 777 of \"%s\": %s", unix_domain_socket_path, SafeStrerror(errno));
    server_->Shutdown();
    server_->Wait();
    return false;
  }

  return true;
}

void ProducerSideServer::ShutdownAndWait() {
  CHECK(server_ != nullptr);
  producer_side_service_.OnExitRequest();
  server_->Shutdown();
  server_->Wait();
}

void ProducerSideServer::OnCaptureStartRequested(CaptureEventBuffer* capture_event_buffer) {
  producer_side_service_.OnCaptureStartRequested(capture_event_buffer);
}

void ProducerSideServer::OnCaptureStopRequested() {
  producer_side_service_.OnCaptureStopRequested();
}

}  // namespace orbit_service
