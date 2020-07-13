// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingUtils.h"

namespace SamplingUtils {

unsigned int GetCountForAddress(const ThreadSampleData& data,
                                uint64_t address) {
  auto res = data.m_RawAddressCount.find(address);
  if (res == data.m_RawAddressCount.end()) {
    return 0;
  }
  return (*res).second;
}

}  // namespace SamplingUtils
