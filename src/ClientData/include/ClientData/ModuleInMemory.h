// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CLIENT_DATA_MODULE_IN_MEMORY_H_
#define CLIENT_DATA_MODULE_IN_MEMORY_H_

#include <absl/strings/str_format.h>

#include <string>

#include "ClientData/ModuleIdentifier.h"

namespace orbit_client_data {
// Small struct to model a space in memory occupied by a module.
class ModuleInMemory {
 public:
  explicit ModuleInMemory(uint64_t start, uint64_t end,
                          orbit_client_data::ModuleIdentifier module_identifier)
      : start_(start), end_(end), module_identifier_(module_identifier) {}

  [[nodiscard]] uint64_t start() const { return start_; }
  [[nodiscard]] uint64_t end() const { return end_; }
  [[nodiscard]] orbit_client_data::ModuleIdentifier module_id() const { return module_identifier_; }
  [[nodiscard]] std::string FormattedAddressRange() const {
    return absl::StrFormat("[%016x - %016x]", start_, end_);
  }

 private:
  uint64_t start_;
  uint64_t end_;
  orbit_client_data::ModuleIdentifier module_identifier_;
};
}  // namespace orbit_client_data

#endif  // CLIENT_DATA_MODULE_IN_MEMORY_H_
