// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_DATA_PROVIDER_H_
#define MIZAR_DATA_MIZAR_DATA_PROVIDER_H_

#include <stdint.h>

#include <optional>
#include <string>

#include "ClientData/CaptureDataHolder.h"
#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/Time.h"

namespace orbit_mizar_data {

// Handles one of the two datasets Mizar operates on
class MizarDataProvider : public orbit_client_data::CaptureDataHolder {
  using AbsoluteAddress = ::orbit_mizar_base::AbsoluteAddress;

 public:
  MizarDataProvider() = default;

  MizarDataProvider(MizarDataProvider&) = delete;
  MizarDataProvider& operator=(const MizarDataProvider& other) = delete;

  MizarDataProvider(MizarDataProvider&&) = default;
  MizarDataProvider& operator=(MizarDataProvider&& other) = default;

  virtual ~MizarDataProvider() = default;

  [[nodiscard]] virtual const absl::flat_hash_map<orbit_grpc_protos::PresentEvent::Source,
                                                  std::vector<orbit_grpc_protos::PresentEvent>>&
  source_to_present_events() const = 0;

  [[nodiscard]] virtual std::optional<std::string> GetFunctionNameFromAddress(
      AbsoluteAddress address) const = 0;
  [[nodiscard]] virtual absl::flat_hash_map<AbsoluteAddress, orbit_mizar_base::FunctionSymbol>
  AllAddressToFunctionSymbol() const = 0;

  [[nodiscard]] virtual orbit_mizar_base::TimestampNs GetCaptureStartTimestampNs() const = 0;

  [[nodiscard]] virtual orbit_mizar_base::RelativeTimeNs GetNominalSamplingPeriodNs() const = 0;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_DATA_PROVIDER_H_