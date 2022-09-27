// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsApiCallManager.h"

#include <absl/container/btree_map.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_windows_api_shim {

ApiFunctionCallManager manager;

ApiFunctionCallManager& ApiFunctionCallManager::Get() { return manager; }

void ApiFunctionCallManager::Reset() {
  for (ApiFunctionCallCounter& counter : api_counters_) {
    counter.call_count = 0;
  }
}

std::string ApiFunctionCallManager::GetSummary() {
  std::string result;
  absl::btree_multimap<uint32_t, const char*> call_count_to_function_key;
  for (size_t i = 0; i < kWindowsApiFunctions.size(); ++i) {
    uint32_t call_count = api_counters_[i].call_count;
    if (call_count > 0) {
      call_count_to_function_key.insert({call_count, kWindowsApiFunctions[i].function_key});
    }
  }

  for (auto it = call_count_to_function_key.rbegin(); it != call_count_to_function_key.rend();
       ++it) {
    result += absl::StrFormat("%s: %u\n", it->second, it->first);
  }

  return result;
}

}  // namespace orbit_windows_api_shim
