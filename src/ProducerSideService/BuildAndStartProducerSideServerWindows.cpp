// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "BuildAndStartProducerSideServerWithUri.h"
#include "OrbitBase/Result.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "ProducerSideService/BuildAndStartProducerSideServer.h"
#include "ProducerSideService/ProducerSideServer.h"

namespace orbit_producer_side_service {

ErrorMessageOr<std::unique_ptr<ProducerSideServer>> BuildAndStartProducerSideServer() {
  return BuildAndStartProducerSideServerWithUri(
      orbit_producer_side_channel::kProducerSideWindowsServerAddress);
}

}  // namespace orbit_producer_side_service