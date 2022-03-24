// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_H_
#define ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_H_

#include <memory>

#include "OrbitBase/Result.h"
#include "ProducerSideService/ProducerSideServer.h"

namespace orbit_producer_side_service {

// This is a temporary type that can implicitly convert to either the new return type or the old
// one. It is required to keep the cloud collector build working until we can change the internal
// code to the new type. The new return type will be
// `ErrorMessageOr<std::unique_ptr<ProducerSideServer>>`.
class BuildAndStartReturnValue : public ErrorMessageOr<std::unique_ptr<ProducerSideServer>> {
 public:
  template <typename... Args>
  BuildAndStartReturnValue(Args&&... args)  // NOLINT
      : ErrorMessageOr{std::forward<Args>(args)...} {}

  // NOLINTNEXTLINE
  operator std::unique_ptr<ProducerSideServer>() && {
    if (has_value()) return std::move(value());

    ORBIT_ERROR("%s", error().message());
    return nullptr;
  }
};

// TODO(b/221369463): Replace temporary return value
BuildAndStartReturnValue BuildAndStartProducerSideServer();
}  // namespace orbit_producer_side_service

#endif  // ORBIT_PRODUCER_SIDE_SERVICE_BUILD_AND_START_PRODUCER_SIDE_SERVER_H_
