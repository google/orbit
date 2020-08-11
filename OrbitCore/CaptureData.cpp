// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureData.h"

orbit_client_protos::LinuxAddressInfo* CaptureData::GetAddressInfo(
    uint64_t address) {
  auto address_info_it = address_infos_.find(address);
  if (address_info_it == address_infos_.end()) {
    return nullptr;
  }
  return &address_info_it->second;
}
