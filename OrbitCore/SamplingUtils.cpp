// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingUtils.h"

namespace SamplingUtils {

unsigned int GetCountForAddress(const ThreadSampleData& data,
                                uint64_t address) {
  auto res = data.raw_address_count().find(address);
  if (res == data.raw_address_count().end()) {
    return 0;
  }
  return (*res).second;
}

ThreadSampleData CreateThreadSampleData() {
  ThreadSampleData data;
  data.add_thread_usage(0);
  return data;
}

}  // namespace SamplingUtils
