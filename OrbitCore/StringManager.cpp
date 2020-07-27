// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "StringManager.h"

bool StringManager::AddIfNotPresent(uint64_t key, std::string_view str) {
  absl::MutexLock lock{&mutex_};
  if (key_to_string_.contains(key)) {
    return false;
  }
  key_to_string_.emplace(key, str);
  return true;
}

std::optional<std::string> StringManager::Get(uint64_t key) {
  absl::MutexLock lock{&mutex_};
  auto it = key_to_string_.find(key);
  if (it != key_to_string_.end()) {
    return it->second;
  } else {
    return std::optional<std::string>{};
  }
}

bool StringManager::Contains(uint64_t key) {
  absl::MutexLock lock{&mutex_};
  return key_to_string_.contains(key);
}

void StringManager::Clear() {
  absl::MutexLock lock{&mutex_};
  key_to_string_.clear();
}
