// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_MIZAR_DATA_PROVIDER_H_
#define MIZAR_MIZAR_DATA_PROVIDER_H_

#include <cstdint>
#include <optional>
#include <string>

#include "ClientData/CaptureData.h"
#include "ClientData/OwnsCaptureData.h"

namespace orbit_mizar_data {

// Handles one of the two datasets Mizar operates on
class MizarDataProvider : public orbit_client_data::OwnsCaptureData {
 public:
  virtual ~MizarDataProvider() = default;

  [[nodiscard]] virtual std::optional<std::string> GetFunctionNameFromAddress(
      uint64_t address) const = 0;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_MIZAR_DATA_PROVIDER_H_