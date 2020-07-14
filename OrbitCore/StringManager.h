// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_STRING_MANAGER_H_
#define ORBIT_CORE_STRING_MANAGER_H_

#include <optional>
#include <string>

#include "absl/synchronization/mutex.h"
#include "basic_types.pb.h"

class StringManager {
 public:
  StringManager() { key_to_string_ = std::make_unique<Uint64ToString>(); }

  bool AddIfNotPresent(uint64_t key, std::string_view str);
  std::optional<std::string> Get(uint64_t key);
  bool Contains(uint64_t key);
  void Clear();
  const std::unique_ptr<Uint64ToString>& GetData() { return key_to_string_; }
  void SetData(std::unique_ptr<Uint64ToString> data_ptr) {
    key_to_string_ = std::move(data_ptr);
  }

 private:
  std::unique_ptr<Uint64ToString> key_to_string_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_CORE_STRING_MANAGER_H_
