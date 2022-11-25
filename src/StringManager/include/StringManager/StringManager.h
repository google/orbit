// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STRING_MANAGER_STRING_MANAGER_H_
#define STRING_MANAGER_STRING_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/synchronization/mutex.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace orbit_string_manager {

// This class is a thread-safe wrapper around `absl::flat_hash_map<uint64_t, std::string>`.
class StringManager {
 public:
  StringManager() = default;

  // Returns true if insertion took place.
  bool AddIfNotPresent(uint64_t key, std::string_view str);
  // Returns true if a new insertion took place, false if the value was replaced.
  bool AddOrReplace(uint64_t key, std::string_view str);

  [[nodiscard]] std::optional<std::string> Get(uint64_t key) const;
  [[nodiscard]] bool Contains(uint64_t key) const;

  void Clear();

 private:
  absl::flat_hash_map<uint64_t, std::string> key_to_string_;
  mutable absl::Mutex mutex_;
};

}  // namespace orbit_string_manager

#endif  // STRING_MANAGER_STRING_MANAGER_H_
