// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BuildAndStartProducerSideServerWithUri.h"
#include "ProducerSideService/BuildAndStartProducerSideServer.h"

namespace orbit_producer_side_service {

std::unique_ptr<ProducerSideServer> BuildAndStartProducerSideServer() {
  constexpr const char* kProducerSideServerUri = "127.0.0.1:1789";
  return BuildAndStartProducerSideServerWithUri(kProducerSideServerUri);
}

}  // namespace orbit_producer_side_service