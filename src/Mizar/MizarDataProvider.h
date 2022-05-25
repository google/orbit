// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_MIZAR_DATA_PROVIDER_H_
#define MIZAR_MIZAR_DATA_PROVIDER_H_

#include <string>

#include "ClientData/CaptureData.h"

namespace orbit_mizar {

class MizarDataProvider {
 public:
  virtual ~MizarDataProvider() = default;

  [[nodiscard]] virtual std::string GetFunctionNameFromAddress(uint64_t address) const = 0;

  [[nodiscard]] virtual orbit_client_data::CaptureData& GetCaptureData() const = 0;
};

}  // namespace orbit_mizar

#endif  // MIZAR_MIZAR_DATA_PROVIDER_H_