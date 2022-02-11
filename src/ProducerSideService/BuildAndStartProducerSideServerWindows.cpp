// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BuildAndStartProducerSideServerWithUri.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "ProducerSideService/BuildAndStartProducerSideServer.h"


namespace orbit_producer_side_service {

std::unique_ptr<ProducerSideServer> BuildAndStartProducerSideServer() {
  return BuildAndStartProducerSideServerWithUri(
      orbit_producer_side_channel::kProducerSideWindowsServerAddress);
}

}  // namespace orbit_producer_side_service