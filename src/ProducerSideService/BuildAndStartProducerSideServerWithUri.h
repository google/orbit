// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_WITH_URI_H_
#define ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_WITH_URI_H_

#include "OrbitBase/Logging.h"
#include "ProducerSideService/ProducerSideServer.h"

namespace orbit_producer_side_service {

inline ErrorMessageOr<std::unique_ptr<ProducerSideServer>> BuildAndStartProducerSideServerWithUri(
    std::string_view uri) {
  auto producer_side_server = std::make_unique<ProducerSideServer>();
  ORBIT_LOG("Starting producer-side server at %s", uri);
  if (!producer_side_server->BuildAndStart(uri)) {
    return ErrorMessage{"Unable to start producer-side server."};
  }
  ORBIT_LOG("Producer-side server is running");
  return producer_side_server;
}

}  // namespace orbit_producer_side_service

#endif  // ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_WITH_URI_H_