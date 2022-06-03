// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_DATA_PROVIDER_H_
#define MIZAR_DATA_MIZAR_DATA_PROVIDER_H_

#include <stdint.h>

#include <optional>
#include <string>

#include "ClientData/CaptureData.h"
#include "ClientData/CaptureDataHolder.h"

namespace orbit_mizar_data {

// Handles one of the two datasets Mizar operates on
class MizarDataProvider : public orbit_client_data::CaptureDataHolder {
 public:
  MizarDataProvider() = default;

  MizarDataProvider(MizarDataProvider&) = delete;
  MizarDataProvider& operator=(const MizarDataProvider& other) = delete;

  MizarDataProvider(MizarDataProvider&&) = default;
  MizarDataProvider& operator=(MizarDataProvider&& other) = default;

  virtual ~MizarDataProvider() = default;

  [[nodiscard]] virtual std::optional<std::string> GetFunctionNameFromAddress(
      uint64_t address) const = 0;
  [[nodiscard]] virtual absl::flat_hash_map<uint64_t, std::string> AllAddressToName() const = 0;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_DATA_PROVIDER_H_