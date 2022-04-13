// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_H_
#define ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_H_

#include <memory>

#include "OrbitBase/Result.h"
#include "ProducerSideService/ProducerSideServer.h"

namespace orbit_producer_side_service {

ErrorMessageOr<std::unique_ptr<ProducerSideServer>> BuildAndStartProducerSideServer();
}  // namespace orbit_producer_side_service

#endif  // ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_H_
