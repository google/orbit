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

void InsertToUint64Set(Uint64List* data, uint64_t new_value) {
  auto it = std::find(data->data().begin(), data->data().end(), new_value);
  if (it == data->data().end()) {
    data->add_data(new_value);
  }
}

}  // namespace SamplingUtils
