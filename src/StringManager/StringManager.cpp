// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "StringManager/StringManager.h"

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_string_manager {

// TODO(b/181207737): Make this assert that it is not present and rename to "Add".
bool StringManager::AddIfNotPresent(uint64_t key, std::string_view str) {
  absl::MutexLock lock{&mutex_};
  bool inserted = key_to_string_.emplace(key, str).second;
  if (!inserted) {
    ORBIT_ERROR("String collision for key: %u and string: %s", key, str);
  }
  return inserted;
}

bool StringManager::AddOrReplace(uint64_t key, std::string_view str) {
  absl::MutexLock lock{&mutex_};
  bool inserted = key_to_string_.insert_or_assign(key, str).second;
  return inserted;
}

std::optional<std::string> StringManager::Get(uint64_t key) const {
  absl::MutexLock lock{&mutex_};
  auto it = key_to_string_.find(key);
  if (it != key_to_string_.end()) {
    return it->second;
  }
  return std::nullopt;
}

bool StringManager::Contains(uint64_t key) const {
  absl::MutexLock lock{&mutex_};
  return key_to_string_.contains(key);
}

void StringManager::Clear() {
  absl::MutexLock lock{&mutex_};
  key_to_string_.clear();
}

}  // namespace orbit_string_manager
